#include "stagemanager.h"

StageManager::StageManager(QWidget *parent) : QGraphicsView(parent)
{
	this->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
	this->gsMetatiles = new QGraphicsScene(this);
	this->gsMetatiles->setItemIndexMethod(QGraphicsScene::NoIndex);

	this->setScene(this->gsMetatiles);
	this->iScale = SM_DEFAULT_ZOOM;
	this->bShowScreenGrid = this->bShowTileGrid = true;
	this->iTileQueue = 0;
	this->setBackgroundBrush(QBrush(Qt::black));

	this->vScreens = ScreenList(SM_SCREENS_W*SM_SCREENS_H);

	this->groupMetatiles = new QGraphicsItemGroup();
	this->gsMetatiles->addItem(this->groupMetatiles);

	this->pMouseTranslation = QPointF(0,0);
	this->pRightMousePos = QPointF(-1,-1);
	this->pSceneTranslation = QPointF(0,0);

	this->populateBlankTiles();

	this->updateScreen();
}

StageManager::~StageManager()
{
	this->gsMetatiles->clear();
	this->gsMetatiles->deleteLater();
}



void StageManager::dropEvent(QDropEvent *e)
{
	e->acceptProposedAction();
	this->openStageFile(e->mimeData()->urls()[0].toLocalFile());
}

void StageManager::mousePressEvent(QMouseEvent *e)
{
	switch(e->button()) {
	case Qt::RightButton:
		this->pRightMousePos = QPointF(-1,-1);
		this->replaceScreenTile(this->mapToScene(e->pos()));
		break;
	case Qt::MiddleButton:
		this->pMouseTranslation = QPointF(e->x(),e->y());
		break;
	default:
		QGraphicsView::mousePressEvent(e);
	}
}

void StageManager::mouseMoveEvent(QMouseEvent *e)
{
	QGraphicsView::mouseMoveEvent(e);

	if(e->buttons()&Qt::MiddleButton) {
		this->setTransformationAnchor(QGraphicsView::NoAnchor);
		this->translate((e->x()-this->pMouseTranslation.x()),(e->y()-this->pMouseTranslation.y()));
		this->pMouseTranslation = QPointF(e->x(),e->y());
	} else if(e->buttons()&Qt::RightButton) {
		this->replaceScreenTile(this->mapToScene(e->pos()));
	}
}

void StageManager::mouseDoubleClickEvent(QMouseEvent *e)
{
	switch(e->button()) {
	case Qt::MiddleButton:
		this->iScale = SM_DEFAULT_ZOOM;
		this->setSceneRect(0, 0, SM_CANVAS_WIDTH*this->iScale, SM_CANVAS_HEIGHT*this->iScale);
		this->updateScreen();
		break;
	case Qt::RightButton:
		this->replaceAllScreenTiles(this->mapToScene(e->pos()));
		break;
	default:
		QGraphicsView::mouseDoubleClickEvent(e);
	}
}

void StageManager::wheelEvent(QWheelEvent *e)
{
	this->pSceneTranslation.setX(qFloor(this->transform().dx()/this->iScale));
	this->pSceneTranslation.setY(qFloor(this->transform().dy()/this->iScale));

	qreal steps = (((qreal)e->angleDelta().y()/8)/15)/4;
	if(((this->iScale+steps)>=1) && ((this->iScale+steps)<=SM_MAX_ZOOM))
		this->iScale += steps;
	else
		this->iScale = ((steps<0)?1:SM_MAX_ZOOM);

	this->updateScreen();
}

void StageManager::mouseReleaseEvent(QMouseEvent *e)
{
	QGraphicsView::mouseReleaseEvent(e);
}

void StageManager::keyPressEvent(QKeyEvent *e)
{
	switch(e->key()) {
//	case Qt::Key_Left:
//	case Qt::Key_Right:
//		this->moveSelectedX((e->key()==Qt::Key_Right),(e->modifiers()&Qt::ShiftModifier));
//		break;
//	case Qt::Key_Up:
//	case Qt::Key_Down:
//		this->moveSelectedY((e->key()==Qt::Key_Down),(e->modifiers()&Qt::ShiftModifier));
//		break;
//	case Qt::Key_PageUp:
//		this->moveSelectedUp();
//		break;
//	case Qt::Key_PageDown:
//		this->moveSelectedDown();
//		break;
//	case Qt::Key_H:
//		this->flipHorizontal();
//		break;
//	case Qt::Key_V:
//		this->flipVertical();
//		break;
	default:
		QGraphicsView::keyPressEvent(e);
	}
}



