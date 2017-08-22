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
	this->bShowObjects = true;
	this->bTileSelectMode = true;
	this->bScreenSelectMode = false;
	this->iSelectedScreen = 0;
	this->iSelectedTileset = 0;
	this->griSelectionBox = NULL;
	this->griTileSelector = NULL;
	this->setBackgroundBrush(QBrush(Qt::black));

	this->groupMetatiles = new QGraphicsItemGroup();
	this->gsMetatiles->addItem(this->groupMetatiles);

	for(int i=0; i<SM_CHECKPOINT_LIMIT; i++) {
		this->lCheckpoints.append(new CheckpointItem(i));
		this->lCheckpoints[i]->setFlags(QGraphicsItem::ItemIsMovable|QGraphicsItem::ItemIsSelectable);
		this->lCheckpoints[i]->setEnabled(false);
		this->gsMetatiles->addItem(this->lCheckpoints[i]);
	}

	this->rTileSelection = QRect(-1,-1,-1,-1);
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
	if(this->bScreenSelectMode) {
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
	if(!this->bScreenSelectMode) {
		if(e->mimeData()->hasUrls()) {
			e->acceptProposedAction();
			emit(stageFileDropped(e->mimeData()->urls()[0].toLocalFile()));
		} else if(e->mimeData()->formats().contains("application/objectdata")) {
			if(this->lObjects.count()>=SM_OBJECT_LIMIT) {
				QMessageBox::warning(this,
									 tr(SM_OBJ_LIMIT_ERROR_TITLE),
									 tr(SM_OBJ_LIMIT_ERROR_BODY).arg(SM_OBJECT_LIMIT),
									 QMessageBox::Ok);
				return;
			}
			QDataStream stream(&e->mimeData()->data(OM_MIME_TYPE),QIODevice::ReadOnly);
			int row;
			stream >> row;
			ObjectItem *item = new ObjectItem(row);
			if(!item->isNull()) {
				e->acceptProposedAction();
				QPointF drop = this->mapToScene(e->pos());
				item->setX(qFloor(drop.x()));
				item->setY(qFloor(drop.y()));
				item->setVisible(!this->bTileSelectMode);
				this->lObjects.append(item);
				this->gsMetatiles->addItem(item);
			}
		} else if(e->mimeData()->formats().contains("application/checkpointdata")) {
			QDataStream stream(&e->mimeData()->data(CM_MIME_TYPE),QIODevice::ReadOnly);
			int id;
			stream >> id;
			e->acceptProposedAction();
			QPointF drop = this->mapToScene(e->pos());
			this->lCheckpoints[id]->setX(qFloor(drop.x()));
			this->lCheckpoints[id]->setY(qFloor(drop.y()));
			this->lCheckpoints[id]->setVisible(!this->bTileSelectMode);
			this->lCheckpoints[id]->setEnabled(!this->bTileSelectMode);
		}
	}
}

void StageManager::mousePressEvent(QMouseEvent *e)
{
	this->pSceneTranslation = QPointF(e->x(),e->y());
	QPointF selection = this->mapToScene(e->pos());

	switch(e->button()) {
	case Qt::LeftButton:
		if(this->bScreenSelectMode) {
			if(selection.x()<0 || selection.y()<0 ||
					selection.x()>=(MTI_TILEWIDTH*this->iScreenTilesW*this->iScreensW) ||
					selection.y()>=(MTI_TILEWIDTH*this->iScreenTilesH*this->iScreensH)) {
				return;
			}
			this->iSelectedScreen = (qFloor(selection.y()/(MTI_TILEWIDTH*this->iScreenTilesH))*this->iScreensW)+
									qFloor(selection.x()/(MTI_TILEWIDTH*this->iScreenTilesW));
			this->drawSelectionBox();
		} else if(this->bTileSelectMode) {
			this->rTileSelection = QRectF(qFloor(selection.x()/MTI_TILEWIDTH),
										  qFloor(selection.y()/MTI_TILEWIDTH),
										  0, 0);
			this->drawTileSelectionBox();
		}
		break;
	case Qt::RightButton:
		if(!this->bScreenSelectMode)
			this->replaceScreenTileset(selection);
		break;
	}

	QGraphicsView::mousePressEvent(e);
}

