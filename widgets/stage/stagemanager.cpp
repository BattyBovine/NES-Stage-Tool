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
	this->bSelectionMode = false;
	this->iSelectedScreen = 0;
	this->iSelectedTileset = 0;
	this->griSelectionBox = NULL;
    this->setBackgroundBrush(QBrush(Qt::black));

	this->iCheckpointIndex = 0;
	this->vCheckpoints = CheckpointList(SM_MAX_CHECKPOINTS);
	for(int i=0; i<SM_MAX_CHECKPOINTS; i++) {
		this->vCheckpoints[i].Screen = 0;
		this->vCheckpoints[i].X = 0;
		this->vCheckpoints[i].Y = 0;
	}

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



void StageManager::resizeEvent(QResizeEvent *e)
{
	if(this->bSelectionMode) {
		this->fitInView(this->gsMetatiles->sceneRect(), Qt::KeepAspectRatio);
		this->iScale = qreal(this->viewport()->width())/qreal(MTI_TILEWIDTH*this->iScreenTilesW*this->iScreensW);
		this->drawSelectionBox();
		this->drawGridLines();
	} else {
		QGraphicsView::resizeEvent(e);
	}
}

void StageManager::dropEvent(QDropEvent *e)
{
	if(!this->bSelectionMode) {
		e->acceptProposedAction();
		emit(stageFileDropped(e->mimeData()->urls()[0].toLocalFile()));
	}
}

void StageManager::mousePressEvent(QMouseEvent *e)
{
	this->pMouseTranslation = QPointF(e->x(),e->y());

	switch(e->button()) {
	case Qt::LeftButton:
		if(this->bSelectionMode) {
			QPointF selection = this->mapToScene(e->pos());
			if(selection.x()<0 || selection.y()<0 ||
					selection.x()>=(MTI_TILEWIDTH*this->iScreenTilesW*this->iScreensW) ||
					selection.y()>=(MTI_TILEWIDTH*this->iScreenTilesH*this->iScreensH)) {
				return;
			}
			this->iSelectedScreen = (qFloor(selection.y()/(MTI_TILEWIDTH*this->iScreenTilesH))*this->iScreensW)+
									qFloor(selection.x()/(MTI_TILEWIDTH*this->iScreenTilesW));
			this->drawSelectionBox();
		} else {
			this->pRightMousePos = QPointF(-1,-1);
			this->replaceStageTile(this->mapToScene(e->pos()));
		}
		break;
	case Qt::RightButton:
		if(!this->bSelectionMode)
			this->replaceScreenTileset(this->mapToScene(e->pos()));
	default:
		QGraphicsView::mousePressEvent(e);
	}
}

void StageManager::mouseMoveEvent(QMouseEvent *e)
{
	QGraphicsView::mouseMoveEvent(e);

	if(!this->bSelectionMode) {
		if(e->buttons()&Qt::MiddleButton) {
			this->setTransformationAnchor(QGraphicsView::NoAnchor);
			this->translate((e->x()/this->iScale)-(this->pMouseTranslation.x()/this->iScale),
							(e->y()/this->iScale)-(this->pMouseTranslation.y()/this->iScale));
		} else if(e->buttons()&Qt::LeftButton) {
			this->replaceStageTile(this->mapToScene(e->pos()));
		}

		this->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
		this->pMouseTranslation = QPointF(e->x(),e->y());
	}

	this->getHoveredTile(this->mapToScene(e->pos()));
}

void StageManager::mouseDoubleClickEvent(QMouseEvent *e)
{
	if(!this->bSelectionMode) {
		switch(e->button()) {
		case Qt::MiddleButton:
			this->iScale = SM_DEFAULT_ZOOM;
			this->updateStageView();
			break;
//		case Qt::RightButton:
//			this->replaceAllScreenTiles(this->mapToScene(e->pos()));
//			break;
		default:
			QGraphicsView::mouseDoubleClickEvent(e);
		}
	}
}