void StageManager::changePalette(int p)
{
	emit(requestPaletteUpdates(p));
}



void StageManager::drawGridLines()
{
	foreach(QGraphicsLineItem *i, this->lGrid)
		this->gsMetatiles->removeItem(i);
	this->lGrid.clear();

	if(this->bShowScreenGrid || this->bShowTileGrid) {
		QPen thicksolid(Qt::red,SM_THICK_GRID_LINES,Qt::SolidLine);
//		QPen thickdashes(Qt::white,SM_THICK_GRID_LINES,Qt::DashLine);
		QPen thinsolid(Qt::darkGray,SM_THIN_GRID_LINES,Qt::SolidLine);
		QPen thindashes(Qt::lightGray,SM_THIN_GRID_LINES,Qt::DashLine);
		QVector<qreal> dp, dplong;
		dp << 1 << 1;
		dplong << 4 << 4;
//		thickdashes.setDashPattern(dplong);
		thindashes.setDashPattern(dp);

		qreal canvaswidth = SM_CANVAS_WIDTH*this->iScale;
		qreal canvasheight = SM_CANVAS_HEIGHT*this->iScale;

		if(this->bShowTileGrid) {
			for(int i=0; i<=canvasheight; i+=MTI_TILEWIDTH*this->iScale) {
				// Horizontal lines
				this->lGrid.append(this->gsMetatiles->addLine(0,i,canvaswidth,i,thinsolid));
				this->lGrid.append(this->gsMetatiles->addLine(0,i,canvaswidth,i,thindashes));
			}
			for(int i=0; i<=canvaswidth; i+=MTI_TILEWIDTH*this->iScale) {
				// Vertical lines
				this->lGrid.append(this->gsMetatiles->addLine(i,0,i,canvasheight,thinsolid));
				this->lGrid.append(this->gsMetatiles->addLine(i,0,i,canvasheight,thindashes));
			}
		}

		if(this->bShowScreenGrid) {
			for(int i=0; i<= canvasheight; i+=SM_SCREEN_HEIGHT*this->iScale) {
				// Horizontal lines
				this->lGrid.append(this->gsMetatiles->addLine(0,i,canvaswidth,i,thicksolid));
//				this->lGrid.append(this->gsMetatiles->addLine(0,i,canvaswidth,i,thickdashes));
			}
			for(int i=0; i<= canvaswidth; i+=SM_SCREEN_WIDTH*this->iScale) {
				// Vertical lines
				this->lGrid.append(this->gsMetatiles->addLine(i,0,i,canvasheight,thicksolid));
//				this->lGrid.append(this->gsMetatiles->addLine(i,0,i,canvasheight,thickdashes));
			}
		}
	}
}

void StageManager::populateBlankTiles()
{
	for(int sy=0; sy<SM_SCREENS_H; sy++) {
		for(int sx=0; sx<SM_SCREENS_W; sx++) {
			// Each screen gets its own tile list
			int screen = sy*SM_SCREENS_W+sx;
			this->vScreens[screen].clear();
			for(int y=0; y<SM_SCREEN_TILES_H; y++) {
				for(int x=0; x<SM_SCREEN_TILES_W; x++) {
					MetatileItem *i = new MetatileItem();
					i->setRealX((x*MTI_TILEWIDTH)+(sx*SM_SCREEN_TILES_W*MTI_TILEWIDTH));
					i->setRealY((y*MTI_TILEWIDTH)+(sy*SM_SCREEN_TILES_H*MTI_TILEWIDTH));
					i->setScreen(screen);
					this->vScreens[screen].append(i);
					this->groupMetatiles->addToGroup(i);
				}
			}
		}
	}

	emit(this->requestNewGlobalPalette());
}

void StageManager::getUpdatedTile(MetatileItem *mtold, quint8 i)
{
	quint8 index = this->vScreens[mtold->screen()].indexOf(mtold);
	this->vScreens[mtold->screen()][index]->setMetatileIndex(i);
	this->updateScreen();
}
void StageManager::getUpdatedPalette(MetatileItem *mtnew)
{
	foreach(MetatileList l, this->vScreens) {
		foreach(MetatileItem *i, l) {
			if(i->metatileIndex()==mtnew->metatileIndex())
				i->setPalette(mtnew->palette());
		}
	}
}

