#include "stagemanager.h"

StageManager::StageManager(QWidget *parent) : QGraphicsView(parent)
{
	this->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
	this->gsMetatiles = new QGraphicsScene(this);
	this->gsMetatiles->setItemIndexMethod(QGraphicsScene::NoIndex);

	this->setScene(this->gsMetatiles);
	this->iScale = SM_DEFAULT_ZOOM;
    this->iScreensW = SM_SCREENS_W_DEFAULT;
    this->iScreensH = SM_SCREENS_H_DEFAULT;
    this->iScreenTilesW = SM_SCREEN_TILES_W_DEFAULT;
    this->iScreenTilesH = SM_SCREEN_TILES_H_DEFAULT;

	this->bShowScreenGrid = this->bShowTileGrid = true;
	this->iSelectedTileset = 0;
    this->setBackgroundBrush(QBrush(Qt::black));

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
}

void StageManager::mousePressEvent(QMouseEvent *e)
{
	this->pMouseTranslation = QPointF(e->x(),e->y());

	switch(e->button()) {
	case Qt::RightButton:
		this->pRightMousePos = QPointF(-1,-1);
		this->replaceStageTile(this->mapToScene(e->pos()));
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
		this->translate((e->x()/this->iScale)-(this->pMouseTranslation.x()/this->iScale),
						(e->y()/this->iScale)-(this->pMouseTranslation.y()/this->iScale));
	} else if(e->buttons()&Qt::RightButton) {
		this->replaceStageTile(this->mapToScene(e->pos()));
	}

	this->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
	this->pMouseTranslation = QPointF(e->x(),e->y());
}

void StageManager::mouseDoubleClickEvent(QMouseEvent *e)
{
	switch(e->button()) {
	case Qt::MiddleButton:
		this->iScale = SM_DEFAULT_ZOOM;
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
	qreal steps = (((qreal)e->angleDelta().y()/8)/15)/4;
	if(((this->iScale+steps)>=1) && ((this->iScale+steps)<=SM_MAX_ZOOM))
		this->iScale += steps;
	else
		this->iScale = ((steps<0)?1:SM_MAX_ZOOM);

	this->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
	QMatrix matrix;
	matrix.scale(this->iScale,this->iScale);
	this->setMatrix(matrix);

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
		QPen thicksolid(Qt::red,qFloor(SM_THICK_GRID_LINES/this->iScale),Qt::SolidLine);
//		QPen thickdashes(Qt::white,qFloor(SM_THICK_GRID_LINES/this->iScale),Qt::DashLine);
		QPen thinsolid(Qt::darkGray,qFloor(SM_THIN_GRID_LINES/this->iScale),Qt::SolidLine);
		QPen thindashes(Qt::lightGray,qFloor(SM_THIN_GRID_LINES/this->iScale),Qt::DashLine);
		QVector<qreal> dp, dplong;
		dp << 1 << 1;
		dplong << 4 << 4;
//		thickdashes.setDashPattern(dplong);
		thindashes.setDashPattern(dp);

		if(this->bShowTileGrid) {
            for(int i=0; i<=((MTI_TILEWIDTH*this->iScreenTilesH)*this->iScreensH); i+=MTI_TILEWIDTH) {
				// Horizontal lines
                this->lGrid.append(this->gsMetatiles->addLine(0,i,((MTI_TILEWIDTH*this->iScreenTilesW)*this->iScreensW),i,thinsolid));
                this->lGrid.append(this->gsMetatiles->addLine(0,i,((MTI_TILEWIDTH*this->iScreenTilesW)*this->iScreensW),i,thindashes));
			}
            for(int i=0; i<=((MTI_TILEWIDTH*this->iScreenTilesW)*this->iScreensW); i+=MTI_TILEWIDTH) {
				// Vertical lines
                this->lGrid.append(this->gsMetatiles->addLine(i,0,i,((MTI_TILEWIDTH*this->iScreenTilesH)*this->iScreensH),thinsolid));
                this->lGrid.append(this->gsMetatiles->addLine(i,0,i,((MTI_TILEWIDTH*this->iScreenTilesH)*this->iScreensH),thindashes));
			}
		}

		if(this->bShowScreenGrid) {
            for(int i=0; i<= ((MTI_TILEWIDTH*this->iScreenTilesH)*this->iScreensH); i+=(MTI_TILEWIDTH*this->iScreenTilesH)) {
				// Horizontal lines
                this->lGrid.append(this->gsMetatiles->addLine(0,i,((MTI_TILEWIDTH*this->iScreenTilesW)*this->iScreensW),i,thicksolid));
//				this->lGrid.append(this->gsMetatiles->addLine(0,i,((MTI_TILEWIDTH*this->iScreenTilesW)*this->iScreensW),i,thickdashes));
			}
            for(int i=0; i<= ((MTI_TILEWIDTH*this->iScreenTilesW)*this->iScreensW); i+=(MTI_TILEWIDTH*this->iScreenTilesW)) {
				// Vertical lines
                this->lGrid.append(this->gsMetatiles->addLine(i,0,i,((MTI_TILEWIDTH*this->iScreenTilesH)*this->iScreensH),thicksolid));
//				this->lGrid.append(this->gsMetatiles->addLine(i,0,i,((MTI_TILEWIDTH*this->iScreenTilesH)*this->iScreensH),thickdashes));
			}
		}
	}
}