void StageManager::wheelEvent(QWheelEvent *e)
{
	if(!this->bSelectionMode) {
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
}

void StageManager::keyPressEvent(QKeyEvent *e)
{
//	switch(e->key()) {
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
//	default:
		QGraphicsView::keyPressEvent(e);
//	}
}



void StageManager::drawGridLines()
{
	for(int i=0; i<this->lGrid.length(); i++) {
		if(this->lGrid[i]) {
			if(this->lGrid[i]->parentItem()) this->gsMetatiles->removeItem(this->lGrid[i]);
			delete this->lGrid[i];
			this->lGrid[i] = NULL;
		}
	}

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

void StageManager::drawSelectionBox()
{
	if(this->bSelectionMode) {
		if(this->griSelectionBox) {
			if(this->griSelectionBox->parentItem()) this->gsMetatiles->removeItem(this->griSelectionBox);
			delete this->griSelectionBox;
			this->griSelectionBox = NULL;
		}
		QPen thicksolid(Qt::red,(SM_THICK_SEL_LINES/this->iScale),Qt::SolidLine);
		this->griSelectionBox = this->gsMetatiles->addRect(
									(this->iSelectedScreen%this->iScreensW)*(MTI_TILEWIDTH*this->iScreenTilesW),
									qFloor(this->iSelectedScreen/this->iScreensW)*(MTI_TILEWIDTH*this->iScreenTilesH),
									(MTI_TILEWIDTH*this->iScreenTilesW),
									(MTI_TILEWIDTH*this->iScreenTilesH),
									thicksolid
									);
		emit(sendSelectionProperties(
				this->vScreenProperties[this->iSelectedScreen].Song,
				this->vScreenProperties[this->iSelectedScreen].ScrollBlockLeft,
				this->vScreenProperties[this->iSelectedScreen].ScrollBlockRight
				));
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
	this->vScreenProperties = ScreenPropList(this->iScreensW*this->iScreensH);
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
	for(int i=0; i<(this->iScreensW*this->iScreensH); i++) {
		this->vScreenProperties[i].Song = 0;
		this->vScreenProperties[i].ScrollBlockLeft = false;
		this->vScreenProperties[i].ScrollBlockRight = false;
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
	for(int i=0; i<this->vScreenProperties.size(); i++) {
		this->vScreenProperties[i].Song = 0;
		this->vScreenProperties[i].ScrollBlockLeft = false;
		this->vScreenProperties[i].ScrollBlockRight = false;
	}

	this->updateStageView();
}

void StageManager::getHoveredTile(QPointF p)
{
	if(p.x()<0 || p.y()<0 || p.x()>=((MTI_TILEWIDTH*this->iScreenTilesW)*this->iScreensW) || p.y()>=((MTI_TILEWIDTH*this->iScreenTilesH)*this->iScreensH))
		return;

	int tilex = qFloor(p.x()/MTI_TILEWIDTH);
	int tiley = qFloor(p.y()/MTI_TILEWIDTH);
	int screenx = qFloor(tilex/this->iScreenTilesW);
	int screeny = qFloor(tiley/this->iScreenTilesH);
	quint8 screen = (screeny*this->iScreensW)+screenx;
	quint8 tile = ((tiley%this->iScreenTilesH)*this->iScreenTilesW)+(tilex%this->iScreenTilesW);

	QString tiledata;
	tiledata += QString("S: $%1 | X: $%2 | Y: $%3 | T: $%4")
				.arg(screen,2,16,QChar('0'))
				.arg(tilex,2,16,QChar('0'))
				.arg(tiley,2,16,QChar('0'))
				.arg(this->vScreens[screen][tile]->metatileIndex(),2,16,QChar('0'))
				.toUpper();
	emit(sendTileData(tiledata));
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

void StageManager::setSelectionMode(bool s)
{
	this->bSelectionMode=s;
	this->toggleShowScreenGrid(!s);
	this->toggleShowTileGrid(!s);
	this->fitInView(this->gsMetatiles->sceneRect(), Qt::KeepAspectRatio);
	this->setRenderHint(QPainter::SmoothPixmapTransform);
}

void StageManager::setScreenProperties(int i, int song, bool sbl, bool sbr)
{
	this->vScreenProperties[i].Song = song;
	this->vScreenProperties[i].ScrollBlockLeft = sbl;
	this->vScreenProperties[i].ScrollBlockRight = sbr;
}



void StageManager::setCheckpointIndex(int i) {
	this->iCheckpointIndex=i;
	emit(sendCheckpointData(
			this->vCheckpoints[this->iCheckpointIndex].Screen,
			this->vCheckpoints[this->iCheckpointIndex].X,
			this->vCheckpoints[this->iCheckpointIndex].Y
			));
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
		quint8 *attributecolumn = new quint8[qFloor(this->iScreenTilesH/2)];
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
		delete [] attributecolumn;
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
	}

	tilesetbytes = tilesetbytes.left(tilesetbytes.length()-1) + QString("\n");

	return databytes+tilesetbytes;
}

QString StageManager::createScreenPropertiesASMData(QString labelprefix)
{
	QString asmlabel = labelprefix.isEmpty()?"emptylabel":labelprefix;
	QString propbytes = asmlabel+QString("_screenprops:\n\t.byte ");
	quint8 propbyte;

	for(int s=0; s<this->vScreenProperties.size(); s++) {
		propbyte = (this->vScreenProperties[s].Song&0x3F);
		propbyte |= this->vScreenProperties[s].ScrollBlockLeft?0x40:0x00;
		propbyte |= this->vScreenProperties[s].ScrollBlockRight?0x80:0x00;

		propbytes += QString("$%1").arg(propbyte,2,16,QChar('0')).toUpper().append(",");
	}

	propbytes = propbytes.left(propbytes.length()-1) + QString("\n");

	return propbytes;
}

QString StageManager::createCheckpointsASMData(QString labelprefix)
{
	QString asmlabel = labelprefix.isEmpty()?"emptylabel":labelprefix;
	QString ckptscreen = asmlabel+QString("_checkpoint_screen:\n\t.byte ");
	QString ckptx = asmlabel+QString("_checkpoint_posx:\n\t.byte ");
	QString ckpty = asmlabel+QString("_checkpoint_posy:\n\t.byte ");

	for(int c=0; c<this->vCheckpoints.size(); c++) {
		ckptscreen += QString("$%1").arg(this->vCheckpoints[c].Screen,2,16,QChar('0')).toUpper().append(",");
		ckptx += QString("$%1").arg(this->vCheckpoints[c].X,2,16,QChar('0')).toUpper().append(",");
		ckpty += QString("$%1").arg(this->vCheckpoints[c].Y,2,16,QChar('0')).toUpper().append(",");
	}

	ckptscreen = ckptscreen.left(ckptscreen.length()-1) + QString("\n");
	ckptx = ckptx.left(ckptx.length()-1) + QString("\n");
	ckpty = ckpty.left(ckpty.length()-1) + QString("\n");

	return ckptscreen+ckptx+ckpty;
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

void StageManager::openScreenPropertiesFile(QString filename)
{
	QFile file(filename);
	if(!file.open(QIODevice::ReadOnly|QIODevice::Text)) {
		QMessageBox::warning(this,tr(SM_FILE_OPEN_ERROR_TITLE),tr(SM_FILE_OPEN_ERROR_BODY),QMessageBox::NoButton);
		return;
	}
	QByteArray inputbytes;
	bool propsfound = false;

	while(!file.atEnd()) {
		QString line = file.readLine();

		QRegularExpression bytes(",?\\$([0-9a-fA-F]+)");
		QRegularExpressionMatchIterator bytesiter = bytes.globalMatch(line);
		QByteArray bytesin;
		while(bytesiter.hasNext()) {
			QRegularExpressionMatch bytesmatch = bytesiter.next();
			bytesin.append(quint8(bytesmatch.captured(1).toUInt(NULL,16)));
		}

		if(!bytesin.isEmpty() && propsfound && bytesin.size()==(this->iScreensW*this->iScreensH)) {
			inputbytes = bytesin;
			propsfound = false;
		}

		QRegularExpression propslabel("^(.*?)_screenprops:$");
		QRegularExpressionMatch propslabelmatch = propslabel.match(line);
		if(propslabelmatch.hasMatch()) {
			propsfound = true;
		}
	}
	file.close();
	if(!inputbytes.isEmpty()) {
		this->importScreenPropertiesBinaryData(inputbytes);
		return;
	}

	QMessageBox::critical(this,tr(SM_INVALID_STAGE_TITLE),tr(SM_INVALID_STAGE_BODY),QMessageBox::NoButton);
}

void StageManager::openCheckpointsFile(QString filename)
{
	QFile file(filename);
	if(!file.open(QIODevice::ReadOnly|QIODevice::Text)) {
		QMessageBox::warning(this,tr(SM_FILE_OPEN_ERROR_TITLE),tr(SM_FILE_OPEN_ERROR_BODY),QMessageBox::NoButton);
		return;
	}
	QVector<QByteArray> inputbytes;
	bool screensfound = false;
	bool cpxfound = false;
	bool cpyfound = false;

	while(!file.atEnd()) {
		QString line = file.readLine();

		QRegularExpression bytes(",?\\$([0-9a-fA-F]+)");
		QRegularExpressionMatchIterator bytesiter = bytes.globalMatch(line);
		QByteArray bytesin;
		while(bytesiter.hasNext()) {
			QRegularExpressionMatch bytesmatch = bytesiter.next();
			bytesin.append(quint8(bytesmatch.captured(1).toUInt(NULL,16)));
		}
		if(!bytesin.isEmpty() && (screensfound || cpxfound || cpyfound) && bytesin.size()==SM_MAX_CHECKPOINTS) {
			inputbytes.append(bytesin);
			screensfound = false;
			cpxfound = false;
			cpyfound = false;
		}

		QRegularExpression ckptscreenlabel("^(.*?)_checkpoint_screen:$");
		QRegularExpressionMatch ckptscreenlabelmatch = ckptscreenlabel.match(line);
		if(ckptscreenlabelmatch.hasMatch()) {
			screensfound = true;
		}

		QRegularExpression ckptxlabel("^(.*?)_checkpoint_posx:$");
		QRegularExpressionMatch ckptxlabelmatch = ckptxlabel.match(line);
		if(ckptxlabelmatch.hasMatch()) {
			cpxfound = true;
		}

		QRegularExpression ckptylabel("^(.*?)_checkpoint_posy:$");
		QRegularExpressionMatch ckptylabelmatch = ckptylabel.match(line);
		if(ckptylabelmatch.hasMatch()) {
			cpyfound = true;
		}
	}
	file.close();
	if(!inputbytes.isEmpty()) {
		this->importCheckpointsBinaryData(inputbytes);
		return;
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

void StageManager::importScreenPropertiesBinaryData(QByteArray bindata)
{
	if(bindata.size()!=(this->iScreensW*this->iScreensH)) {
		QMessageBox::critical(this,tr(SM_COUNT_ERROR_TITLE),tr(SM_COUNT_ERROR_BODY),QMessageBox::NoButton);
		return;
	}
	for(int s=0; s<(this->iScreensW*this->iScreensH); s++) {
		this->vScreenProperties[s].Song = (bindata.at(s)&0x3F);
		this->vScreenProperties[s].ScrollBlockLeft = (bindata.at(s)&0x40)?true:false;
		this->vScreenProperties[s].ScrollBlockRight = (bindata.at(s)&0x80)?true:false;
	}
	emit(sendSelectionProperties(
			this->vScreenProperties[this->iSelectedScreen].Song,
			this->vScreenProperties[this->iSelectedScreen].ScrollBlockLeft,
			this->vScreenProperties[this->iSelectedScreen].ScrollBlockRight
			));
}

void StageManager::importCheckpointsBinaryData(QVector<QByteArray> bindata)
{
	if(bindata.size()!=3) {
		QMessageBox::critical(this,tr(SM_COUNT_ERROR_TITLE),tr(SM_COUNT_ERROR_BODY),QMessageBox::NoButton);
		return;
	}
	for(int j=0; j<SM_MAX_CHECKPOINTS; j++) {
		this->vCheckpoints[j].Screen = quint8(bindata[0].at(j));
	}
	for(int j=0; j<SM_MAX_CHECKPOINTS; j++) {
		this->vCheckpoints[j].X = quint8(bindata[1].at(j));
	}
	for(int j=0; j<SM_MAX_CHECKPOINTS; j++) {
		this->vCheckpoints[j].Y = quint8(bindata[2].at(j));
	}
	emit(sendCheckpointData(
			this->vCheckpoints[this->iCheckpointIndex].Screen,
			this->vCheckpoints[this->iCheckpointIndex].X,
			this->vCheckpoints[this->iCheckpointIndex].Y
			));
}



void StageManager::updateStageView()
{
    this->setSceneRect(0, 0, (MTI_TILEWIDTH*this->iScreenTilesW)*this->iScreensW, (MTI_TILEWIDTH*this->iScreenTilesH)*this->iScreensH);
	this->drawGridLines();
	this->drawSelectionBox();
	this->viewport()->update();
}
