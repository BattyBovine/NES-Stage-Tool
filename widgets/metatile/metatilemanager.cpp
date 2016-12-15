#include "metatilemanager.h"

MetatileManager::MetatileManager(QWidget *parent) : QGraphicsView(parent)
{
	this->gsMetatiles = new QGraphicsScene(this);
	this->setScene(this->gsMetatiles);
	this->iScale = MTM_DEFAULT_ZOOM;
	this->bSelectionMode = false;
	this->griSelection[0] = this->griSelection[1] = NULL;
	this->iSelectedTile = 0;
	this->bShowGrid8 = true;
	this->bShowGrid16 = true;

	this->groupMetatiles = new QGraphicsItemGroup();
	this->gsMetatiles->addItem(this->groupMetatiles);

	this->setSceneRect(0, 0,
					   MTM_CANVAS_SIZE,
					   MTM_CANVAS_SIZE);

	this->populateBlankTiles();
	this->drawGridLines();
	this->drawSelectionBox();
}

MetatileManager::~MetatileManager()
{
	this->gsMetatiles->clear();
	delete this->gsMetatiles;
}



void MetatileManager::resizeEvent(QResizeEvent *e)
{
	QRectF viewrect = this->mapToScene(this->rect()).boundingRect();
	this->iScale = qFloor(viewrect.width()/(MTI_TILEWIDTH*16));
	this->groupMetatiles->setScale(this->iScale);
	this->setSceneRect(0,0,
					   MTI_TILEWIDTH*16*this->iScale,
					   MTI_TILEWIDTH*16*this->iScale
					   );
	this->drawGridLines();
	this->drawSelectionBox();
}

void MetatileManager::dropEvent(QDropEvent *e)
{
	e->acceptProposedAction();
	this->openMetatileFile(e->mimeData()->urls()[0].toLocalFile());
}

void MetatileManager::mousePressEvent(QMouseEvent *e)
{
	QPointF p = this->mapToScene(e->pos());
	switch(e->button()) {
	case Qt::RightButton:
		if(!this->bSelectionMode) this->addNewSubtile(p);
		break;
	case Qt::MiddleButton:
		this->iMouseTranslateY = e->y();
		break;
	case Qt::LeftButton:
		this->pSelection = p;
		if(this->bSelectionMode) this->drawSelectionBox();
		break;
	default:
		QGraphicsView::mousePressEvent(e);
	}
}

void MetatileManager::mouseMoveEvent(QMouseEvent *e)
{
	QGraphicsView::mouseMoveEvent(e);

	if(e->buttons()&Qt::MiddleButton) {
		this->setTransformationAnchor(QGraphicsView::NoAnchor);
		this->translate(0,(e->y()-this->iMouseTranslateY));
		this->iMouseTranslateY = e->y();
	}
}

