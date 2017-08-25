#include "metatilemanager.h"

MetatileManager::MetatileManager(QWidget *parent) : QGraphicsView(parent)
{
	this->gsMetatiles = new QGraphicsScene(this);
	this->setScene(this->gsMetatiles);
	this->iScale = MTM_DEFAULT_ZOOM;
	this->bSelectionMode = false;
	this->griSelection[0] = this->griSelection[1] = NULL;
	this->iGlobalTileset = this->iSelectedSubtile = this->iSelectedTile = this->iSelectedPalette = 0;
	this->bShowGrid8 = true;
    this->bShowGrid16 = true;

	this->groupMetatiles = new QGraphicsItemGroup();
	this->gsMetatiles->addItem(this->groupMetatiles);

	this->undoMetatiles = new QUndoStack(this);

	this->setSceneRect(0, 0, MTM_CANVAS_SIZE, MTM_CANVAS_SIZE);

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
	this->iScale = qFloor(qMin(viewrect.width(),viewrect.height())/(MTI_TILEWIDTH*MTM_METATILES_W));
	this->groupMetatiles->setScale(this->iScale);
	this->setSceneRect(0,0, MTI_TILEWIDTH*MTM_METATILES_W*this->iScale, MTI_TILEWIDTH*MTM_METATILES_H*this->iScale);
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
	case Qt::LeftButton:
		if(this->bSelectionMode) {
			this->pSelection = QPointF(qFloor(p.x()/this->iScale),qFloor(p.y()/this->iScale));
			this->drawSelectionBox();
		} else {
			this->addNewSubtile(p);
		}
		break;
	case Qt::MiddleButton:
		this->pMouseTranslation = QPoint(e->x(),e->y());
		break;
	case Qt::RightButton:
		if(!this->bSelectionMode)
			this->applySelectedPalette(p);
		break;
	default:
		QGraphicsView::mousePressEvent(e);
	}
}
void MetatileManager::mouseDoubleClickEvent(QMouseEvent *e)
{
	this->mousePressEvent(e);
}

void MetatileManager::mouseMoveEvent(QMouseEvent *e)
{
	QGraphicsView::mouseMoveEvent(e);

	if(e->buttons()&Qt::MiddleButton) {
		this->setTransformationAnchor(QGraphicsView::NoAnchor);
		this->translate(0,(e->y()-this->pMouseTranslation.y()));
		this->pMouseTranslation = QPoint(e->x(),e->y());
	}
}

void MetatileManager::wheelEvent(QWheelEvent *e)
{
	if(this->bSelectionMode) {
		int steps = -qFloor(((qreal)e->angleDelta().y()/8)/15);
		this->iGlobalTileset = ((this->iGlobalTileset+steps)<0)?0:((this->iGlobalTileset+steps)>7)?7:(this->iGlobalTileset+steps);
		foreach(MetatileItem *t, this->mtlMetatiles) t->setTileset(this->iGlobalTileset);
		emit(sendSelectedTileset(this->iGlobalTileset));
		this->updateScreen();
	} else {
		QGraphicsView::wheelEvent(e);
	}
}



void MetatileManager::undo()
{
	this->undoMetatiles->undo();
}

void MetatileManager::redo()
{
	this->undoMetatiles->redo();
}

void MetatileManager::clearUndoHistory()
{
	this->undoMetatiles->clear();
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
}