void StageManager::mouseMoveEvent(QMouseEvent *e)
{
	QGraphicsView::mouseMoveEvent(e);

	if(!this->bScreenSelectMode) {
		if(e->buttons()&Qt::LeftButton) {
			QPointF mousepos = this->mapToScene(e->pos());
			if(this->bTileSelectMode) {
				this->rTileSelection.setWidth(qFloor(qFloor(mousepos.x()/MTI_TILEWIDTH)-this->rTileSelection.x()));
				this->rTileSelection.setHeight(qFloor(qFloor(mousepos.y()/MTI_TILEWIDTH)-this->rTileSelection.y()));
				this->drawTileSelectionBox();
			} else {
				foreach(ObjectItem *i, this->lObjects) {
					if(i->isSelected()) {
						i->setX(mousepos.x());
						i->setY(mousepos.y());
					}
				}
				foreach(CheckpointItem *i, this->lCheckpoints) {
					if(i->isSelected()) {
						i->setX(mousepos.x());
						i->setY(mousepos.y());
					}
				}
			}
		} else if(e->buttons()&Qt::MiddleButton) {
			this->setTransformationAnchor(QGraphicsView::NoAnchor);
			this->translate((e->x()/this->iScale)-(this->pSceneTranslation.x()/this->iScale),
							(e->y()/this->iScale)-(this->pSceneTranslation.y()/this->iScale));
		}

		this->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
		this->pSceneTranslation = QPointF(e->x(),e->y());
	}

	this->getHoveredTile(this->mapToScene(e->pos()));
}

void StageManager::mouseReleaseEvent(QMouseEvent *e)
{
	switch(e->button()) {
	case Qt::LeftButton:
		if(!this->bScreenSelectMode && this->bTileSelectMode) this->replaceStageTiles();
		break;
	case Qt::RightButton:
		if(!this->bScreenSelectMode && this->bTileSelectMode)	this->replaceScreenTileset(this->mapToScene(e->pos()));
	default:
		QGraphicsView::mousePressEvent(e);
	}
}

void StageManager::mouseDoubleClickEvent(QMouseEvent *e)
{
	if(!this->bScreenSelectMode) {
		switch(e->button()) {
		case Qt::MiddleButton:
			this->iScale = SM_DEFAULT_ZOOM;
			this->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
			QMatrix matrix;
			matrix.scale(this->iScale,this->iScale);
			this->setMatrix(matrix);
			this->updateStageView();
			return;
		}
	}
	QGraphicsView::mouseDoubleClickEvent(e);
}

