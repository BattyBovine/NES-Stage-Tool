#include "globaltilesetmanager.h"

GlobalTilesetManager::GlobalTilesetManager(QWidget *parent) : QGraphicsView(parent)
{
	this->gsTileset = new QGraphicsScene();
	this->setScene(this->gsTileset);

	this->imgTileset = QImage(128, 128, QImage::Format_Indexed8);
	this->imgTileset.fill(0);
	this->imgTileset.setColor(0,qRgb(0x00,0x00,0x00));
	this->imgTileset.setColor(1,qRgb(0x55,0x55,0x55));
	this->imgTileset.setColor(2,qRgb(0xAA,0xAA,0xAA));
	this->imgTileset.setColor(3,qRgb(0xFF,0xFF,0xFF));
//	this->imgSelectedBank = QImage(this->imgTileset);

	this->gpiTileset = new QGraphicsPixmapItem();
	this->gsTileset->addItem(this->gpiTileset);
	this->griSelection[0] = this->griSelection[1] = NULL;
	this->iSelectedTile = this->iSelectedPalette = 0;
	this->iBankDivider = 1;


	this->iGlobalTileset = 0;
	for(int t=0; t<GTSM_TILESET_COUNT; t++) {
		for(int b=0; b<GTSM_BANKS_COUNT; b++) {
			this->iBankLists[t][b] = 0;
		}
		for(int p=0; p<PM_SUBPALETTES_MAX; p++) {
			for(int c=0; c<PM_PALETTE_COLOURS_MAX; c++)
				this->vPaletteLists[t][p].append(qRgb(0x00,0x00,0x00));
		}
	}
	this->iAnimFrame = 0;
	this->bAnimation = false;

	this->pSelection = QPointF(0,0);

	this->threadCHR = new CHRThread();
	connect(this->threadCHR,SIGNAL(sendCHRImageData(QImage)),this,SLOT(getNewCHRData(QImage)));
	connect(this->threadCHR,SIGNAL(error(QString,QString)),this,SLOT(getCHRError(QString,QString)));

	connect(&this->fswCHR,SIGNAL(fileChanged(QString)),this,SLOT(reloadCurrentTileset()));

	connect(&this->tAnimation,SIGNAL(timeout()),this,SLOT(switchToNextAnimBank()));
}

GlobalTilesetManager::~GlobalTilesetManager()
{
	this->gsTileset->removeItem(this->gpiTileset);
	this->threadCHR->deleteLater();
	delete this->gpiTileset;
	delete this->gsTileset;
}



void GlobalTilesetManager::resizeEvent(QResizeEvent*)
{
	QRectF viewrect = this->mapToScene(this->rect()).boundingRect();
	this->gpiTileset->setScale(qFloor(qMin(viewrect.width(),viewrect.height())/this->imgTileset.width()));
	this->setSceneRect(0,0,
					   this->imgTileset.width()*this->gpiTileset->scale(),
					   this->imgTileset.width()*this->gpiTileset->scale());
}

void GlobalTilesetManager::dropEvent(QDropEvent *e)
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

void GlobalTilesetManager::mousePressEvent(QMouseEvent *e)
{
	this->pSelection = this->mapToScene(e->pos());
}

//void GlobalTilesetManager::wheelEvent(QWheelEvent *e)
//{
//	qreal steps = -(((qreal)e->angleDelta().y()/8)/15);

//	if(((this->iSelectedBank+steps)>=1) && ((this->iSelectedBank+steps)<=((this->imgTileset.height()/(this->iBankDivider>>1))-1)))
//		this->iSelectedBank += steps;
//	else
//		this->iSelectedBank = ((steps<0)?0:((this->imgTileset.height()/(this->iBankDivider>>1))-1));

//	this->loadCHRBank();
//	emit(chrBankChanged(this->iSelectedBank));
//}