void StageManager::setNewGlobalPalette(PaletteVector c)
{
//	for(int colourindex=0; colourindex<c.size(); colourindex++)
//		this->imgScreenCanvas.setColor(colourindex,c.at(colourindex));
//	this->gpiScreenCanvas->setPixmap(QPixmap::fromImage(this->imgScreenCanvas));
}

void StageManager::replaceScreenTile(QPointF p)
{
	if(p.x()<0 || p.y()<0)	return;

	int tilex = roundToMult(qRound(p.x()/this->iScale),MTI_TILEWIDTH)/MTI_TILEWIDTH;
	int tiley = roundToMult(qRound(p.y()/this->iScale),MTI_TILEWIDTH)/MTI_TILEWIDTH;
	int screenx = qFloor(tilex/SM_SCREEN_TILES_W);
	int screeny = qFloor(tiley/SM_SCREEN_TILES_H);
	quint8 screen = (screeny*SM_SCREENS_W)+screenx;
	quint8 tile = ((tiley%SM_SCREEN_TILES_H)*SM_SCREEN_TILES_W)+(tilex%SM_SCREEN_TILES_W);

	if((screen>=(SM_SCREENS_W*SM_SCREENS_H) || tile>=(SM_SCREEN_TILES_W*SM_SCREEN_TILES_H)) ||
			(this->pRightMousePos.x()==screen && this->pRightMousePos.y()==tile))
		return;
	this->pRightMousePos = QPointF(screen,tile);

	emit(this->requestSelectedMetatile(this->vScreens[screen][tile]));
}

void StageManager::replaceAllScreenTiles(QPointF p)
{
	if(p.x()<0 || p.y()<0)	return;

	int tilex = roundToMult(qRound(p.x()/this->iScale),MTI_TILEWIDTH)/MTI_TILEWIDTH;
	int tiley = roundToMult(qRound(p.y()/this->iScale),MTI_TILEWIDTH)/MTI_TILEWIDTH;
	int screenx = qFloor(tilex/SM_SCREEN_TILES_W);
	int screeny = qFloor(tiley/SM_SCREEN_TILES_H);
	quint8 screen = (screeny*SM_SCREENS_W)+screenx;
	quint8 tile = ((tiley%SM_SCREEN_TILES_H)*SM_SCREEN_TILES_W)+(tilex%SM_SCREEN_TILES_W);

	foreach(MetatileItem *i, this->vScreens[screen])
		i->setMetatileIndex(this->vScreens[screen][tile]->metatileIndex());
	this->updateScreen();
}

void StageManager::deleteSelectedTiles()
{
	QList<QGraphicsItem*> sel = this->gsMetatiles->selectedItems();
	foreach(QGraphicsItem *s, sel) {
		this->gsMetatiles->removeItem(s);
	}

	QList<QGraphicsItem*> items = this->gsMetatiles->items(Qt::AscendingOrder);
	MetatileList store;
	foreach(QGraphicsItem *i, items) {
		if(i->type()!=MetatileItem::Type)   continue;
		MetatileItem *ms = qgraphicsitem_cast<MetatileItem*>(i);
		store.append(ms);
	}
}

void StageManager::clearAllMetatileData()
{
	this->gsMetatiles->clear();
	foreach(MetatileList l, this->vScreens) {
		foreach(MetatileItem *i, l) {
			i->setMetatileIndex(0);
		}
	}
	this->updateScreen();
}



void StageManager::toggleShowScreenGrid(bool showgrid)
{
	this->bShowScreenGrid = showgrid;
	this->updateScreen();
}

void StageManager::toggleShowTileGrid(bool showgrid)
{
	this->bShowTileGrid = showgrid;
	this->updateScreen();
}



QVector<QByteArray> StageManager::createStageBinaryData()
{
	this->updateScreen();

	int numscreens = SM_SCREENS_W*SM_SCREENS_H;
	QVector<QByteArray> bindata = QVector<QByteArray>(numscreens);
	for(int s=0; s<numscreens; s++) {
		QByteArray bin;
		for(int tilewidth=0; tilewidth<SM_SCREEN_TILES_W; tilewidth++) {
			for(int tileheight=0; tileheight<SM_SCREEN_TILES_H; tileheight++) {
				bin.append(this->vScreens[s][(tileheight*SM_SCREEN_TILES_W)+tilewidth]->metatileIndex());
			}
		}
		bindata.replace(s,bin);
	}
	return bindata;
}

