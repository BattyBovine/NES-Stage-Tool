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



void MetatileManager::resizeEvent(QResizeEvent*)
{
	QRectF viewrect = this->mapToScene(this->rect()).boundingRect();
	this->iScale = qFloor(viewrect.width()/(MTI_TILEWIDTH*MTM_METATILES_W));
	this->groupMetatiles->setScale(this->iScale);
	this->setSceneRect(0,0,
					   MTI_TILEWIDTH*MTM_METATILES_W*this->iScale,
					   MTI_TILEWIDTH*MTM_METATILES_H*this->iScale
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
	for(int y=0; y<MTM_METATILES_H; y++) {
		for(int x=0; x<MTM_METATILES_W; x++) {
			MetatileItem *i = new MetatileItem();
			i->setRealX(x*MTI_TILEWIDTH);
			i->setRealY(y*MTI_TILEWIDTH);
			i->setMetatileIndex((y*MTM_METATILES_W)+x);
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
		if(tilex>=MTM_METATILES_W||tiley>=MTM_METATILES_H)	return false;

		if(this->griSelection[0]/* && this->griSelection[1]*/) {
			if(this->griSelection[0]->parentItem()) this->gsMetatiles->removeItem(this->griSelection[0]);
//			if(this->griSelection[1]->parentItem()) this->gsMetatiles->removeItem(this->griSelection[1]);
			delete this->griSelection[0];
//			delete this->griSelection[1];
			this->griSelection[0] = NULL;
//			this->griSelection[1] = NULL;
		}

		this->iSelectedTile = (tiley*MTM_METATILES_W)+tilex;

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
	if(p.x()<0 || p.y()<0 || p.x()>=(MTM_CANVAS_SIZE*this->iScale) || p.y()>=(MTM_CANVAS_SIZE*this->iScale))
		return;

	int tilex = roundToMult(qFloor(p.x()/this->iScale),MTI_TILEWIDTH)/MTI_TILEWIDTH;
	int tiley = roundToMult(qFloor(p.y()/this->iScale),MTI_TILEWIDTH)/MTI_TILEWIDTH;
	int index = (tiley*MTM_METATILES_W)+tilex;
	int subtilex = qFloor((qFloor(p.x()/this->iScale)%MTI_TILEWIDTH)/MTI_SUBTILEWIDTH);
	int subtiley = qFloor((qFloor(p.y()/this->iScale)%MTI_TILEWIDTH)/MTI_SUBTILEWIDTH);
	quint8 subtileindex = subtiley*(MTI_TILEWIDTH/MTI_SUBTILEWIDTH)+subtilex;

	emit(this->requestNewSubtile(subtileindex, this->mtlMetatiles[index]));
	emit(this->getPaletteUpdate(this->mtlMetatiles[index]));
}



void MetatileManager::updateMetatileStage()
{
	this->drawGridLines();
	this->drawSelectionBox();
	this->viewport()->update();
}

void MetatileManager::getMetatileEditorChange(quint8 i, MetatileItem *t)
{
	MetatileItem *newtile = new MetatileItem(t);
	this->mtlMetatiles.replace(i,newtile);
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

	QVector<QByteArray> bindata = QVector<QByteArray>(4);
	for(int i=0; i<this->mtlMetatiles.size(); i++) {
		bindata[0].append(this->mtlMetatiles[i]->tileIndex(0));
		bindata[1].append(this->mtlMetatiles[i]->tileIndex(1));
		bindata[2].append(this->mtlMetatiles[i]->tileIndex(2));
		bindata[3].append(this->mtlMetatiles[i]->tileIndex(3));
	}
	return bindata;
}

QString MetatileManager::createMetatileASMData(QString labelprefix)
{
	QString asmlabel = labelprefix.isEmpty()?"emptylabel":labelprefix;
	asmlabel += "_subtiles";
	QString databytes_tl = asmlabel+"_tl:\n\t.byte ";
	QString databytes_tr = asmlabel+"_tr:\n\t.byte ";
	QString databytes_bl = asmlabel+"_bl:\n\t.byte ";
	QString databytes_br = asmlabel+"_br:\n\t.byte ";

	for(int i=0; i<this->mtlMetatiles.size(); i++) {
		databytes_tl += QString("$%1").arg(this->mtlMetatiles[i]->tileIndex(0),2,16,QChar('0')).append(",").toUpper();
		databytes_bl += QString("$%1").arg(this->mtlMetatiles[i]->tileIndex(2),2,16,QChar('0')).append(",").toUpper();
		databytes_tr += QString("$%1").arg(this->mtlMetatiles[i]->tileIndex(1),2,16,QChar('0')).append(",").toUpper();
		databytes_br += QString("$%1").arg(this->mtlMetatiles[i]->tileIndex(3),2,16,QChar('0')).append(",").toUpper();
	}

	databytes_tl = databytes_tl.left(databytes_tl.length()-1);
	databytes_bl = databytes_bl.left(databytes_bl.length()-1);
	databytes_tr = databytes_tr.left(databytes_tr.length()-1);
	databytes_br = databytes_br.left(databytes_br.length()-1);

	databytes_tl += "\n";
	databytes_bl += "\n";
	databytes_tr += "\n";
	databytes_br += "\n";

	return databytes_tl+databytes_bl+databytes_tr+databytes_br;
}



void MetatileManager::openMetatileFile(QString filename)
{
	QFile file(filename);
	if(!file.open(QIODevice::ReadOnly|QIODevice::Text)) {
		QMessageBox::warning(this,tr(MTM_FILE_OPEN_ERROR_TITLE),tr(MTM_FILE_OPEN_ERROR_BODY),QMessageBox::NoButton);
		return;
	}
	quint8 labelnum = 0;
	QVector<QByteArray> inputbytes(4);
	while(!file.atEnd()) {
		QString line = file.readLine();

		QRegularExpression bytes(",?\\$([0-9a-fA-F]+)");
		QRegularExpressionMatchIterator bytesiter = bytes.globalMatch(line);
		QByteArray bytesin;
		while(bytesiter.hasNext()) {
			QRegularExpressionMatch bytesmatch = bytesiter.next();
			bytesin.append(quint8(bytesmatch.captured(1).toUInt(NULL,16)));
		}
		if(!bytesin.isEmpty() && bytesin.size()==(MTM_METATILES_W*MTM_METATILES_H)) {
			inputbytes.replace(labelnum,bytesin);
			labelnum++;
		}
	}
	if(!inputbytes.isEmpty()) {
		this->importMetatileBinaryData(inputbytes);
		file.close();
		return;
	}

	file.close();
	QMessageBox::critical(this,tr(MTM_INVALID_SPRITES_TITLE),tr(MTM_INVALID_SPRITES_BODY),QMessageBox::NoButton);
}

void MetatileManager::importMetatileBinaryData(QVector<QByteArray> bindata)
{
	if(bindata.size()!=4 ||
			bindata[0].size()!=(MTM_METATILES_W*MTM_METATILES_H) ||
			bindata[1].size()!=(MTM_METATILES_W*MTM_METATILES_H) ||
			bindata[2].size()!=(MTM_METATILES_W*MTM_METATILES_H) ||
			bindata[3].size()!=(MTM_METATILES_W*MTM_METATILES_H)) {
		QMessageBox::critical(this,tr(MTM_COUNT_ERROR_TITLE),tr(MTM_COUNT_ERROR_BODY),QMessageBox::NoButton);
		return;
	}
	for(int count=0; count<(MTM_METATILES_W*MTM_METATILES_H); count++) {
		this->mtlMetatiles[count]->setTileIndex(0,bindata[0].at(count));
		this->mtlMetatiles[count]->setTileIndex(2,bindata[1].at(count));
		this->mtlMetatiles[count]->setTileIndex(1,bindata[2].at(count));
		this->mtlMetatiles[count]->setTileIndex(3,bindata[3].at(count));
	}

	this->sendTileUpdates();
}

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
