#include "stagemanager.h"

StageManager::StageManager(QWidget *parent) : QGraphicsView(parent)
{
	this->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
	this->gsMetatiles = new QGraphicsScene(this);
	this->gsMetatiles->setItemIndexMethod(QGraphicsScene::NoIndex);

	this->setScene(this->gsMetatiles);
	this->iScale = SM_DEFAULT_ZOOM;
	this->bShowScreenGrid = this->bShowTileGrid = true;
	this->iSelectedTileset = 0;
	this->setBackgroundBrush(QBrush(Qt::black));

	this->vScreens = ScreenList(SM_SCREENS_W*SM_SCREENS_H);

	this->groupMetatiles = new QGraphicsItemGroup();
	this->gsMetatiles->addItem(this->groupMetatiles);

	this->pMouseTranslation = QPoint(-1,-1);
	this->pRightMousePos = QPoint(-1,-1);
	this->pSceneTranslation = QPoint(-1,-1);

	this->populateBlankTiles();

	this->updateStageView();
}

StageManager::~StageManager()
{
	this->gsMetatiles->clear();
	this->gsMetatiles->deleteLater();
}



void StageManager::dropEvent(QDropEvent *e)
{
	e->acceptProposedAction();
	emit(stageFileDropped(e->mimeData()->urls()[0].toLocalFile()));
//	this->openStageFile(e->mimeData()->urls()[0].toLocalFile());
}

void StageManager::mousePressEvent(QMouseEvent *e)
{
	switch(e->button()) {
	case Qt::RightButton:
		this->pRightMousePos = QPointF(-1,-1);
		this->replaceStageTile(this->mapToScene(e->pos()));
		break;
	case Qt::MiddleButton:
		this->pMouseTranslation = QPointF(e->x(),e->y());
		break;
	case Qt::LeftButton:
		this->replaceScreenTileset(this->mapToScene(e->pos()));
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
		this->replaceStageTile(this->mapToScene(e->pos()));
	}
}

void StageManager::mouseDoubleClickEvent(QMouseEvent *e)
{
	switch(e->button()) {
	case Qt::MiddleButton:
		this->iScale = SM_DEFAULT_ZOOM;
		this->setSceneRect(0, 0, SM_CANVAS_WIDTH*this->iScale, SM_CANVAS_HEIGHT*this->iScale);
		this->updateStageView();
		break;
//	case Qt::RightButton:
//		this->replaceAllScreenTiles(this->mapToScene(e->pos()));
//		break;
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

	this->updateStageView();
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
}

void StageManager::replaceStageTile(QPointF p)
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
	this->updateStageView();
}

void StageManager::replaceScreenTileset(QPointF p)
{
	if(p.x()<0 || p.y()<0)	return;

	int tilex = roundToMult(qRound(p.x()/this->iScale),MTI_TILEWIDTH)/MTI_TILEWIDTH;
	int tiley = roundToMult(qRound(p.y()/this->iScale),MTI_TILEWIDTH)/MTI_TILEWIDTH;
	int screenx = qFloor(tilex/SM_SCREEN_TILES_W);
	int screeny = qFloor(tiley/SM_SCREEN_TILES_H);
	quint8 screen = (screeny*SM_SCREENS_W)+screenx;
//	quint8 tile = ((tiley%SM_SCREEN_TILES_H)*SM_SCREEN_TILES_W)+(tilex%SM_SCREEN_TILES_W);

	foreach(MetatileItem* t, this->vScreens[screen]) {
		t->setTileset(this->iSelectedTileset);
	}
	this->updateStageView();
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
	this->updateStageView();
}

void StageManager::clearAllMetatileData()
{
	quint8 tileindices[4] = {0,0,0,0};
	foreach(MetatileList l, this->vScreens) {
		foreach(MetatileItem *i, l) {
			i->setTileset(0);
			i->setTileIndices(tileindices);
			i->setMetatileIndex(0);
			i->setPalette(0);
		}
	}
	this->updateStageView();
}



void StageManager::getNewTile(MetatileItem *mtold, MetatileItem *mtnew)
{
	this->vScreens[mtold->screen()][mtold->metatileIndex()]->setPalette(mtnew->palette());
	this->vScreens[mtold->screen()][mtold->metatileIndex()]->setTileIndices(mtnew->tileIndices());
}

void StageManager::getUpdatedTile(MetatileItem *mtnew)
{
	foreach(MetatileList l, this->vScreens) {
		foreach(MetatileItem *t, l) {
			if(t->metatileIndex() == mtnew->metatileIndex()) {
				t->setPalette(mtnew->palette());
				t->setTileIndices(mtnew->tileIndices());
			}
		}
	}
}