bool GlobalTilesetManager::drawSelectionBox()
{
	if(this->griSelection[0] && this->griSelection[1]) {
		this->gsTileset->removeItem(this->griSelection[0]);
		this->gsTileset->removeItem(this->griSelection[1]);
		delete this->griSelection[0];
		delete this->griSelection[1];
	}

	quint8 xorigin = qFloor(this->pSelection.x())-(qFloor(this->pSelection.x())%(GTSM_TILEWIDTH*GTSM_SCALE));
	quint16 yorigin = (qFloor(this->pSelection.y())-(qFloor(this->pSelection.y())%(GTSM_TILEWIDTH*GTSM_SCALE)))%(this->iBankDivider);
	this->pSelection = QPointF(xorigin,yorigin);
	this->iSelectedTile = ((qFloor((yorigin/(GTSM_TILEWIDTH*GTSM_SCALE))<<4)|qFloor(xorigin/(GTSM_TILEWIDTH*GTSM_SCALE)))%this->iBankDivider);

	QPen dashes(Qt::black,1,Qt::DashLine);
	QVector<qreal> dp;
	dp << 2 << 2;
	dashes.setDashPattern(dp);
	this->griSelection[0] = this->gsTileset->addRect(QRectF(xorigin,yorigin,(GTSM_TILEWIDTH*GTSM_SCALE)-1,(GTSM_TILEWIDTH*GTSM_SCALE)-1),QPen(Qt::white),Qt::NoBrush);
	this->griSelection[1] = this->gsTileset->addRect(QRectF(xorigin,yorigin,(GTSM_TILEWIDTH*GTSM_SCALE)-1,(GTSM_TILEWIDTH*GTSM_SCALE)-1),dashes,Qt::NoBrush);

	return true;
}

void GlobalTilesetManager::reloadCurrentTileset()
{
	threadCHR->loadFile(this->sCurrentTilesetFile);
}



void GlobalTilesetManager::loadCHRData(QString filename)
{
	this->threadCHR->loadFile(filename);
	if(!this->fswCHR.files().isEmpty()) this->fswCHR.removePath(this->sCurrentTilesetFile);
	this->sCurrentTilesetFile = filename;
	this->fswCHR.addPath(filename);
}

void GlobalTilesetManager::loadCHRBank(int set)
{
	int bankheight = qFloor(128/this->iBankDivider);
	uchar *src = this->imgTileset.bits();
	for(int pal=0; pal<PM_SUBPALETTES_MAX; pal++) {
		for(int anim=0; anim<GTSM_ANIM_FRAMES; anim++) {
			QImage destimg = QImage(128, 128, QImage::Format_Indexed8);
			uchar *dest = destimg.bits();
			for(int i=0; i<this->iBankDivider; i++) {
//				int yoffset = bankheight*this->iBankList[i];
				int bankval = this->iBankLists[set][i]+(i==(this->iBankDivider-1)?anim:0);
				for(int j=0; j<128*bankheight; j++)
					dest[(128*bankheight*i)+j] = src[(128*bankheight*(bankval%(this->imgTileset.height()/bankheight)))+j];
			}
//			this->setFixedSize(256,this->iBankDivider);
//			this->setSceneRect(0,0,256,this->iBankDivider);
			for(int col=0; col<PM_PALETTE_COLOURS_MAX; col++)
				destimg.setColor(col,vPaletteLists[set][pal][col]);
//			this->gpiTileset->setPixmap(QPixmap::fromImage(this->imgSelectedBank));
			TilesetCache::insert(set,pal,anim,QPixmap::fromImage(destimg));
		}
	}
	this->iAnimFrame = 0;
	this->gpiTileset->setPixmap(TilesetCache::find(this->iGlobalTileset,this->iSelectedPalette,this->iAnimFrame));
	emit(tilesetChanged(this->imgSelectedBank));

	this->resizeEvent(NULL);
}



