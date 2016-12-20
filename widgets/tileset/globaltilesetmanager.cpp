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
	this->imgSelectedBank = QImage(this->imgTileset);

	this->gpiTileset = new QGraphicsPixmapItem();
	this->gsTileset->addItem(this->gpiTileset);
	this->griSelection[0] = this->griSelection[1] = NULL;
	this->iSelectedTile = 0;
	this->iPalette = 0;
	this->bTallSprite = false;
	this->iBankDivider = 1;
	this->iSelectedBank = 0;


	this->iGlobalTileset = 0;
	for(int i=0; i<GTSM_TILESET_COUNT; i++) {
		for(int j=0; j<GTSM_BANKS_COUNT; j++) {
			this->iBankLists[i][j] = 0;
		}
	}
	this->iAnimBank = 0;
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

void GlobalTilesetManager::setSelectedBank(quint16 bankno)
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



bool GlobalTilesetManager::drawSelectionBox()
{
	if(this->griSelection[0] && this->griSelection[1]) {
		this->gsTileset->removeItem(this->griSelection[0]);
		this->gsTileset->removeItem(this->griSelection[1]);
		delete this->griSelection[0];
		delete this->griSelection[1];
	}

	quint8 xorigin = qFloor(this->pSelection.x())-(qFloor(this->pSelection.x())%((GTSM_TILEWIDTH*GTSM_SCALE)*(this->bTallSprite?2:1)));
	quint16 yorigin = (qFloor(this->pSelection.y())-(qFloor(this->pSelection.y())%(GTSM_TILEWIDTH*GTSM_SCALE)))%(this->iBankDivider);
	this->pSelection = QPointF(xorigin,yorigin);
	this->iSelectedTile = ((qFloor((yorigin/(GTSM_TILEWIDTH*GTSM_SCALE))<<4)|qFloor(xorigin/(GTSM_TILEWIDTH*GTSM_SCALE)))%this->iBankDivider);

	QPen dashes(Qt::black,1,Qt::DashLine);
	QVector<qreal> dp;
	dp << 2 << 2;
	dashes.setDashPattern(dp);
	this->griSelection[0] = this->gsTileset->addRect(QRectF(xorigin,yorigin,(GTSM_TILEWIDTH*GTSM_SCALE)*(this->bTallSprite?2:1)-1,(GTSM_TILEWIDTH*GTSM_SCALE)-1),QPen(Qt::white),Qt::NoBrush);
	this->griSelection[1] = this->gsTileset->addRect(QRectF(xorigin,yorigin,(GTSM_TILEWIDTH*GTSM_SCALE)*(this->bTallSprite?2:1)-1,(GTSM_TILEWIDTH*GTSM_SCALE)-1),dashes,Qt::NoBrush);

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

void GlobalTilesetManager::loadCHRBank()
{
	int bankheight = qFloor(128/this->iBankDivider);
	uchar *src = this->imgTileset.bits();
	uchar *dest = this->imgSelectedBank.bits();
	for(int i=0; i<this->iBankDivider; i++) {
//		int yoffset = bankheight*this->iBankList[i];
		for(int j=0; j<128*bankheight; j++) {
			int bankval = this->iBankLists[this->iGlobalTileset][i];
			if(this->bAnimation && i==(this->iBankDivider-1)) bankval += this->iAnimBank;
			dest[(128*bankheight*i)+j] = src[(128*bankheight*(bankval%(this->imgTileset.height()/bankheight)))+j];
		}
	}
//	this->setFixedSize(256,this->iBankDivider);
//	this->setSceneRect(0,0,256,this->iBankDivider);
	this->imgSelectedBank.setColor(1,this->imgTileset.color(1));
	this->imgSelectedBank.setColor(2,this->imgTileset.color(2));
	this->imgSelectedBank.setColor(3,this->imgTileset.color(3));

	this->gpiTileset->setPixmap(QPixmap::fromImage(this->imgSelectedBank));
	emit(tilesetChanged(this->imgSelectedBank));

	this->resizeEvent(NULL);
}

void GlobalTilesetManager::setNewSpriteColours(PaletteVector c, quint8 i)
{
	this->gsTileset->setBackgroundBrush(QBrush(QColor(c.at(PM_PALETTE_COLOURS_MAX*i))));
	this->imgTileset.setColor(1,c.at((PM_PALETTE_COLOURS_MAX*i)+1));
	this->imgTileset.setColor(2,c.at((PM_PALETTE_COLOURS_MAX*i)+2));
	this->imgTileset.setColor(3,c.at((PM_PALETTE_COLOURS_MAX*i)+3));
	this->iPalette = i;

	this->loadCHRBank();
}



void GlobalTilesetManager::getNewTile(QPointF p)
{
	quint32 selection = this->iSelectedTile+(this->iBankDivider*this->iSelectedBank);
	emit(sendNewTile(p,this->createNewTile(selection),selection,this->iPalette));
}

void GlobalTilesetManager::getNewSubtile(quint8 i, MetatileItem *t)
{
	quint8 selection = this->iSelectedTile+(this->iBankDivider*this->iSelectedBank);
	t->setTileIndex(i,selection);
	t->setPalette(this->iPalette);
	this->updateMetatile(t);
	emit(sendNewMetatile(t));
}

void GlobalTilesetManager::updateMetatile(MetatileItem *t)
{
	t->setTile(0,this->createNewTile((t->tileIndex(0)+(this->iSelectedBank*this->iBankDivider))));
	t->setTile(1,this->createNewTile((t->tileIndex(1)+(this->iSelectedBank*this->iBankDivider))));
	t->setTile(2,this->createNewTile((t->tileIndex(2)+(this->iSelectedBank*this->iBankDivider))));
	t->setTile(3,this->createNewTile((t->tileIndex(3)+(this->iSelectedBank*this->iBankDivider))));
	emit(this->metatileUpdated(t));
}

void GlobalTilesetManager::getNewCHRData(QImage img)
{
	img.setColor(1,this->imgTileset.color(1));
	img.setColor(2,this->imgTileset.color(2));
	img.setColor(3,this->imgTileset.color(3));
	this->imgTileset = img;

	this->loadCHRBank();
	emit(this->chrDataChanged(this->imgTileset));
}

void GlobalTilesetManager::getCHRError(QString title,QString body)
{
	QMessageBox::warning(NULL,title,body,QMessageBox::NoButton);
}

void GlobalTilesetManager::getBankSize(int bankdiv)
{
	this->iBankDivider = qPow(2,bankdiv);
	this->loadCHRBank();
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
}

void GlobalTilesetManager::getGlobalTilesetDelta(int d)
{
	int temp = ((this->iGlobalTileset+d)<0)?0:((this->iGlobalTileset+d)>=GTSM_TILESET_COUNT)?GTSM_TILESET_COUNT-1:(this->iGlobalTileset+d);
	if(temp!=this->iGlobalTileset)	emit(this->setGlobalTilesetSelectedIndex(temp));
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
	this->loadCHRBank();
}

void GlobalTilesetManager::enableAnimation(bool b)
{
	this->bAnimation = b;
	this->iAnimBank = 0;
	if(b)
		this->tAnimation.start(50);
	else
		this->tAnimation.stop();
	this->loadCHRBank();
}

void GlobalTilesetManager::switchToNextAnimBank()
{
	this->iAnimBank++;
	if(this->iAnimBank>=4/* || (this->iBankList[this->iBankDivider-1]+this->iAnimBank)>256*/)
		this->iAnimBank = 0;
	this->loadCHRBank();
	this->tAnimation.start(50);
}



QImage GlobalTilesetManager::createNewTile(quint32 tile)
{
	QImage newtile(GTSM_TILEWIDTH, GTSM_TILEWIDTH, QImage::Format_Indexed8);

	int x = (tile&0x0F)*GTSM_TILEWIDTH;
	int y = (((tile%this->iBankDivider)&0xF0)>>4)*GTSM_TILEWIDTH;
	QImage toptile = (this->imgSelectedBank.copy(x,y,GTSM_TILEWIDTH,GTSM_TILEWIDTH));
	newtile.setColor(0,toptile.color(0));
	newtile.setColor(1,toptile.color(1));
	newtile.setColor(2,toptile.color(2));
	newtile.setColor(3,toptile.color(3));

	uchar *newtilepixels = newtile.bits();
	uchar *toptilepixels = toptile.bits();
	for(quint8 y=0; y<GTSM_TILEWIDTH; y++) {
		for(quint8 x=0; x<GTSM_TILEWIDTH; x++) {
			newtilepixels[(y*GTSM_TILEWIDTH)+x] = toptilepixels[(y*GTSM_TILEWIDTH)+x];
		}
	}

	return newtile;
}