void MetatileManager::keyPressEvent(QKeyEvent *e)
{
	switch(e->key()) {
	case Qt::Key_Left:
	case Qt::Key_Right:
	case Qt::Key_Up:
	case Qt::Key_Down:
		break;
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



void MetatileManager::populateBlankTiles() {
	this->mtlMetatiles.clear();
	for(int y=0; y<16; y++) {
		for(int x=0; x<16; x++) {
			MetatileItem *i = new MetatileItem();
			i->setRealX(x*MTI_TILEWIDTH);
			i->setRealY(y*MTI_TILEWIDTH);
			i->setMetatileIndex((y*16)+x);
			this->mtlMetatiles.append(i);
			this->groupMetatiles->addToGroup(i);
		}
	}

	this->sendTileUpdates();
}

bool MetatileManager::drawSelectionBox()
{
	if(this->bSelectionMode) {
		int tilex = roundToMult(qRound(this->pSelection.x()/this->iScale),MTI_TILEWIDTH)/MTI_TILEWIDTH;
		int tiley = roundToMult(qRound(this->pSelection.y()/this->iScale),MTI_TILEWIDTH)/MTI_TILEWIDTH;
		if(tilex>=16||tiley>=16)	return false;

		if(this->griSelection[0] && this->griSelection[1]) {
			if(this->griSelection[0]->parentItem()) this->gsMetatiles->removeItem(this->griSelection[0]);
//			if(this->griSelection[1]->parentItem()) this->gsMetatiles->removeItem(this->griSelection[1]);
			delete this->griSelection[0];
//			delete this->griSelection[1];
			this->griSelection[0] = NULL;
//			this->griSelection[1] = NULL;
		}

		this->iSelectedTile = tiley*MTI_SUBTILEWIDTH+tilex;

		QPen dashes(Qt::red,1,Qt::DashLine);
		QVector<qreal> dp;
		dp << 2 << 1;
		dashes.setDashPattern(dp);
		QRectF rect = QRectF(tilex*MTI_TILEWIDTH*this->iScale,tiley*MTI_TILEWIDTH*this->iScale,(MTI_TILEWIDTH*this->iScale)-1,(MTI_TILEWIDTH*this->iScale)-1);
		this->griSelection[0] = this->gsMetatiles->addRect(rect,QPen(Qt::red),Qt::NoBrush);
//		this->griSelection[1] = this->gsMetatiles->addRect(rect,dashes,Qt::NoBrush);

		return true;
	}
	return false;
}

void MetatileManager::drawGridLines()
{
	foreach(QGraphicsLineItem *i, this->lGrid)
		this->gsMetatiles->removeItem(i);
	this->lGrid.clear();

	if(!this->bSelectionMode) {
		QPen thicksolid(Qt::black,MTM_THICK_GRID_LINES,Qt::SolidLine);
		QPen thickdashes(Qt::white,MTM_THICK_GRID_LINES,Qt::DashLine);
		QPen thinsolid(Qt::black,MTM_THIN_GRID_LINES,Qt::SolidLine);
		QPen thindashes(Qt::white,MTM_THIN_GRID_LINES,Qt::DashLine);
		QVector<qreal> dp;
		dp << 2 << 2;
		thickdashes.setDashPattern(dp);
		thindashes.setDashPattern(dp);

		qreal canvas = MTM_CANVAS_SIZE*this->iScale;

		if(this->bShowGrid8) {
			for(int i=0; i<=canvas; i+=MTI_TILEWIDTH*this->iScale/2) {
				this->lGrid.append(this->gsMetatiles->addLine(0,i,canvas,i,thinsolid));
				this->lGrid.append(this->gsMetatiles->addLine(0,i,canvas,i,thindashes));
				this->lGrid.append(this->gsMetatiles->addLine(i,0,i,canvas,thinsolid));
				this->lGrid.append(this->gsMetatiles->addLine(i,0,i,canvas,thindashes));
			}
		}
		if(this->bShowGrid16) {
			for(int i=0; i<=canvas; i+=MTI_TILEWIDTH*this->iScale) {
				this->lGrid.append(this->gsMetatiles->addLine(0,i,canvas,i,thicksolid));
				this->lGrid.append(this->gsMetatiles->addLine(0,i,canvas,i,thickdashes));
				this->lGrid.append(this->gsMetatiles->addLine(i,0,i,canvas,thicksolid));
				this->lGrid.append(this->gsMetatiles->addLine(i,0,i,canvas,thickdashes));
			}
		}
	}
}

void MetatileManager::setNewTileColours(PaletteVector c, quint8 p, bool s)
{
	this->gsMetatiles->setBackgroundBrush(QBrush(QColor(c.at(0))));

	QList<QGraphicsItem*> items = this->gsMetatiles->items(Qt::AscendingOrder);
	foreach(QGraphicsItem *ms, items) {
		if(ms->type()!=MetaspriteTileItem::Type)   continue;
		quint8 currentpal = qgraphicsitem_cast<MetaspriteTileItem*>(ms)->palette();
		qgraphicsitem_cast<MetaspriteTileItem*>(ms)->setNewColours(c.at((PM_PALETTE_COLOURS_MAX*currentpal)+1),
																   c.at((PM_PALETTE_COLOURS_MAX*currentpal)+2),
																   c.at((PM_PALETTE_COLOURS_MAX*currentpal)+3),
																   currentpal);
	}

	if(s) {
		items = this->gsMetatiles->selectedItems();
		foreach(QGraphicsItem *i, items) {
			((MetaspriteTileItem*)i)->setNewColours(c.at((PM_PALETTE_COLOURS_MAX*p)+1),
													c.at((PM_PALETTE_COLOURS_MAX*p)+2),
													c.at((PM_PALETTE_COLOURS_MAX*p)+3),
													p);
		}
	}
	this->sendTileUpdates();
}

void MetatileManager::addNewSubtile(QPointF p) {
	if(p.x()<0 || p.y()<0)	return;

	int tilex = roundToMult(qRound(p.x()/this->iScale),MTI_TILEWIDTH)/MTI_TILEWIDTH;
	int tiley = roundToMult(qRound(p.y()/this->iScale),MTI_TILEWIDTH)/MTI_TILEWIDTH;
	int index = tiley*qFloor(MTI_TILEWIDTH/this->iScale)+tilex;
	int subtilex = qFloor((qRound(p.x()/this->iScale)%MTI_TILEWIDTH)/MTI_SUBTILEWIDTH);
	int subtiley = qFloor((qRound(p.y()/this->iScale)%MTI_TILEWIDTH)/MTI_SUBTILEWIDTH);
	quint8 subtileindex = subtiley*(MTI_TILEWIDTH/MTI_SUBTILEWIDTH)+subtilex;

	emit(this->requestNewSubtile(subtileindex, this->mtlMetatiles[index]));
	emit(this->getPaletteUpdate(this->mtlMetatiles[index]));
}



void MetatileManager::updateMetatileStage() {
//	QList<QGraphicsItem*> items = this->gsMetatiles->items(Qt::AscendingOrder);
//	QList<MetatileItem*> store;
//	foreach(QGraphicsItem *i, items) {
//		if(i->type()!=MetatileItem::Type) {
//			this->gsMetatiles->removeItem(i);
//		} else {
//			MetatileItem *ms = qgraphicsitem_cast<MetatileItem*>(i);
//			store.append(ms);
//			this->gsMetatiles->removeItem(ms);
//		}
//	}
//	this->mtlMetatiles = store;

//	foreach(MetatileItem *ms, this->mtlMetatiles) {
//		ms->setX(ms->realX()*this->iScale);
//		ms->setY(ms->realY()*this->iScale);
//		ms->setScale(this->iScale);
//		this->gsMetatiles->addItem(ms);
//	}

	this->drawGridLines();
	this->drawSelectionBox();
	this->viewport()->update();
}

void MetatileManager::getMetatileEditorChange(quint8 i, MetatileItem *t) {
	MetatileItem *newtile = new MetatileItem(t);
	this->mtlMetatiles.replace(i,newtile);
//	emit(metatileUpdated(t));
}

void MetatileManager::getStageMetatile(MetatileItem *mtold)
{
//	quint8 index = mtold->metatileIndex();
//	MetatileItem *mtnew = new MetatileItem(this->mtlMetatiles.at(index));
//	emit(this->stageMetatileReady(mtold,mtnew));
}

void MetatileManager::getSelectedStageTile(MetatileItem *mtold)
{
	emit(this->selectedStageTileReady(mtold,this->iSelectedTile));
}



void MetatileManager::toggleShowGrid8(bool showgrid)
{
	this->bShowGrid8 = showgrid;
	this->drawGridLines();
}

void MetatileManager::toggleShowGrid16(bool showgrid)
{
	this->bShowGrid16 = showgrid;
	this->drawGridLines();
}

void MetatileManager::setBankDivider(int banksizeindex)
{
	this->iBankDivider = (0x100/(1<<banksizeindex));
//	emit(this->bankDividerChanged(this->iBankDivider));
}

void MetatileManager::setSelectedBank(quint16 bankno)
{
	this->iSelectedBank = bankno;
}



QVector<QByteArray> MetatileManager::createMetatileBinaryData()
{
	this->updateMetatileStage();

	QVector<QByteArray> bindata = QVector<QByteArray>(256);
	for(int i=0; i<256; i++) {
		MetatileList mslist = this->mtlMetatiles;
		QByteArray bin;
		if(!mslist.isEmpty()) {
			bin.append(quint8(mslist.length()));
			for(int j=mslist.size()-1; j>=0; j--) {
				MetatileItem *ms = mslist.at(j);
				quint8 oamx = ms->realX();
				quint8 oamy = ms->realY();
				quint8 oamindex = ms->tileIndex(0)%this->iBankDivider;
				quint8 oamattr = ms->palette();//|(ms->flippedHorizontal()?0x40:0x00)|(ms->flippedVertical()?0x80:0x00);
				bin.append(oamy);
				bin.append(oamindex);
				bin.append(oamattr);
				bin.append(oamx);
			}
			bindata.replace(i,bin);
		}
	}
	return bindata;
}

QString MetatileManager::createMetatileASMData(QString labelprefix)
{
	QString asmlabel = labelprefix.isEmpty()?"emptylabel_":labelprefix;
	QString datatable_hi = asmlabel+"hi:\n\t.byte ";
	QString datatable_lo = asmlabel+"lo:\n\t.byte ";
	QString databanks = asmlabel+"bank:\n\t.byte ";
	QString databytes;

	for(int i=0; i<256; i++) {
		MetatileList mslist = this->mtlMetatiles;
		if(mslist.isEmpty())    continue;
		QString countedlabel = labelprefix+QString::number(i);

		datatable_hi += QString(">").append(countedlabel).append(",");
		datatable_lo += QString("<").append(countedlabel).append(",");

		databytes += "\n";
		databytes += countedlabel+":\n\t.byte ";
		databytes += QString("$%1").arg(mslist.size(),2,16,QChar('0')).toUpper();
		quint8 oamfullindex;
		foreach(MetatileItem *mti, mslist) {
			quint8 oamx = mti->realX();
			quint8 oamy = mti->realY();
			oamfullindex = (oamfullindex>mti->tileIndex(0)) ? oamfullindex : mti->tileIndex(0);
			quint8 oamindex = mti->tileIndex(0)%this->iBankDivider;
			quint8 oamattr = mti->palette();//|(mti->flippedHorizontal()?0x40:0x00)|(mti->flippedVertical()?0x80:0x00);
			databytes += QString(",$%1").arg(oamy,2,16,QChar('0')).toUpper();
			databytes += QString(",$%1").arg(oamindex,2,16,QChar('0')).toUpper();
			databytes += QString(",$%1").arg(oamattr,2,16,QChar('0')).toUpper();
			databytes += QString(",$%1").arg(oamx,2,16,QChar('0')).toUpper();
		}
		databanks += QString("$%1").arg(int(floor(oamfullindex/this->iBankDivider)),2,16,QChar('0')).append(",");
		oamfullindex = 0;
	}

	datatable_hi.remove(datatable_hi.size()-1,1);
	datatable_lo.remove(datatable_lo.size()-1,1);
	databanks.remove(databanks.size()-1,1);
	datatable_hi += "\n";
	datatable_lo += "\n";
	databanks += "\n";
	databytes += "\n";
	databytes += asmlabel+"end:\n";

	return datatable_hi+datatable_lo+databanks+databytes;
}



void MetatileManager::openMetatileFile(QString filename)
{
	QFile file(filename);
	if(!file.open(QIODevice::ReadOnly|QIODevice::Text)) {
		QMessageBox::warning(this,tr(MTM_FILE_OPEN_ERROR_TITLE),tr(MTM_FILE_OPEN_ERROR_BODY),QMessageBox::NoButton);
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
//				this->importMetaspriteBinaryData(inputbytes,bankbytes);
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
//            QMessageBox::critical(this,tr(MTM_EOF_ERROR_TITLE),tr(MTM_EOF_ERROR_BODY),QMessageBox::NoButton);
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
	QMessageBox::critical(this,tr(MTM_INVALID_SPRITES_TITLE),tr(MTM_INVALID_SPRITES_BODY),QMessageBox::NoButton);
}

//void MetatileManager::importMetaspriteBinaryData(QVector<QByteArray> bindata, QByteArray banks)
//{
//	int blankcounter = 0;
//	for(int j=0; j<256; j++) {
//		QByteArray bin = bindata.at(j);
//		QList<MetatileItem*> mslist = this->vMetaspriteStages.at(j);
//		mslist.clear();
//		QByteArray::iterator biniter = bin.begin();
//		for(int count = *biniter; count>0; count--) {
//			if((biniter+(count*4))>=bin.end()) {
//				QMessageBox::critical(this,tr(MTM_COUNT_ERROR_TITLE),tr(MTM_COUNT_ERROR_BODY),QMessageBox::NoButton);
//				return;
//			}
//			int oamy = *(++biniter);
//			quint8 oamindex = *(++biniter);
//			quint8 oamattr = *(++biniter);
//			int oamx = *(++biniter);
//			MetatileItem *ms = new MetatileItem();
//			ms->setFlags(QGraphicsItem::ItemIsMovable|QGraphicsItem::ItemIsSelectable);
//			ms->setShapeMode(QGraphicsPixmapItem::BoundingRectShape);
//			ms->setScale(this->iScale);
//			ms->setTallSprite(this->bTallSprites);
//			ms->setRealX(oamx);
//			ms->setRealY(oamy);
//			ms->setTileIndex((oamindex&(this->bTallSprites?0xFE:0xFF))+(this->iBankDivider*banks[j-blankcounter]));
//			ms->setPalette(oamattr&0x03);
//			ms->flipHorizontal((oamattr&0x40)?true:false);
//			ms->flipVertical((oamattr&0x80)?true:false);
//			emit(this->getTileUpdate(ms));
//			emit(this->getPaletteUpdate(ms));
//			mslist.append(ms);
//		}
//		this->vMetaspriteStages.replace(j,mslist);
//		if(mslist.isEmpty()) blankcounter += 1;
//	}

//	QList<MetatileItem*> store = this->vMetaspriteStages.at(this->iMetaspriteStage);
//	this->gsMetasprite->clear();
//	this->drawGridLines();
//	foreach(MetaspriteTileItem *ms, store) {
//		this->gsMetasprite->addItem(ms);
//	}

//	this->sendTileUpdates();
//}

void MetatileManager::clearAllMetatileData()
{
	this->gsMetatiles->clear();
	this->griSelection[0] = NULL;
	this->griSelection[1] = NULL;

	foreach(MetatileItem *t, this->mtlMetatiles) {
		quint8 indices[4] = {0,0,0,0};
		t->setTileIndices(indices);
		t->setMetatileIndex(0);
		t->setPalette(0);
	}

	this->sendTileUpdates();
	this->drawGridLines();
	if(this->bSelectionMode) this->drawSelectionBox();
}



void MetatileManager::setSelectionMode(bool s)
{
	this->bSelectionMode=s;
	this->toggleShowGrid8(!s);
	this->toggleShowGrid16(!s);
	this->setSceneRect(0, 0,
					   MTM_CANVAS_SIZE*this->iScale,
					   MTM_CANVAS_SIZE*this->iScale);
}

void MetatileManager::sendTileUpdates(MetatileItem *t)
{
	if(t) {
		emit(this->getMetatileUpdate(t));
		emit(this->getPaletteUpdate(t));
	} else {
		foreach(MetatileItem *i, this->mtlMetatiles) {
			emit(this->getMetatileUpdate(i));
			emit(this->getPaletteUpdate(i));
		}
	}
}

void MetatileManager::getUpdatedMetatile(MetatileItem*)
{
	this->viewport()->update();
}
