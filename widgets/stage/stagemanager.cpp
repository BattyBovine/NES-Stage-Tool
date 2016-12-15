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
	this->openMetatileFile(e->mimeData()->urls()[0].toLocalFile());
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
		this->setSceneRect(0, 0,
						   SM_CANVAS_WIDTH*this->iScale,
						   SM_CANVAS_HEIGHT*this->iScale);
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
	this->pSceneTranslation.setX(qRound(this->transform().dx()/this->iScale));
	this->pSceneTranslation.setY(qRound(this->transform().dy()/this->iScale));

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



QVector<QByteArray> StageManager::createMetaspriteBinaryData()
{
	this->updateScreen();

	QVector<QByteArray> bindata = QVector<QByteArray>(256);
//	for(int i=0; i<256; i++) {
//		MetatileList mslist = this->vScreens[i];
//		QByteArray bin;
//		if(!mslist.isEmpty()) {
//			bin.append(quint8(mslist.length()));
//			for(int j=mslist.size()-1; j>=0; j--) {
//				MetatileItem *ms = mslist.at(j);
//				quint8 oamx = ms->realX();
//				quint8 oamy = ms->realY();
//				quint8 oamindex = ms->tileIndex()%this->iBankDivider;
//				quint8 oamattr = ms->palette()|(ms->flippedHorizontal()?0x40:0x00)|(ms->flippedVertical()?0x80:0x00);
//				bin.append(oamy);
//				bin.append(oamindex);
//				bin.append(oamattr);
//				bin.append(oamx);
//			}
//			bindata.replace(i,bin);
//		}
//	}
	return bindata;
}

QString StageManager::createMetaspriteASMData(QString labelprefix)
{
	QString asmlabel = labelprefix.isEmpty()?"emptylabel_":labelprefix;
	QString datatable_hi = asmlabel+"hi:\n\t.byte ";
	QString datatable_lo = asmlabel+"lo:\n\t.byte ";
	QString databanks = asmlabel+"bank:\n\t.byte ";
	QString databytes;

//	for(int i=0; i<256; i++) {
//		MetatileList mslist = this->vScreens[i];
//		if(mslist.isEmpty())    continue;
//		QString countedlabel = labelprefix+QString::number(i);

//		datatable_hi += QString(">").append(countedlabel).append(",");
//		datatable_lo += QString("<").append(countedlabel).append(",");

//		databytes += "\n";
//		databytes += countedlabel+":\n\t.byte ";
//		databytes += QString("$%1").arg(mslist.size(),2,16,QChar('0')).toUpper();
//		quint8 oamfullindex;
//		foreach(MetatileItem *mti, mslist) {
//			quint8 oamx = mti->realX();
//			quint8 oamy = mti->realY();
//			oamfullindex = (oamfullindex>mti->tileIndex()) ? oamfullindex : mti->tileIndex();
//			quint8 oamindex = mti->tileIndex()%this->iBankDivider;
//			quint8 oamattr = mti->palette()|(mti->flippedHorizontal()?0x40:0x00)|(mti->flippedVertical()?0x80:0x00);
//			databytes += QString(",$%1").arg(oamy,2,16,QChar('0')).toUpper();
//			databytes += QString(",$%1").arg(oamindex,2,16,QChar('0')).toUpper();
//			databytes += QString(",$%1").arg(oamattr,2,16,QChar('0')).toUpper();
//			databytes += QString(",$%1").arg(oamx,2,16,QChar('0')).toUpper();
//		}
//		databanks += QString("$%1").arg(int(floor(oamfullindex/this->iBankDivider)),2,16,QChar('0')).append(",");
//		oamfullindex = 0;
//	}

//	datatable_hi.remove(datatable_hi.size()-1,1);
//	datatable_lo.remove(datatable_lo.size()-1,1);
//	databanks.remove(databanks.size()-1,1);
//	datatable_hi += "\n";
//	datatable_lo += "\n";
//	databanks += "\n";
//	databytes += "\n";
//	databytes += asmlabel+"end:\n";

	return datatable_hi+datatable_lo+databanks+databytes;
}