bool MetatileManager::drawSelectionBox()
{
	if(this->bSelectionMode) {
		if(this->griSelection[0]/* && this->griSelection[1]*/) {
			if(this->griSelection[0]->parentItem()) this->gsMetatiles->removeItem(this->griSelection[0]);
//			if(this->griSelection[1]->parentItem()) this->gsMetatiles->removeItem(this->griSelection[1]);
			delete this->griSelection[0];
//			delete this->griSelection[1];
			this->griSelection[0] = NULL;
//			this->griSelection[1] = NULL;
		}

        if(this->pSelection.x()<0 || this->pSelection.y()<0 ||
                this->pSelection.x()>=MTM_CANVAS_SIZE ||
                this->pSelection.y()>=MTM_CANVAS_SIZE)
            return false;

        quint8 tilex = qFloor(this->pSelection.x()/MTI_TILEWIDTH);
        quint8 tiley = qFloor(this->pSelection.y()/MTI_TILEWIDTH);
        if(tilex>=MTM_METATILES_W||tiley>=MTM_METATILES_H)	return false;

		this->iSelectedTile = (tiley*MTM_METATILES_W)+tilex;

		QPen dashes(Qt::red,1,Qt::DashLine);
		QVector<qreal> dp;
		dp << 2 << 1;
		dashes.setDashPattern(dp);
		QRectF rect = QRectF(tilex*MTI_TILEWIDTH*this->iScale,tiley*MTI_TILEWIDTH*this->iScale,(MTI_TILEWIDTH*this->iScale)-1,(MTI_TILEWIDTH*this->iScale)-1);
		this->griSelection[0] = this->gsMetatiles->addRect(rect,QPen(Qt::red),Qt::NoBrush);
//		this->griSelection[1] = this->gsMetatiles->addRect(rect,dashes,Qt::NoBrush);

        emit(this->sendSelectionProperties(this->mtlMetatiles[this->iSelectedTile]->collision(),
                this->mtlMetatiles[this->iSelectedTile]->destructible(),
                this->mtlMetatiles[this->iSelectedTile]->deadly()));

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

void MetatileManager::setSelectedSubtile(int s)
{
	this->iSelectedSubtile = s;
}

void MetatileManager::setNewTileColours(PaletteVector c, quint8 p, bool/* s*/)
{
	this->gsMetatiles->setBackgroundBrush(QBrush(QColor(c[this->iGlobalTileset*(PM_SUBPALETTES_MAX*PM_PALETTE_COLOURS_MAX)])));
	this->iSelectedPalette = p;
	this->viewport()->update();
}

void MetatileManager::addNewSubtile(QPointF p) {
	if(p.x()<0 || p.y()<0 || p.x()>=(MTM_CANVAS_SIZE*this->iScale) || p.y()>=(MTM_CANVAS_SIZE*this->iScale))
		return;

    int tilex = qFloor(p.x()/this->iScale)/MTI_TILEWIDTH;
    int tiley = qFloor(p.y()/this->iScale)/MTI_TILEWIDTH;
	int index = (tiley*MTM_METATILES_W)+tilex;
	int subtilex = qFloor((qFloor(p.x()/this->iScale)%MTI_TILEWIDTH)/MTI_SUBTILEWIDTH);
	int subtiley = qFloor((qFloor(p.y()/this->iScale)%MTI_TILEWIDTH)/MTI_SUBTILEWIDTH);
	quint8 subtileindex = subtiley*(MTI_TILEWIDTH/MTI_SUBTILEWIDTH)+subtilex;

	this->undoMetatiles->push(new ChangeMetatile(this->mtlMetatiles[index],subtileindex,this->iSelectedSubtile,this->iSelectedPalette));

	emit(sendMetatileToSelector(this->mtlMetatiles[index]));
	emit(metatileUpdated(this->mtlMetatiles[index]));
	this->updateScreen();
}

void MetatileManager::applySelectedPalette(QPointF p) {
	if(p.x()<0 || p.y()<0 || p.x()>=(MTM_CANVAS_SIZE*this->iScale) || p.y()>=(MTM_CANVAS_SIZE*this->iScale))
		return;

    int tilex = qFloor(p.x()/this->iScale)/MTI_TILEWIDTH;
    int tiley = qFloor(p.y()/this->iScale)/MTI_TILEWIDTH;
	int index = (tiley*MTM_METATILES_W)+tilex;

	this->undoMetatiles->push(new ChangeMetatile(this->mtlMetatiles[index],this->iSelectedPalette%PM_SUBPALETTES_MAX));

	emit(sendMetatileToSelector(this->mtlMetatiles[index]));
	emit(metatileUpdated(this->mtlMetatiles[index]));
	this->updateScreen();
}

void MetatileManager::getEditorMetatile(MetatileItem *t)
{
	this->mtlMetatiles[t->metatileIndex()]->setPalette(t->palette());
    this->mtlMetatiles[t->metatileIndex()]->setTileIndices(t->tileIndices());
    this->mtlMetatiles[t->metatileIndex()]->setAnimFrame(t->animFrame());
	this->updateScreen();
}

void MetatileManager::getSelectedStageTile(MetatileItem *mtold)
{
	ChangeStageTile *change = new ChangeStageTile(mtold,this->mtlMetatiles[this->iSelectedTile]);
	emit(changeStageTile(change));
}

void MetatileManager::updateStageMetatile(MetatileItem *mtold)
{
	MetatileItem *copyfrom = this->mtlMetatiles[mtold->metatileIndex()];
	mtold->setPalette(copyfrom->palette());
	mtold->setTileIndices(copyfrom->tileIndices());
	this->updateScreen();
}

void MetatileManager::getNewAnimationFrame(int animframe)
{
	foreach(MetatileItem *t, this->mtlMetatiles) {
		t->setAnimFrame(animframe);
		emit(sendMetatileToSelector(t));
	}
	this->updateScreen();
}



void MetatileManager::setMetatileProperties(int i, int col, bool des, bool ded)
{
    this->mtlMetatiles[i]->setCollision(col);
    this->mtlMetatiles[i]->setDestructible(des);
    this->mtlMetatiles[i]->setDeadly(ded);
}

void MetatileManager::setGlobalTileset(int i)
{
	this->iGlobalTileset = i;
	if(!this->bSelectionMode) {
		foreach(MetatileItem *t, this->mtlMetatiles) t->setTileset(i);
	}
}



QVector<QByteArray> MetatileManager::createMetatileBinaryData()
{
	QVector<QByteArray> bindata = QVector<QByteArray>(5);
	quint8 palettecompressed[4] = {0,0,0,0};
	for(int i=0; i<this->mtlMetatiles.size(); i++) {
		bindata[0].append(this->mtlMetatiles[i]->tileIndex(0));
		bindata[1].append(this->mtlMetatiles[i]->tileIndex(1));
		bindata[2].append(this->mtlMetatiles[i]->tileIndex(2));
		bindata[3].append(this->mtlMetatiles[i]->tileIndex(3));
		palettecompressed[i%4] = (this->mtlMetatiles[i]->palette()&0x03)<<((i%4)*2);
		if((i%4)==3)
			bindata[4].append(palettecompressed[0] | palettecompressed[1] | palettecompressed[2] | palettecompressed[3]);
	}
	return bindata;
}

QString MetatileManager::createMetatileASMData(QString labelprefix)
{
    QString asmlabel = labelprefix.isEmpty()?"emptylabel":labelprefix;
    QString databytes_tl = asmlabel+"_subtiles_tl:\n\t.byte ";
    QString databytes_tr = asmlabel+"_subtiles_tr:\n\t.byte ";
    QString databytes_bl = asmlabel+"_subtiles_bl:\n\t.byte ";
    QString databytes_br = asmlabel+"_subtiles_br:\n\t.byte ";
    QString databytes_p  = asmlabel+"_mtpalettes:\n\t.byte ";
    QString databytes_prop = asmlabel+"_mtprops:\n\t.byte ";

	quint8 palettecompressed[4] = {0,0,0,0};
//    quint8 propertiescompressed[2] = {0,0};
	for(int i=0; i<this->mtlMetatiles.size(); i++) {
		databytes_tl += QString("$%1").arg(this->mtlMetatiles[i]->tileIndex(0),2,16,QChar('0')).append(",").toUpper();
		databytes_bl += QString("$%1").arg(this->mtlMetatiles[i]->tileIndex(2),2,16,QChar('0')).append(",").toUpper();
		databytes_tr += QString("$%1").arg(this->mtlMetatiles[i]->tileIndex(1),2,16,QChar('0')).append(",").toUpper();
		databytes_br += QString("$%1").arg(this->mtlMetatiles[i]->tileIndex(3),2,16,QChar('0')).append(",").toUpper();

		palettecompressed[i%4] = (this->mtlMetatiles[i]->palette()&0x03)<<((i%4)*2);
		if((i%4)==3)
			databytes_p += QString("$%1").arg((palettecompressed[0] | palettecompressed[1] | palettecompressed[2] | palettecompressed[3]),2,16,QChar('0')).append(",").toUpper();

//        propertiescompressed[i%2] = (this->mtlMetatiles[i]->collision()|(this->mtlMetatiles[i]->destructible()?0x04:0x00)|(this->mtlMetatiles[i]->deadly()?0x08:0x00))<<((i%2)*4);
//        if((i%2)==1)
//            databytes_prop += QString("$%1").arg((propertiescompressed[0] | propertiescompressed[1]),2,16,QChar('0')).append(",").toUpper();
		databytes_prop += QString("$%1").arg((this->mtlMetatiles[i]->collision()|(this->mtlMetatiles[i]->destructible()?0x40:0x00)|(this->mtlMetatiles[i]->deadly()?0x80:0x00)),2,16,QChar('0')).append(",").toUpper();
	}

	databytes_tl = databytes_tl.left(databytes_tl.length()-1);
	databytes_bl = databytes_bl.left(databytes_bl.length()-1);
	databytes_tr = databytes_tr.left(databytes_tr.length()-1);
    databytes_br = databytes_br.left(databytes_br.length()-1);
    databytes_p = databytes_p.left(databytes_p.length()-1);
    databytes_prop = databytes_prop.left(databytes_prop.length()-1);

	databytes_tl += "\n";
	databytes_bl += "\n";
	databytes_tr += "\n";
    databytes_br += "\n";
    databytes_p += "\n";
    databytes_prop += "\n";

    return databytes_tl+databytes_bl+databytes_tr+databytes_br+databytes_p+databytes_prop;
}



void MetatileManager::openMetatileFile(QString filename)
{
	QFile file(filename);
	if(!file.open(QIODevice::ReadOnly|QIODevice::Text)) {
		QMessageBox::warning(this,tr(MTM_FILE_OPEN_ERROR_TITLE),tr(MTM_FILE_OPEN_ERROR_BODY),QMessageBox::NoButton);
		return;
	}
	quint8 labelnum = 0;
	bool subtilesfound = false;
	bool palettesfound = false;
	bool propsfound = false;
    QVector<QByteArray> inputbytes(6);
	while(!file.atEnd()) {
		QString line = file.readLine();

		QRegularExpression bytes(",?\\$([0-9a-fA-F]+)");
		QRegularExpressionMatchIterator bytesiter = bytes.globalMatch(line);
		QByteArray bytesin;
		while(bytesiter.hasNext()) {
			QRegularExpressionMatch bytesmatch = bytesiter.next();
			bytesin.append(quint8(bytesmatch.captured(1).toUInt(NULL,16)));
		}

		QRegularExpression subtileslabel("^(.*?)_subtiles_(tl|bl|tr|br):$");
		QRegularExpressionMatch subtileslabelmatch = subtileslabel.match(line);
		if(subtileslabelmatch.hasMatch()) {
			subtilesfound = true;
		}

		QRegularExpression paletteslabel("^(.*?)_mtpalettes:$");
		QRegularExpressionMatch paletteslabelmatch = paletteslabel.match(line);
		if(paletteslabelmatch.hasMatch()) {
			palettesfound = true;
		}

		QRegularExpression propslabel("^(.*?)_mtprops:$");
		QRegularExpressionMatch propslabelmatch = propslabel.match(line);
		if(propslabelmatch.hasMatch()) {
			propsfound = true;
		}

		if(!bytesin.isEmpty() && (subtilesfound || palettesfound || propsfound)) {
			inputbytes.replace(labelnum,bytesin);
			subtilesfound = false;
			palettesfound = false;
			propsfound = false;
			labelnum++;
        }
	}

	file.close();
	if(!inputbytes.isEmpty()) {
		this->importMetatileBinaryData(inputbytes);
		return;
	}

	QMessageBox::critical(this,tr(MTM_INVALID_SPRITES_TITLE),tr(MTM_INVALID_SPRITES_BODY),QMessageBox::NoButton);
}

void MetatileManager::importMetatileBinaryData(QVector<QByteArray> bindata)
{
    if(bindata.size()!=6 ||
			bindata[0].size()!=(MTM_METATILES_W*MTM_METATILES_H) ||
			bindata[1].size()!=(MTM_METATILES_W*MTM_METATILES_H) ||
			bindata[2].size()!=(MTM_METATILES_W*MTM_METATILES_H) ||
            bindata[3].size()!=(MTM_METATILES_W*MTM_METATILES_H) ||
            bindata[4].size()!=qFloor((MTM_METATILES_W*MTM_METATILES_H)/4) ||
			bindata[5].size()!=qFloor((MTM_METATILES_W*MTM_METATILES_H)/*/2*/)) {
		QMessageBox::critical(this,tr(MTM_COUNT_ERROR_TITLE),tr(MTM_COUNT_ERROR_BODY),QMessageBox::NoButton);
		return;
	}
	for(int count=0; count<(MTM_METATILES_W*MTM_METATILES_H); count++) {
		this->mtlMetatiles[count]->setTileIndex(0,bindata[0].at(count));
		this->mtlMetatiles[count]->setTileIndex(2,bindata[1].at(count));
		this->mtlMetatiles[count]->setTileIndex(1,bindata[2].at(count));
		this->mtlMetatiles[count]->setTileIndex(3,bindata[3].at(count));
		this->mtlMetatiles[count]->setPalette(((bindata[4].at(qFloor(count/4)))&(0x03<<((count%4)*2)))>>((count%4)*2));

		emit(sendMetatileToSelector(this->mtlMetatiles[count]));
		emit(this->sendMetatileProperties(count, bindata[5].at(count),false,false));
//		emit(this->sendMetatileProperties(count,
//                (bindata[5].at(qFloor(count/2))>>((count%2)*4))&0x03,
//                ((bindata[5].at(qFloor(count/2))>>((count%2)*4))&0x04)?true:false,
//                ((bindata[5].at(qFloor(count/2))>>((count%2)*4))&0x08)?true:false));
	}
	this->updateScreen();
}

void MetatileManager::clearAllMetatileData()
{
    this->pSelection = QPoint(0,0);

	quint8 tileindices[4] = {0,0,0,0};
	foreach(MetatileItem *t, this->mtlMetatiles) {
		t->setTileset(0);
		t->setTileIndices(tileindices);
        t->setPalette(0);
        t->setCollision(0);
        t->setDestructible(false);
        t->setDeadly(false);
	}

	this->drawGridLines();
    this->drawSelectionBox();
}



void MetatileManager::setSelectionMode(bool s)
{
	this->bSelectionMode=s;
	this->toggleShowGrid8(!s);
	this->toggleShowGrid16(!s);
	this->setSceneRect(0, 0, MTM_CANVAS_SIZE*this->iScale, MTM_CANVAS_SIZE*this->iScale);
}



void MetatileManager::updateScreen()
{
	this->viewport()->update();
}