void StageManager::getSelectedTileset(quint8 ts)
{
	this->iSelectedTileset = ts;
}

void StageManager::getNewAnimationFrame(int animframe)
{
	foreach(MetatileList l, this->vScreens) {
		foreach(MetatileItem *t, l)
			t->setAnimFrame(animframe);
	}
	this->viewport()->update();
}

void StageManager::toggleShowScreenGrid(bool showgrid)
{
	this->bShowScreenGrid = showgrid;
	this->drawGridLines();
}

void StageManager::toggleShowTileGrid(bool showgrid)
{
	this->bShowTileGrid = showgrid;
	this->drawGridLines();
}



QVector<QByteArray> StageManager::createStageBinaryData()
{
	int numscreens = SM_SCREENS_W*SM_SCREENS_H;
	QVector<QByteArray> bindata = QVector<QByteArray>(numscreens);
	for(int s=0; s<numscreens; s++) {
		// Create metatile data (vertical columns)
		QByteArray bin;
		for(int tilewidth=0; tilewidth<SM_SCREEN_TILES_W; tilewidth++) {
			for(int tileheight=0; tileheight<SM_SCREEN_TILES_H; tileheight++) {
				bin.append(this->vScreens[s][(tileheight*SM_SCREEN_TILES_W)+tilewidth]->metatileIndex());
			}
		}
		bindata.replace(s,bin);

		// Create attribute data (vertical columns)
		quint8 attributecolumn[qFloor(SM_SCREEN_TILES_H/2)];
		bin.clear();
		for(int i=0; i<qFloor(SM_SCREEN_TILES_H/2); i++) attributecolumn[i] = 0;
		for(int tilewidth=0; tilewidth<SM_SCREEN_TILES_W; tilewidth+=2) {
			for(int tileheight=0; tileheight<SM_SCREEN_TILES_H; tileheight+=2) {
				quint8 attr0 = (this->vScreens[s][(tileheight*SM_SCREEN_TILES_W)+tilewidth]->palette()%PM_SUBPALETTES_MAX);
				quint8 attr1 = (this->vScreens[s][(tileheight*SM_SCREEN_TILES_W)+tilewidth+1]->palette()%PM_SUBPALETTES_MAX);
				quint8 attr2 = (this->vScreens[s][((tileheight+1)*SM_SCREEN_TILES_W)+tilewidth]->palette()%PM_SUBPALETTES_MAX);
				quint8 attr3 = (this->vScreens[s][((tileheight+1)*SM_SCREEN_TILES_W)+tilewidth+1]->palette()%PM_SUBPALETTES_MAX);
				attributecolumn[qFloor(tileheight/qFloor(SM_SCREEN_TILES_H/2))%8] = ((attr0)|(attr1<<2)|(attr2<<4)|(attr3<<6));
			}
			for(int i=0; i<qFloor(SM_SCREEN_TILES_H/2); i++) {
				bin.append(attributecolumn[i]);
			}
		}
	}
	return bindata;
}