void GlobalTilesetManager::getNewCHRData(QImage img)
{
	img.setColor(1,this->imgTileset.color(1));
	img.setColor(2,this->imgTileset.color(2));
	img.setColor(3,this->imgTileset.color(3));
	this->imgTileset = img;

	for(int sets=0; sets<GTSM_TILESET_COUNT; sets++) this->loadCHRBank(sets);
}

void GlobalTilesetManager::getCHRError(QString title,QString body)
{
	QMessageBox::warning(NULL,title,body,QMessageBox::NoButton);
}

void GlobalTilesetManager::getBankSize(int bankdiv)
{
	this->iBankDivider = qPow(2,bankdiv);
	this->loadCHRBank(this->iGlobalTileset);
	emit(this->chrDataChanged(this->imgTileset));
}

void GlobalTilesetManager::getGlobalTileset(int tileset)
{
	this->iGlobalTileset = tileset;
	emit(banksChanged(
			this->iBankLists[this->iGlobalTileset][0],
			this->iBankLists[this->iGlobalTileset][1],
			this->iBankLists[this->iGlobalTileset][2],
			this->iBankLists[this->iGlobalTileset][3],
			this->iBankLists[this->iGlobalTileset][4],
			this->iBankLists[this->iGlobalTileset][5],
			this->iBankLists[this->iGlobalTileset][6],
			this->iBankLists[this->iGlobalTileset][7] ));
	this->iAnimFrame = 0;
}

void GlobalTilesetManager::getGlobalTilesetDelta(int d)
{
	int temp = ((this->iGlobalTileset+d)<0)?0:((this->iGlobalTileset+d)>=GTSM_TILESET_COUNT)?GTSM_TILESET_COUNT-1:(this->iGlobalTileset+d);
	this->iAnimFrame = 0;
	emit(this->setGlobalTilesetSelectedIndex(temp));
}

void GlobalTilesetManager::getGlobalPalette(PaletteVector pal, quint8 s)
{
	this->iSelectedPalette = s;
	for(int p=0; p<PM_SUBPALETTES_MAX; p++) {
		for(int c=0; c<PM_PALETTE_COLOURS_MAX; c++)
			this->vPaletteLists[this->iGlobalTileset][p].replace(c,pal.at((p*PM_PALETTE_COLOURS_MAX)+c));
	}
	this->loadCHRBank(this->iGlobalTileset);
}

void GlobalTilesetManager::getBankSelections(int b0, int b1,
											 int b2, int b3,
											 int b4, int b5,
											 int b6, int b7)
{
	this->iBankLists[this->iGlobalTileset][0] = b0;
	this->iBankLists[this->iGlobalTileset][1] = b1;
	this->iBankLists[this->iGlobalTileset][2] = b2;
	this->iBankLists[this->iGlobalTileset][3] = b3;
	this->iBankLists[this->iGlobalTileset][4] = b4;
	this->iBankLists[this->iGlobalTileset][5] = b5;
	this->iBankLists[this->iGlobalTileset][6] = b6;
	this->iBankLists[this->iGlobalTileset][7] = b7;
	this->loadCHRBank(this->iGlobalTileset);
}

void GlobalTilesetManager::enableAnimation(bool b)
{
	this->bAnimation = b;
	this->iAnimFrame = 0;
	if(b)
		this->tAnimation.start(GTSM_ANIM_DELAY);
	else
		this->tAnimation.stop();
	this->gpiTileset->setPixmap(TilesetCache::find(this->iGlobalTileset,this->iSelectedPalette,this->iAnimFrame));
	emit(newAnimationFrame(this->iAnimFrame));
}

void GlobalTilesetManager::switchToNextAnimBank()
{
	this->iAnimFrame++;
	if(this->iAnimFrame>=4) this->iAnimFrame = 0;
	this->tAnimation.start(GTSM_ANIM_DELAY);
	this->gpiTileset->setPixmap(TilesetCache::find(this->iGlobalTileset,this->iSelectedPalette,this->iAnimFrame));
	emit(newAnimationFrame(this->iAnimFrame));
}