QString StageManager::createStageASMData(QString labelprefix)
{
	QString asmlabel = labelprefix.isEmpty()?"emptylabel":labelprefix;
	asmlabel += "_metatiles";
	QString databytes;

	int numscreens = SM_SCREENS_W*SM_SCREENS_H;
	for(int s=0; s<numscreens; s++) {
		QString countedlabel = asmlabel+QString("_%1").arg(s,2,16,QChar('0')).toUpper();

		databytes += countedlabel+":\n\t.byte ";

		QString screenbytes;
		for(int tilewidth=0; tilewidth<SM_SCREEN_TILES_W; tilewidth++) {
			for(int tileheight=0; tileheight<SM_SCREEN_TILES_H; tileheight++) {
				screenbytes += QString("$%1").arg(this->vScreens[s][(tileheight*SM_SCREEN_TILES_W)+tilewidth]->metatileIndex(),2,16,QChar('0')).toUpper().append(",");
			}
		}
		screenbytes = screenbytes.left(screenbytes.length()-1);
		databytes.append(screenbytes).append("\n");
	}

	databytes += asmlabel+"_end:\n";

	return databytes;
}



void StageManager::openStageFile(QString filename)
{
	QFile file(filename);
	if(!file.open(QIODevice::ReadOnly|QIODevice::Text)) {
		QMessageBox::warning(this,tr(SM_FILE_OPEN_ERROR_TITLE),tr(SM_FILE_OPEN_ERROR_BODY),QMessageBox::NoButton);
		return;
	}
	quint8 labelnum = 0;
	QVector<QByteArray> inputbytes(SM_SCREENS_W*SM_SCREENS_H);
	QString labelname;
	while(!file.atEnd()) {
		QString line = file.readLine();

		QRegularExpression label("^(.*?)_metatiles_([0-9a-fA-F]+?):$");
		QRegularExpressionMatch labelmatch = label.match(line);
		if(labelmatch.hasMatch()) {
			if(labelname.isEmpty()) {
				labelname = labelmatch.captured(1);
			}
			labelnum = labelmatch.captured(2).toUInt(NULL,16);
		}
		QRegularExpression bytes(",?\\$([0-9a-fA-F]+)");
		QRegularExpressionMatchIterator bytesiter = bytes.globalMatch(line);
		QByteArray bytesin;
		while(bytesiter.hasNext()) {
			QRegularExpressionMatch bytesmatch = bytesiter.next();
			bytesin.append(quint8(bytesmatch.captured(1).toUInt(NULL,16)));
		}
		if(!bytesin.isEmpty() && bytesin.size()==(SM_SCREEN_TILES_W*SM_SCREEN_TILES_H))
			inputbytes.replace(labelnum,bytesin);
	}
	if(!labelname.isEmpty()) {
		foreach(QByteArray test, inputbytes) {
			if(!test.isEmpty()) {
				this->importStageBinaryData(inputbytes);
				file.close();
				return;
			}
		}
	}

	file.close();
	QMessageBox::critical(this,tr(SM_INVALID_STAGE_TITLE),tr(SM_INVALID_STAGE_BODY),QMessageBox::NoButton);
}

void StageManager::importStageBinaryData(QVector<QByteArray> bindata)
{
	for(int s=0; s<(SM_SCREENS_W*SM_SCREENS_H); s++) {
		if(bindata[s].size()!=(SM_SCREEN_TILES_W*SM_SCREEN_TILES_H)) {
			QMessageBox::critical(this,tr(SM_COUNT_ERROR_TITLE),tr(SM_COUNT_ERROR_BODY),QMessageBox::NoButton);
			return;
		}
		for(int x=0; x<SM_SCREEN_TILES_W; x++) {
			for(int y=0; y<SM_SCREEN_TILES_H; y++) {
				this->vScreens[s][(y*SM_SCREEN_TILES_W)+x]->setMetatileIndex(bindata[s].at((x*SM_SCREEN_TILES_H)+y));
			}
		}
	}

	this->updateScreen();
}



void StageManager::refreshScreen()
{
	this->viewport()->update();
}
void StageManager::updateScreen()
{
	this->groupMetatiles->setScale(this->iScale);
	this->setSceneRect(0, 0, SM_CANVAS_WIDTH*this->iScale, SM_CANVAS_HEIGHT*this->iScale);
	this->drawGridLines();
	this->viewport()->update();
}