void StageManager::openMetatileFile(QString filename)
{
	QFile file(filename);
	if(!file.open(QIODevice::ReadOnly|QIODevice::Text)) {
		QMessageBox::warning(this,tr(SM_FILE_OPEN_ERROR_TITLE),tr(SM_FILE_OPEN_ERROR_BODY),QMessageBox::NoButton);
		return;
	}
	quint8 labelnum = 0;
	QVector<QByteArray> inputbytes(256);
	QByteArray bankbytes;
	QString labelname;
	while(!file.atEnd()) {
		QString line = file.readLine();

		QRegularExpression banklabel("^(.*?)_bank:$");
		QRegularExpressionMatch banklabelmatch = banklabel.match(line);
		if(banklabelmatch.hasMatch()) {
			QString line = file.readLine();
			QRegularExpression bytes(",?\\$([0-9a-fA-F]+)");
			QRegularExpressionMatchIterator bytesiter = bytes.globalMatch(line);
			while(bytesiter.hasNext()) {
				QRegularExpressionMatch bytesmatch = bytesiter.next();
				bankbytes.append(quint8(bytesmatch.captured(1).toUInt(NULL,16)));
			}
		}

		QRegularExpression label("^(.*?)_(\\d+?):$");
		QRegularExpressionMatch labelmatch = label.match(line);
		if(labelmatch.hasMatch()) {
			if(labelname.isEmpty()) {
				labelname = labelmatch.captured(1);
				emit(this->setMetaspriteLabel(labelname));
			}
			labelnum = labelmatch.captured(2).toInt();
		}
		QRegularExpression bytes(",?\\$([0-9a-fA-F]+)");
		QRegularExpressionMatchIterator bytesiter = bytes.globalMatch(line);
		QByteArray bytesin;
		while(bytesiter.hasNext()) {
			QRegularExpressionMatch bytesmatch = bytesiter.next();
			bytesin.append(quint8(bytesmatch.captured(1).toUInt(NULL,16)));
		}
		if(!bytesin.isEmpty())  inputbytes.replace(labelnum,bytesin);
	}
	if(!labelname.isEmpty() && !bankbytes.isEmpty()) {
		foreach(QByteArray test, inputbytes) {
			if(!test.isEmpty()) {
				this->importMetatileBinaryData(inputbytes,bankbytes);
				file.close();
				return;
			}
		}
	}

//    file.reset();
//    QByteArray byteblob = file.readAll(), bytesin;
//    QByteArray::iterator i = byteblob.begin();
//    int loopcount = 0;
//    while(i!=byteblob.end()) {
//        bytesin.append(*i);
//        if((i+((*i)*4))>=byteblob.end()) {
//            QMessageBox::critical(this,tr(MSM_EOF_ERROR_TITLE),tr(MSM_EOF_ERROR_BODY),QMessageBox::NoButton);
//            return;
//        }
//        for(int count=*(i++); count>0; count--) {
//            for(int j=0; j<4; j++) {
//                bytesin.append(*(i++));
//            }
//        }
//        inputbytes.replace(loopcount++,bytesin);
//    }
//    QFileInfo fileinfo(filename);
//    emit(this->setMetaspriteLabel(fileinfo.baseName()));

//    this->importMetaspriteBinaryData(inputbytes);

	file.close();
	QMessageBox::critical(this,tr(SM_INVALID_SPRITES_TITLE),tr(SM_INVALID_SPRITES_BODY),QMessageBox::NoButton);
}

void StageManager::importMetatileBinaryData(QVector<QByteArray> bindata, QByteArray banks)
{
//	int blankcounter = 0;
//	for(int j=0; j<256; j++) {
//		QByteArray bin = bindata.at(j);
//		MetatileList mslist = this->vScreens.at(j);
//		mslist.clear();
//		QByteArray::iterator biniter = bin.begin();
//		for(int count = *biniter; count>0; count--) {
//			if((biniter+(count*4))>=bin.end()) {
//				QMessageBox::critical(this,tr(SM_COUNT_ERROR_TITLE),tr(SM_COUNT_ERROR_BODY),QMessageBox::NoButton);
//				return;
//			}
//			int oamy = *(++biniter);
//			quint8 oamindex = *(++biniter);
//			quint8 oamattr = *(++biniter);
//			int oamx = *(++biniter);
//			MetatileItem *ms = new MetaspriteTileItem();
//			ms->setFlags(QGraphicsItem::ItemIsMovable|QGraphicsItem::ItemIsSelectable);
//			ms->setShapeMode(QGraphicsPixmapItem::BoundingRectShape);
//			ms->setScale(this->iScale);
//			ms->setTallSprite(this->bTallSprites);
//			ms->setRealX(oamx);
//			ms->setRealY(oamy);
//			ms->setTileIndex((oamindex&(this->bTallSprites?0xFE:0xFF))+(this->iBankDivider*banks[j-blankcounter]));
//			ms->setPalette(oamattr&0x03);
//			emit(this->getTileUpdate(ms));
//			emit(this->getPaletteUpdate(ms));
//			mslist.append(ms);
//		}
//		this->vScreens.replace(j,mslist);
//		if(mslist.isEmpty()) blankcounter += 1;
//	}

//	QList<MetatileItem*> store = this->vScreens.at(this->iMetaspriteStage);
//	this->gsMetatiles->clear();
//	this->drawGridLines();
//	foreach(MetatileItem *ms, store) {
//		this->gsMetatiles->addItem(ms);
//	}

//	this->sendTileUpdates();
}



void StageManager::updateScreen()
{
	this->groupMetatiles->setScale(this->iScale);
	this->setSceneRect(0, 0,
					   SM_CANVAS_WIDTH*this->iScale,
					   SM_CANVAS_HEIGHT*this->iScale);
	this->drawGridLines();
	this->viewport()->update();
}