QString StageManager::createStageASMData(QString labelprefix)
{
	QString asmlabel = labelprefix.isEmpty()?"emptylabel":labelprefix;
	asmlabel += "_metatiles";
	QString databytes;
//	QString attrbytes;
	QString tilesetbytes = labelprefix+QString("_tilesets:\n\t.byte ");
	quint8 tilesetbyte = 0x00;

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

		tilesetbyte |= (this->vScreens[s][0]->tileset()&0x0F)<<(4*(s&0x01));
		if((s&0x01)) {
			tilesetbytes += QString("$%1").arg(tilesetbyte,2,16,QChar('0')).toUpper().append(",");
			tilesetbyte = 0x00;
		}

		// Create attribute data (vertical columns)
//		QString attrcountedlabel = labelprefix+QString("_attributes")+QString("_%1").arg(s,2,16,QChar('0')).toUpper();
//		attrbytes = attrcountedlabel+":\n\t.byte ";
//		quint8 attributecolumn[qFloor(SM_SCREEN_TILES_H/2)];
//		for(int i=0; i<qFloor(SM_SCREEN_TILES_H/2); i++) attributecolumn[i] = 0;
//		for(int tilewidth=0; tilewidth<SM_SCREEN_TILES_W; tilewidth+=2) {
//			for(int tileheight=0; tileheight<SM_SCREEN_TILES_H; tileheight+=2) {
//				quint8 attr0index = (tileheight*SM_SCREEN_TILES_W)+tilewidth;
//				quint8 attr1index = (tileheight*SM_SCREEN_TILES_W)+tilewidth+1;
//				quint8 attr2index = ((tileheight+1)*SM_SCREEN_TILES_W)+tilewidth;
//				quint8 attr3index = ((tileheight+1)*SM_SCREEN_TILES_W)+tilewidth+1;

//				quint8 attr0 = (this->vScreens[s][attr0index]->palette()%PM_SUBPALETTES_MAX);
//				quint8 attr1 = (this->vScreens[s][attr1index]->palette()%PM_SUBPALETTES_MAX);
//				quint8 attr2 = (this->vScreens[s][attr2index]->palette()%PM_SUBPALETTES_MAX);
//				quint8 attr3 = (this->vScreens[s][attr3index]->palette()%PM_SUBPALETTES_MAX);
//				quint8 attrindex = qFloor(tileheight/2)%qFloor(SM_SCREEN_TILES_H/2);
//				attributecolumn[attrindex] = ((attr0)|(attr1<<2)|(attr2<<4)|(attr3<<6));
//			}
//			attrbytes += QString("$%1").arg(attributecolumn[0],2,16,QChar('0')).toUpper().append(",");
//			attrbytes += QString("$%1").arg(attributecolumn[4],2,16,QChar('0')).toUpper().append(",");
//			attrbytes += QString("$%1").arg(attributecolumn[1],2,16,QChar('0')).toUpper().append(",");
//			attrbytes += QString("$%1").arg(attributecolumn[5],2,16,QChar('0')).toUpper().append(",");
//			attrbytes += QString("$%1").arg(attributecolumn[2],2,16,QChar('0')).toUpper().append(",");
//			attrbytes += QString("$%1").arg(attributecolumn[3],2,16,QChar('0')).toUpper().append(",");
//		}
//		attrbytes = attrbytes.left(attrbytes.length()-1);
//		databytes.append(attrbytes).append("\n");
	}

	databytes += asmlabel+"_end:\n";

	tilesetbytes = tilesetbytes.left(tilesetbytes.length()-1) + QString("\n");

	return databytes+tilesetbytes;
}



void StageManager::openStageFile(QString filename)
{
	QFile file(filename);
	if(!file.open(QIODevice::ReadOnly|QIODevice::Text)) {
		QMessageBox::warning(this,tr(SM_FILE_OPEN_ERROR_TITLE),tr(SM_FILE_OPEN_ERROR_BODY),QMessageBox::NoButton);
		return;
	}
	quint8 labelnum = 0;
	QVector<QByteArray> inputbytes(SM_SCREENS_W*SM_SCREENS_H+1);
	QString labelname;
	bool tilesetsfound = false;

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

		if(!bytesin.isEmpty() && bytesin.size()==(SM_SCREEN_TILES_W*SM_SCREEN_TILES_H)) {
			inputbytes.replace(labelnum,bytesin);
		} else if(!bytesin.isEmpty() && tilesetsfound && bytesin.size()==qFloor((SM_SCREENS_W*SM_SCREENS_H)/2)) {
			inputbytes.append(bytesin);
			tilesetsfound = false;
		}

		QRegularExpression tilesetlabel("^(.*?)_tilesets:$");
		QRegularExpressionMatch tilesetlabelmatch = tilesetlabel.match(line);
		if(tilesetlabelmatch.hasMatch()) {
			tilesetsfound = true;
		}
	}
	file.close();
	if(!labelname.isEmpty()) {
		emit(setStageLabel(labelname));
		foreach(QByteArray test, inputbytes) {
			if(!test.isEmpty()) {
				this->importStageBinaryData(inputbytes);
				return;
			}
		}
	}

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
				emit(requestTileUpdate(this->vScreens[s][(y*SM_SCREEN_TILES_W)+x]));
			}
		}
	}

	for(int s=0; s<(SM_SCREENS_W*SM_SCREENS_H); s++) {
		if(bindata.last().size()!=qFloor((SM_SCREENS_W*SM_SCREENS_H)/2)) {
			QMessageBox::critical(this,tr(SM_COUNT_ERROR_TITLE),tr(SM_COUNT_ERROR_BODY),QMessageBox::NoButton);
			return;
		}
		quint8 tileset = ((bindata.last().at((s>>1))>>(4*(s&0x01)))&0x0F);
		foreach(MetatileItem *t, this->vScreens[s])
			t->setTileset(tileset);
	}
}



void StageManager::updateStageView()
{
	this->groupMetatiles->setScale(this->iScale);
	this->setSceneRect(0, 0, SM_CANVAS_WIDTH*this->iScale, SM_CANVAS_HEIGHT*this->iScale);
	this->drawGridLines();
	this->viewport()->update();
}