void StageManager::wheelEvent(QWheelEvent *e)
{
	if(!this->bScreenSelectMode) {
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
	switch(e->key()) {
	case Qt::Key_Left:
		foreach(ObjectItem *i, this->lObjects)
			if(i->isSelected())
				i->moveBy(-1*(e->modifiers()&Qt::ShiftModifier?10:1),0);
		foreach(CheckpointItem *i, this->lCheckpoints)
			if(i->isSelected())
				i->moveBy(-1*(e->modifiers()&Qt::ShiftModifier?10:1),0);
		break;
	case Qt::Key_Right:
		foreach(ObjectItem *i, this->lObjects)
			if(i->isSelected())
				i->moveBy(1*(e->modifiers()&Qt::ShiftModifier?10:1),0);
		foreach(CheckpointItem *i, this->lCheckpoints)
			if(i->isSelected())
				i->moveBy(1*(e->modifiers()&Qt::ShiftModifier?10:1),0);
		break;
	case Qt::Key_Up:
		foreach(ObjectItem *i, this->lObjects)
			if(i->isSelected())
				i->moveBy(0,-1*(e->modifiers()&Qt::ShiftModifier?10:1));
		foreach(CheckpointItem *i, this->lCheckpoints)
			if(i->isSelected())
				i->moveBy(0,-1*(e->modifiers()&Qt::ShiftModifier?10:1));
		break;
	case Qt::Key_Down:
		foreach(ObjectItem *i, this->lObjects)
			if(i->isSelected())
				i->moveBy(0,1*(e->modifiers()&Qt::ShiftModifier?10:1));
		foreach(CheckpointItem *i, this->lCheckpoints)
			if(i->isSelected())
				i->moveBy(0,1*(e->modifiers()&Qt::ShiftModifier?10:1));
		break;
	case Qt::Key_Delete:
		if(!this->bTileSelectMode) {
			foreach(ObjectItem *i, this->lObjects) {
				if(i->isSelected()) {
					this->lObjects.removeAll(i);
					delete i;
				}
			}
			foreach(CheckpointItem *i, this->lCheckpoints) {
				if(i->isSelected())
					i->setEnabled(false);
			}
		}
	default:
		QGraphicsView::keyPressEvent(e);
	}
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
	if(this->bScreenSelectMode) {
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

void StageManager::drawTileSelectionBox()
{
	if(!this->bScreenSelectMode) {
		if(this->griTileSelector) {
			if(this->griTileSelector->parentItem()) this->gsMetatiles->removeItem(this->griTileSelector);
			delete this->griTileSelector;
			this->griTileSelector = NULL;
		}

		if(this->rTileSelection.x()<0)	this->rTileSelection.setX(0);
		if(this->rTileSelection.y()<0)	this->rTileSelection.setY(0);

		QPen outline(Qt::blue,(SM_THICK_SEL_LINES/this->iScale),Qt::SolidLine);
		QColor fillcolour(Qt::blue);
		fillcolour.setAlpha(128);
		QBrush fillrect(fillcolour);
		this->griTileSelector = this->gsMetatiles->addRect(
									(this->rTileSelection.x()*MTI_TILEWIDTH)+((this->rTileSelection.width()<0?MTI_TILEWIDTH:0)),
									(this->rTileSelection.y()*MTI_TILEWIDTH)+((this->rTileSelection.height()<0?MTI_TILEWIDTH:0)),
									(this->rTileSelection.width()*MTI_TILEWIDTH)+((this->rTileSelection.width()<0)?(-MTI_TILEWIDTH):MTI_TILEWIDTH),
									(this->rTileSelection.height()*MTI_TILEWIDTH)+((this->rTileSelection.height()<0)?(-MTI_TILEWIDTH):MTI_TILEWIDTH),
									outline,fillrect);
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
					i->setScreenIndex(this->vScreens[screen].size());
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

void StageManager::replaceStageTiles()
{
	if(this->griTileSelector) {
		if(this->griTileSelector->parentItem()) this->gsMetatiles->removeItem(this->griTileSelector);
		delete this->griTileSelector;
		this->griTileSelector = NULL;
	}

	if(!this->bScreenSelectMode) {
		int xbegin=0,xend=0,ybegin=0,yend=0;
		if(this->rTileSelection.width()<0) {
			if(this->rTileSelection.height()<0) {	// Selection box is fully inverted
				xbegin = (this->rTileSelection.x()+this->rTileSelection.width());
				ybegin = (this->rTileSelection.y()+this->rTileSelection.height());
				xend = this->rTileSelection.x();
				yend = this->rTileSelection.y();
			} else {								// Selection box width is inverted
				xbegin = (this->rTileSelection.x()+this->rTileSelection.width());
				ybegin = this->rTileSelection.y();
				xend = this->rTileSelection.x();
				yend = (this->rTileSelection.y()+this->rTileSelection.height());
			}
		} else {
			if(this->rTileSelection.height()<0) {
				xbegin = this->rTileSelection.x();	// Selection box height is inverted
				ybegin = (this->rTileSelection.y()+this->rTileSelection.height());
				xend = (this->rTileSelection.x()+this->rTileSelection.width());
				yend = this->rTileSelection.y();
			} else {								// Selection box is normal
				xbegin = this->rTileSelection.x();
				ybegin = this->rTileSelection.y();
				xend = (this->rTileSelection.x()+this->rTileSelection.width());
				yend = (this->rTileSelection.y()+this->rTileSelection.height());
			}
		}

		for(int y=ybegin; y<=yend; y++) {
			for(int x=xbegin; x<=xend; x++) {
				if(x<0 || y<0 || x>=((MTI_TILEWIDTH*this->iScreenTilesW)*this->iScreensW) || y>=((MTI_TILEWIDTH*this->iScreenTilesH)*this->iScreensH))	continue;
				int screenx = qFloor(x/this->iScreenTilesW);
				int screeny = qFloor(y/this->iScreenTilesH);
				quint8 screen = (screeny*this->iScreensW)+screenx;
				quint8 tile = ((y%this->iScreenTilesH)*this->iScreenTilesW)+(x%this->iScreenTilesW);
				if((screen>=(this->iScreensW*this->iScreensH) || tile>=(this->iScreenTilesW*this->iScreenTilesH)))	continue;

				emit(this->requestSelectedMetatile(this->vScreens[screen][tile]));
			}
		}
		this->updateStageView();
	}
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
	this->setScreenTileset(screen,this->iSelectedTileset);
	emit(updateTileset(screen,this->iSelectedTileset));
	this->updateStageView();
}

void StageManager::setScreenTileset(quint8 screen, quint8 tileset)
{
	foreach(MetatileItem* t, this->vScreens[screen]) {
		t->setTileset(tileset);
	}
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

void StageManager::clearAllObjectData()
{
	foreach(ObjectItem *i, this->lObjects)
		delete i;
	this->lObjects.clear();
}

void StageManager::clearAllCheckpointData()
{
	foreach(CheckpointItem *i, this->lCheckpoints)
		i->setEnabled(false);
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

void StageManager::getReplacementTile(MetatileItem *mt)
{
	emit(requestSelectedMetatile(this->vScreens[mt->screen()][mt->screenIndex()]));
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
	this->bScreenSelectMode=s;
	this->toggleShowScreenGrid(!s);
	this->toggleShowTileGrid(!s);
	if(s) {
		QRectF view(0,0,this->iScreensW*this->iScreenTilesW*MTI_TILEWIDTH,this->iScreensH*this->iScreenTilesH*MTI_TILEWIDTH);
		this->fitInView(view, Qt::KeepAspectRatio);
		this->setRenderHint(QPainter::SmoothPixmapTransform);
	}
}

void StageManager::setScreenProperties(int i, int song, bool sbl, bool sbr)
{
	this->vScreenProperties[i].Song = song;
	this->vScreenProperties[i].ScrollBlockLeft = sbl;
	this->vScreenProperties[i].ScrollBlockRight = sbr;
}



void StageManager::setToolMode(int t) {
	this->bTileSelectMode = t?false:true;
	foreach(ObjectItem *i, this->lObjects)
		i->setVisible(!this->bTileSelectMode);
	foreach(CheckpointItem *i, this->lCheckpoints)
		i->setVisible(!this->bTileSelectMode);
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

	for(int c=0; c<SM_CHECKPOINT_LIMIT; c++) {
		if(this->lCheckpoints[c]->isEnabled()) {
			ckptscreen += QString("$%1").arg(this->lCheckpoints[c]->screen(),2,16,QChar('0')).toUpper().append(",");
			ckptx += QString("$%1").arg(this->lCheckpoints[c]->screenX(),2,16,QChar('0')).toUpper().append(",");
			ckpty += QString("$%1").arg(this->lCheckpoints[c]->screenY(),2,16,QChar('0')).toUpper().append(",");
		} else {
			ckptscreen += QString("$00,");
			ckptx += QString("$00,");
			ckpty += QString("$00,");
		}
	}

	ckptscreen = ckptscreen.left(ckptscreen.length()-1) + QString("\n");
	ckptx = ckptx.left(ckptx.length()-1) + QString("\n");
	ckpty = ckpty.left(ckpty.length()-1) + QString("\n");

	return ckptscreen+ckptx+ckpty;
}

QString StageManager::createObjectsASMData(QString labelprefix)
{
	QString asmlabel = labelprefix.isEmpty()?"emptylabel":labelprefix;
	QString objid = asmlabel+QString("_object_id:\n\t.byte ");
	QString objscreen = asmlabel+QString("_object_screen:\n\t.byte ");
	QString objx = asmlabel+QString("_object_posx:\n\t.byte ");
	QString objy = asmlabel+QString("_object_posy:\n\t.byte ");

	for(int c=0; c<SM_OBJECT_LIMIT; c++) {
		if(c<this->lObjects.count()) {
			objid += QString("$%1").arg(this->lObjects[c]->id(),2,16,QChar('0')).toUpper().append(",");
			objscreen += QString("$%1").arg(this->lObjects[c]->screen(),2,16,QChar('0')).toUpper().append(",");
			objx += QString("$%1").arg(this->lObjects[c]->screenX(),2,16,QChar('0')).toUpper().append(",");
			objy += QString("$%1").arg(this->lObjects[c]->screenY(),2,16,QChar('0')).toUpper().append(",");
		} else {
			objid += QString("$00,");
			objscreen += QString("$00,");
			objx += QString("$00,");
			objy += QString("$00,");
		}
	}

	objid = objid.left(objid.length()-1) + QString("\n");
	objscreen = objscreen.left(objscreen.length()-1) + QString("\n");
	objx = objx.left(objx.length()-1) + QString("\n");
	objy = objy.left(objy.length()-1) + QString("\n");

	return objid+objscreen+objx+objy;
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

	foreach(CheckpointItem *i, this->lCheckpoints)
		i->setEnabled(false);

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
		if(!bytesin.isEmpty() && (screensfound || cpxfound || cpyfound) && bytesin.size()==SM_CHECKPOINT_LIMIT) {
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

void StageManager::openObjectsFile(QString filename)
{
	QFile file(filename);
	if(!file.open(QIODevice::ReadOnly|QIODevice::Text)) {
		QMessageBox::warning(this,tr(SM_FILE_OPEN_ERROR_TITLE),tr(SM_FILE_OPEN_ERROR_BODY),QMessageBox::NoButton);
		return;
	}

	foreach(ObjectItem *i, this->lObjects)
		delete i;
	this->lObjects.clear();

	QVector<QByteArray> inputbytes;
	bool idsfound = false;
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
		if(!bytesin.isEmpty() && (idsfound || screensfound || cpxfound || cpyfound) && bytesin.size()==SM_OBJECT_LIMIT) {
			inputbytes.append(bytesin);
			idsfound = false;
			screensfound = false;
			cpxfound = false;
			cpyfound = false;
		}

		QRegularExpression objectidlabel("^(.*?)_object_id:$");
		QRegularExpressionMatch objectidlabelmatch = objectidlabel.match(line);
		if(objectidlabelmatch.hasMatch()) {
			idsfound = true;
		}

		QRegularExpression objectscreenlabel("^(.*?)_object_screen:$");
		QRegularExpressionMatch objectscreenlabelmatch = objectscreenlabel.match(line);
		if(objectscreenlabelmatch.hasMatch()) {
			screensfound = true;
		}

		QRegularExpression objectxlabel("^(.*?)_object_posx:$");
		QRegularExpressionMatch objectxlabelmatch = objectxlabel.match(line);
		if(objectxlabelmatch.hasMatch()) {
			cpxfound = true;
		}

		QRegularExpression objectylabel("^(.*?)_object_posy:$");
		QRegularExpressionMatch objectylabelmatch = objectylabel.match(line);
		if(objectylabelmatch.hasMatch()) {
			cpyfound = true;
		}
	}
	file.close();
	if(!inputbytes.isEmpty()) {
		this->importObjectsBinaryData(inputbytes);
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

	quint8 screen, x, y;
	for(int j=0; j<SM_CHECKPOINT_LIMIT; j++) {
		screen = quint8(bindata[0].at(j));
		x = quint8(bindata[1].at(j));
		y = quint8(bindata[2].at(j));
		if(screen==0 && x==0 && y==0) {
			this->lCheckpoints[j]->setEnabled(false);
		} else {
			this->lCheckpoints[j]->setX(((screen%8)*256)+x+0.5f);
			this->lCheckpoints[j]->setY((qFloor(screen/8)*192)+y+0.5f);
			this->lCheckpoints[j]->setVisible(!this->bTileSelectMode);
			this->lCheckpoints[j]->setEnabled(true);
		}
	}
}

void StageManager::importObjectsBinaryData(QVector<QByteArray> bindata)
{
	if(bindata.size()!=4) {
		QMessageBox::critical(this,tr(SM_COUNT_ERROR_TITLE),tr(SM_COUNT_ERROR_BODY),QMessageBox::NoButton);
		return;
	}

	quint8 id, screen, x, y;
	for(int j=0; j<SM_OBJECT_LIMIT; j++) {
		id = quint8(bindata[0].at(j));
		screen = quint8(bindata[1].at(j));
		x = quint8(bindata[2].at(j));
		y = quint8(bindata[3].at(j));
		if(id!=0 || screen!=0 || x!=0 || y!=0) {
			ObjectItem *i = new ObjectItem(id);
			i->setX(((screen%8)*256)+x+0.5f);
			i->setY((qFloor(screen/8)*192)+y+0.5f);
			i->setVisible(!this->bTileSelectMode);
			this->gsMetatiles->addItem(i);
			this->lObjects.append(i);
		}
	}
}



void StageManager::updateStageView()
{
	this->setSceneRect(0, 0, (MTI_TILEWIDTH*this->iScreenTilesW)*this->iScreensW, (MTI_TILEWIDTH*this->iScreenTilesH)*this->iScreensH);
	this->drawGridLines();
	this->drawSelectionBox();
	this->viewport()->update();
}
