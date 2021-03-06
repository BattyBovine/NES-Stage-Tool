#include "tilesetmanager.h"

TilesetManager::TilesetManager(QWidget *parent) : QGraphicsView(parent)
{
	this->gsTileset = new QGraphicsScene();
	this->setScene(this->gsTileset);

	this->imgTileset = QImage(128, 128, QImage::Format_Indexed8);
	this->imgTileset.fill(0);
	this->imgTileset.setColor(0,qRgba(0x00,0x00,0x00,0x00));
	this->imgTileset.setColor(1,qRgb(0x00,0x00,0x00));
	this->imgTileset.setColor(2,qRgb(0x00,0x00,0x00));
	this->imgTileset.setColor(3,qRgb(0x00,0x00,0x00));

	this->gpiTileset = new QGraphicsPixmapItem();
	this->gpiTileset->setScale(TSM_SCALE);
	this->gsTileset->addItem(this->gpiTileset);
	this->griSelection[0] = this->griSelection[1] = NULL;
	this->iScale = TSM_SCALE;

	this->iGlobalTileset = 0;
	this->iSelectedTile = 0;
	this->iSelectedPalette = 0;
	this->iAnimFrame = 0;
	this->bTallSprite = false;

	this->pSelection = QPointF(0,0);
	this->drawSelectionBox();

//	this->threadCHR = new CHRThread();
//	connect(this->threadCHR,SIGNAL(sendCHRImageData(QImage)),this,SLOT(getNewCHRData(QImage)));
//	connect(this->threadCHR,SIGNAL(error(QString,QString)),this,SLOT(getCHRError(QString,QString)));

//	connect(&this->fswCHR,SIGNAL(fileChanged(QString)),this,SLOT(reloadCurrentTileset()));
}

TilesetManager::~TilesetManager()
{
	this->gsTileset->removeItem(this->gpiTileset);
//	this->threadCHR->deleteLater();
	delete this->gpiTileset;
	this->gsTileset->deleteLater();
}



void TilesetManager::resizeEvent(QResizeEvent*)
{
	QRectF viewrect = this->mapToScene(this->rect()).boundingRect();
	this->iScale = qFloor(qMin(viewrect.width(),viewrect.height())/this->imgTileset.width());
	this->gpiTileset->setScale(this->iScale);
	this->setSceneRect(0,0,this->imgTileset.width()*this->gpiTileset->scale(),this->imgTileset.height()*this->gpiTileset->scale());
	this->drawSelectionBox();
}

void TilesetManager::dropEvent(QDropEvent *e)
{
	e->acceptProposedAction();

	foreach(QUrl url, e->mimeData()->urls()) {
		if(url.toLocalFile().endsWith(".chr",Qt::CaseInsensitive)) {
			this->loadCHRData(url.toLocalFile());
			return;
		}
	}
	this->loadCHRData(e->mimeData()->urls()[0].toLocalFile());
}

void TilesetManager::mousePressEvent(QMouseEvent *e)
{
	this->pSelection = this->mapToScene(e->pos());
	this->pSelection.setX(qFloor(pSelection.x()/this->iScale));
	this->pSelection.setY(qFloor(pSelection.y()/this->iScale));
	this->drawSelectionBox();
}

void TilesetManager::wheelEvent(QWheelEvent *e)
{
	int steps = -qFloor(((qreal)e->angleDelta().y()/8)/15);

	this->iGlobalTileset = ((this->iGlobalTileset+steps)<0)?0:((this->iGlobalTileset+steps)>7)?7:(this->iGlobalTileset+steps);

	this->loadCHRBank();
//	emit(chrBankChanged(this->iGlobalTileset));
	emit(tilesetChangedDelta(steps));
}



bool TilesetManager::drawSelectionBox()
{
	qreal xorigin = qFloor(this->pSelection.x()/TSM_TILEWIDTH);
	qreal yorigin = qFloor(this->pSelection.y()/TSM_TILEWIDTH);
	if((xorigin < 0 || xorigin>=this->imgTileset.width()/TSM_TILEWIDTH) ||
		(yorigin < 0 || yorigin>=this->imgTileset.height()/TSM_TILEWIDTH))
		return false;

	this->iSelectedTile = (yorigin*(this->imgTileset.width()/TSM_TILEWIDTH))+xorigin;

	if(this->griSelection[0]/* && this->griSelection[1]*/) {
		if(this->griSelection[0]->parentItem()) this->gsTileset->removeItem(this->griSelection[0]);
//		if(this->griSelection[1]->parentItem()) this->gsTileset->removeItem(this->griSelection[1]);
		delete this->griSelection[0];
//		delete this->griSelection[1];
		this->griSelection[0] = NULL;
//		this->griSelection[1] = NULL;
	}

	//	QPen dashes(Qt::black,1,Qt::DashLine);
//	QVector<qreal> dp;
//	dp << 2 << 2;
//	dashes.setDashPattern(dp);
	this->griSelection[0] = this->gsTileset->addRect(QRectF(xorigin*TSM_TILEWIDTH*this->iScale,yorigin*TSM_TILEWIDTH*this->iScale,(TSM_TILEWIDTH*this->iScale)-1,(TSM_TILEWIDTH*this->iScale)-1),QPen(Qt::red),Qt::NoBrush);
//	this->griSelection[1] = this->gsTileset->addRect(QRectF(xorigin*TSM_TILEWIDTH*this->iScale,yorigin*TSM_TILEWIDTH*this->iScale,(TSM_TILEWIDTH*this->iScale)-1,(TSM_TILEWIDTH*this->iScale)-1),dashes,Qt::NoBrush);

	emit(selectedTileChanged(this->iSelectedTile));

	return true;
}

void TilesetManager::reloadCurrentTileset()
{
	threadCHR->loadFile(this->sCurrentTilesetFile);
}



void TilesetManager::loadCHRData(QString filename)
{
	this->threadCHR->loadFile(filename);
	if(!this->fswCHR.files().isEmpty()) this->fswCHR.removePath(this->sCurrentTilesetFile);
	this->sCurrentTilesetFile = filename;
	this->fswCHR.addPath(filename);
}

void TilesetManager::loadCHRBank()
{
	this->gpiTileset->setPixmap(TilesetCache::find(this->iGlobalTileset,this->iSelectedPalette,this->iAnimFrame));
}

void TilesetManager::setNewSpriteColours(PaletteVector/* c*/, quint8 i)
{
//	this->gsTileset->setBackgroundBrush(QBrush(QColor(c.at(PM_PALETTE_COLOURS_MAX*i))));
//	this->imgTileset.setColor(1,c.at((PM_PALETTE_COLOURS_MAX*i)+1));
//	this->imgTileset.setColor(2,c.at((PM_PALETTE_COLOURS_MAX*i)+2));
//	this->imgTileset.setColor(3,c.at((PM_PALETTE_COLOURS_MAX*i)+3));
	this->iSelectedPalette = i;

	this->loadCHRBank();
}



void TilesetManager::getNewCHRData()
{
	this->gpiTileset->setPixmap(TilesetCache::find(this->iGlobalTileset,this->iSelectedPalette,this->iAnimFrame));
}

void TilesetManager::getCHRError(QString title,QString body)
{
	QMessageBox::warning(NULL,title,body,QMessageBox::NoButton);
}

void TilesetManager::getGlobalTileset(int t)
{
	this->iGlobalTileset = t;
	this->loadCHRBank();
}

void TilesetManager::getNewAnimationFrame(int f)
{
	this->iAnimFrame = f;
	this->loadCHRBank();
}