void StageManager::populateBlankTiles()
{
    foreach(MetatileList l, this->vScreens) {
        foreach(MetatileItem *i, l) {
            this->groupMetatiles->removeFromGroup(i);
        }
    }

    this->vScreens = ScreenList(this->iScreensW*this->iScreensH);
    for(int sy=0; sy<this->iScreensH; sy++) {
        for(int sx=0; sx<this->iScreensW; sx++) {
			// Each screen gets its own tile list
            int screen = sy*this->iScreensW+sx;
			this->vScreens[screen].clear();
            for(int y=0; y<this->iScreenTilesH; y++) {
                for(int x=0; x<this->iScreenTilesW; x++) {
					MetatileItem *i = new MetatileItem();
                    i->setRealX((x*MTI_TILEWIDTH)+(sx*this->iScreenTilesW*MTI_TILEWIDTH));
                    i->setRealY((y*MTI_TILEWIDTH)+(sy*this->iScreenTilesH*MTI_TILEWIDTH));
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
    if(p.x()<0 || p.y()<0 || p.x()>=((MTI_TILEWIDTH*this->iScreenTilesW)*this->iScreensW) || p.y()>=((MTI_TILEWIDTH*this->iScreenTilesH)*this->iScreensH))
		return;

	int tilex = qFloor(p.x()/MTI_TILEWIDTH);
	int tiley = qFloor(p.y()/MTI_TILEWIDTH);
    int screenx = qFloor(tilex/this->iScreenTilesW);
    int screeny = qFloor(tiley/this->iScreenTilesH);
    quint8 screen = (screeny*this->iScreensW)+screenx;
    quint8 tile = ((tiley%this->iScreenTilesH)*this->iScreenTilesW)+(tilex%this->iScreenTilesW);

    if((screen>=(this->iScreensW*this->iScreensH) || tile>=(this->iScreenTilesW*this->iScreenTilesH)) ||
			(this->pRightMousePos.x()==screen && this->pRightMousePos.y()==tile))
		return;
	this->pRightMousePos = QPointF(screen,tile);

	emit(this->requestSelectedMetatile(this->vScreens[screen][tile]));
	this->updateStageView();
}

void StageManager::replaceScreenTileset(QPointF p)
{
    if(p.x()<0 || p.y()<0 || p.x()>=((MTI_TILEWIDTH*this->iScreenTilesW)*this->iScreensW) || p.y()>=((MTI_TILEWIDTH*this->iScreenTilesH)*this->iScreensH))
		return;

	int tilex = qFloor(p.x()/MTI_TILEWIDTH);
	int tiley = qFloor(p.y()/MTI_TILEWIDTH);
    int screenx = qFloor(tilex/this->iScreenTilesW);
    int screeny = qFloor(tiley/this->iScreenTilesH);
    quint8 screen = (screeny*this->iScreensW)+screenx;

	foreach(MetatileItem* t, this->vScreens[screen]) {
		t->setTileset(this->iSelectedTileset);
	}
	this->updateStageView();
}

void StageManager::replaceAllScreenTiles(QPointF p)
{
    if(p.x()<0 || p.y()<0 || p.x()>=((MTI_TILEWIDTH*this->iScreenTilesW)*this->iScreensW) || p.y()>=((MTI_TILEWIDTH*this->iScreenTilesH)*this->iScreensH))
		return;

	int tilex = qFloor(p.x()/MTI_TILEWIDTH);
	int tiley = qFloor(p.y()/MTI_TILEWIDTH);
    int screenx = qFloor(tilex/this->iScreenTilesW);
    int screeny = qFloor(tiley/this->iScreenTilesH);
    quint8 screen = (screeny*this->iScreensW)+screenx;
    quint8 tile = ((tiley%this->iScreenTilesH)*this->iScreenTilesW)+(tilex%this->iScreenTilesW);

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
    int numscreens = this->iScreensW*this->iScreensH;
	QVector<QByteArray> bindata = QVector<QByteArray>(numscreens);
	for(int s=0; s<numscreens; s++) {
		// Create metatile data (vertical columns)
		QByteArray bin;
        for(int tilewidth=0; tilewidth<this->iScreenTilesW; tilewidth++) {
            for(int tileheight=0; tileheight<this->iScreenTilesH; tileheight++) {
                bin.append(this->vScreens[s][(tileheight*this->iScreenTilesW)+tilewidth]->metatileIndex());
			}
		}
		bindata.replace(s,bin);

		// Create attribute data (vertical columns)
        quint8 attributecolumn[qFloor(this->iScreenTilesH/2)];
		bin.clear();
        for(int i=0; i<qFloor(this->iScreenTilesH/2); i++) attributecolumn[i] = 0;
        for(int tilewidth=0; tilewidth<this->iScreenTilesW; tilewidth+=2) {
            for(int tileheight=0; tileheight<this->iScreenTilesH; tileheight+=2) {
                quint8 attr0 = (this->vScreens[s][(tileheight*this->iScreenTilesW)+tilewidth]->palette()%PM_SUBPALETTES_MAX);
                quint8 attr1 = (this->vScreens[s][(tileheight*this->iScreenTilesW)+tilewidth+1]->palette()%PM_SUBPALETTES_MAX);
                quint8 attr2 = (this->vScreens[s][((tileheight+1)*this->iScreenTilesW)+tilewidth]->palette()%PM_SUBPALETTES_MAX);
                quint8 attr3 = (this->vScreens[s][((tileheight+1)*this->iScreenTilesW)+tilewidth+1]->palette()%PM_SUBPALETTES_MAX);
                attributecolumn[qFloor(tileheight/qFloor(this->iScreenTilesH/2))%8] = ((attr0)|(attr1<<2)|(attr2<<4)|(attr3<<6));
			}
            for(int i=0; i<qFloor(this->iScreenTilesH/2); i++) {
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

    int numscreens = this->iScreensW*this->iScreensH;
	for(int s=0; s<numscreens; s++) {
		QString countedlabel = asmlabel+QString("_%1").arg(s,2,16,QChar('0')).toUpper();

		databytes += countedlabel+":\n\t.byte ";

		QString screenbytes;
        for(int tilewidth=0; tilewidth<this->iScreenTilesW; tilewidth++) {
            for(int tileheight=0; tileheight<this->iScreenTilesH; tileheight++) {
                screenbytes += QString("$%1").arg(this->vScreens[s][(tileheight*this->iScreenTilesW)+tilewidth]->metatileIndex(),2,16,QChar('0')).toUpper().append(",");
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
    QVector<QByteArray> inputbytes(this->iScreensW*this->iScreensH+1);
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

        if(!bytesin.isEmpty() && bytesin.size()==(this->iScreenTilesW*this->iScreenTilesH)) {
			inputbytes.replace(labelnum,bytesin);
        } else if(!bytesin.isEmpty() && tilesetsfound && bytesin.size()==qFloor((this->iScreensW*this->iScreensH)/2)) {
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
    for(int s=0; s<(this->iScreensW*this->iScreensH); s++) {
        if(bindata[s].size()!=(this->iScreenTilesW*this->iScreenTilesH)) {
			QMessageBox::critical(this,tr(SM_COUNT_ERROR_TITLE),tr(SM_COUNT_ERROR_BODY),QMessageBox::NoButton);
			return;
		}
        for(int x=0; x<this->iScreenTilesW; x++) {
            for(int y=0; y<this->iScreenTilesH; y++) {
                this->vScreens[s][(y*this->iScreenTilesW)+x]->setMetatileIndex(bindata[s].at((x*this->iScreenTilesH)+y));
                emit(requestTileUpdate(this->vScreens[s][(y*this->iScreenTilesW)+x]));
			}
		}
	}

    for(int s=0; s<(this->iScreensW*this->iScreensH); s++) {
        if(bindata.last().size()!=qFloor((this->iScreensW*this->iScreensH)/2)) {
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
    this->setSceneRect(0, 0, (MTI_TILEWIDTH*this->iScreenTilesW)*this->iScreensW, (MTI_TILEWIDTH*this->iScreenTilesH)*this->iScreensH);
	this->drawGridLines();
	this->viewport()->update();
}
