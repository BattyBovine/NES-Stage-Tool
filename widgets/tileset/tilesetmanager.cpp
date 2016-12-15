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
	this->iSelectedTile = 0;
	this->iPalette = 0;
	this->bTallSprite = false;
	this->iBankDivider = 0x100;
	this->iSelectedBank = 0;

	this->pSelection = QPointF(0,0);
	this->drawSelectionBox();

	this->threadCHR = new CHRThread();
	connect(this->threadCHR,SIGNAL(sendCHRImageData(QImage)),this,SLOT(getNewCHRData(QImage)));
	connect(this->threadCHR,SIGNAL(error(QString,QString)),this,SLOT(getCHRError(QString,QString)));

	connect(&this->fswCHR,SIGNAL(fileChanged(QString)),this,SLOT(reloadCurrentTileset()));
}

TilesetManager::~TilesetManager()
{
	this->gsTileset->removeItem(this->gpiTileset);
	this->threadCHR->deleteLater();
	delete this->gpiTileset;
	this->gsTileset->deleteLater();
}



void TilesetManager::resizeEvent(QResizeEvent*)
{
	QRectF viewrect = this->mapToScene(this->rect()).boundingRect();
	this->iScale = qFloor(qMin(viewrect.width(),viewrect.height())/this->imgTileset.width());
	this->gpiTileset->setScale(this->iScale);
	this->setSceneRect(0,0,
					   this->imgTileset.width()*this->gpiTileset->scale(),
					   this->imgTileset.height()*this->gpiTileset->scale());
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
	qreal steps = -(((qreal)e->angleDelta().y()/8)/15);

	if(((this->iSelectedBank+steps)>=1) && ((this->iSelectedBank+steps)<=((this->imgTileset.height()/(this->iBankDivider>>1))-1))) {
		this->iSelectedBank += steps;
	} else {
		this->iSelectedBank = ((steps<0)?0:((this->imgTileset.height()/(this->iBankDivider>>1))-1));
	}

	this->loadCHRBank();
	emit(chrBankChanged(this->iSelectedBank));
}

void TilesetManager::setSelectedBank(quint16 bankno)
{
	quint16 bankmax = ((this->imgTileset.height()/(this->iBankDivider>>1))-1);
	quint16 newbank = ((bankno>=bankmax)?bankmax:bankno);
	emit(this->checkTilesBank(newbank,bankmax));

	if(this->iSelectedBank != bankno) {
		this->iSelectedBank = newbank;

		this->loadCHRBank();
		emit(chrBankChanged(this->iSelectedBank));
	}
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
	this->imgSelectedBank = this->imgTileset.copy(QRect(0,((this->iBankDivider>>1)*this->iSelectedBank),128,(this->iBankDivider>>1)));
//	this->setFixedSize(256,this->iBankDivider);
//	this->setSceneRect(0,0,256,this->iBankDivider);

	this->gpiTileset->setPixmap(QPixmap::fromImage(this->imgSelectedBank));
	emit(tilesetChanged(this->bTallSprite));
}

void TilesetManager::setNewSpriteColours(PaletteVector c, quint8 i)
{
	this->gsTileset->setBackgroundBrush(QBrush(QColor(c.at(PM_PALETTE_COLOURS_MAX*i))));
	this->imgTileset.setColor(1,c.at((PM_PALETTE_COLOURS_MAX*i)+1));
	this->imgTileset.setColor(2,c.at((PM_PALETTE_COLOURS_MAX*i)+2));
	this->imgTileset.setColor(3,c.at((PM_PALETTE_COLOURS_MAX*i)+3));
	this->iPalette = i;

	this->loadCHRBank();
}



void TilesetManager::getNewTile(QPointF p)
{
	quint32 selection = this->iSelectedTile+(this->iBankDivider*this->iSelectedBank);
	emit(sendNewTile(p,this->createNewTile(selection),selection,this->iPalette));
}

void TilesetManager::getNewSubtile(quint8 i, MetatileItem *t)
{
	quint8 selection = this->iSelectedTile+(this->iBankDivider*this->iSelectedBank);
	t->setTileIndex(i,selection);
	t->setPalette(this->iPalette);
	this->updateMetatile(t);
	emit(sendNewMetatile(t));
}

void TilesetManager::updateMetatile(MetatileItem *t)
{
	t->setTile(0,this->createNewTile((t->tileIndex(0)+(this->iSelectedBank*this->iBankDivider))));
	t->setTile(1,this->createNewTile((t->tileIndex(1)+(this->iSelectedBank*this->iBankDivider))));
	t->setTile(2,this->createNewTile((t->tileIndex(2)+(this->iSelectedBank*this->iBankDivider))));
	t->setTile(3,this->createNewTile((t->tileIndex(3)+(this->iSelectedBank*this->iBankDivider))));
	emit(this->metatileUpdated(t));
}

void TilesetManager::getNewCHRData(QImage img)
{
	img.setColor(1,this->imgTileset.color(1));
	img.setColor(2,this->imgTileset.color(2));
	img.setColor(3,this->imgTileset.color(3));
	this->imgTileset = img;

	this->loadCHRBank();
	emit(this->chrDataChanged(this->imgTileset));
}

void TilesetManager::getCHRError(QString title,QString body)
{
	QMessageBox::warning(NULL,title,body,QMessageBox::NoButton);
}

void TilesetManager::getBankDivider(quint16 bankdiv)
{
	qreal diff = qreal(this->iBankDivider)/qreal(bankdiv);
	this->iBankDivider = bankdiv;
	this->iSelectedBank *= diff;

	this->loadCHRBank();
	emit(this->chrDataChanged(this->imgTileset));
	this->drawSelectionBox();
}



QImage TilesetManager::createNewTile(quint32 tile)
{
	QImage newtile(TSM_TILEWIDTH, TSM_TILEWIDTH*(this->bTallSprite?2:1), QImage::Format_Indexed8);

	int x = (tile&0x0F)*TSM_TILEWIDTH;
	int y = (((tile%this->iBankDivider)&0xF0)>>4)*TSM_TILEWIDTH;
	QImage toptile = (this->imgSelectedBank.copy(x,y,TSM_TILEWIDTH,TSM_TILEWIDTH));
	newtile.setColor(0,toptile.color(0));
	newtile.setColor(1,toptile.color(1));
	newtile.setColor(2,toptile.color(2));
	newtile.setColor(3,toptile.color(3));

	uchar *newtilepixels = newtile.bits();
	uchar *toptilepixels = toptile.bits();
	for(quint8 y=0; y<TSM_TILEWIDTH; y++) {
		for(quint8 x=0; x<TSM_TILEWIDTH; x++) {
			newtilepixels[(y*TSM_TILEWIDTH)+x] = toptilepixels[(y*TSM_TILEWIDTH)+x];
		}
	}
	if(this->bTallSprite) {
		x = ((tile+1)&0x0F)*TSM_TILEWIDTH;
		y = (((tile%this->iBankDivider)&0xF0)>>4)*TSM_TILEWIDTH;
		QImage antoniostellabottomtile = (this->imgSelectedBank.copy(x,y,TSM_TILEWIDTH,TSM_TILEWIDTH));
		uchar *antoniostellabottomtilepixels = antoniostellabottomtile.bits();
		for(quint8 y=0; y<TSM_TILEWIDTH; y++) {
			for(quint8 x=0; x<TSM_TILEWIDTH; x++) {
				newtilepixels[(TSM_TILEWIDTH*TSM_TILEWIDTH)+(y*TSM_TILEWIDTH)+x] = antoniostellabottomtilepixels[(y*TSM_TILEWIDTH)+x];
			}
		}
	}

//	for(quint8 y=0; y<TSM_TILEWIDTH; y++) {
//		for(quint8 x=0; x<TSM_TILEWIDTH; x++) {
//			newtile.setPixel(x,y,toptile.pixelIndex(x,y));
//		}
//	}

//	if(this->bTallSprite) {
//		x = ((tile+1)&0x0F)*TSM_TILEWIDTH;
//		y = (((tile%this->iBankDivider)&0xF0)>>4)*TSM_TILEWIDTH;
//		QImage antoniostellabottomtile = (this->imgSelectedBank.copy(x,y,TSM_TILEWIDTH,TSM_TILEWIDTH));
//		for(quint8 y=0; y<TSM_TILEWIDTH; y++) {
//			for(quint8 x=0; x<TSM_TILEWIDTH; x++) {
//				newtile.setPixel(x,y+TSM_TILEWIDTH,antoniostellabottomtile.pixelIndex(x,y));
//			}
//		}
//	}

	return newtile;
}
