#include "globaltilesetmanager.h"

GlobalTilesetManager::GlobalTilesetManager(QWidget *parent) : QGraphicsView(parent)
{
	this->gsTileset = new QGraphicsScene();
	this->setScene(this->gsTileset);

	this->imgTileset = QImage(128, 128, QImage::Format_Indexed8);

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

void GlobalTilesetManager::wheelEvent(QWheelEvent *e)
{
	this->getGlobalTilesetDelta(-qFloor(((qreal)e->angleDelta().y()/8)/15));
}



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
    if(!filename.isEmpty()) {
        if(!this->fswCHR.files().isEmpty()) this->fswCHR.removePath(this->sCurrentTilesetFile);
        this->sCurrentTilesetFile = filename;
        this->fswCHR.addPath(this->sCurrentTilesetFile);
    }
    this->threadCHR->loadFile(this->sCurrentTilesetFile);
}

void GlobalTilesetManager::loadCHRBank(int set)
{
	int bankheight = qFloor(128/this->iBankDivider);
	uchar *src = this->imgTileset.bits();
	for(int pal=0; pal<PM_SUBPALETTES_MAX; pal++) {
		for(int anim=0; anim<GTSM_ANIM_FRAMES; anim++) {
			QImage destimg = QImage(128, 128, QImage::Format_Indexed8);
			destimg.fill(0);
			uchar *dest = destimg.bits();
			for(int i=0; i<this->iBankDivider; i++) {
				int bankval = this->iBankLists[set][i]+(i==(this->iBankDivider-1)?anim:0);
				for(int j=0; j<128*bankheight; j++) {
					int destpixely = (128*bankheight*i);
					int srcpixely = (128*bankheight*(bankval%(this->imgTileset.height()/bankheight)));
					dest[destpixely+j] = src[srcpixely+j];
				}
			}
			if(set==this->iGlobalTileset) {
				destimg.setColor(0,qRgb(0x00,0x00,0x00));
				destimg.setColor(1,qRgb(0x55,0x55,0x55));
				destimg.setColor(2,qRgb(0xAA,0xAA,0xAA));
				destimg.setColor(3,qRgb(0xFF,0xFF,0xFF));
				this->pixLocalCache[anim] = QPixmap::fromImage(destimg);
			}
			for(int col=0; col<PM_PALETTE_COLOURS_MAX; col++)
				destimg.setColor(col,vPaletteLists[set][pal][col]);
			TilesetCache::insert(set,pal,anim,QPixmap::fromImage(destimg));
		}
	}
	this->iAnimFrame = 0;
	this->gpiTileset->setPixmap(this->pixLocalCache[this->iAnimFrame]);
	emit(tilesetChanged(this->imgTileset));
	emit(newAnimationFrame(this->iAnimFrame));

	this->resizeEvent(NULL);
}

void GlobalTilesetManager::loadCHRBank()
{
	for(int ts=0; ts<GTSM_TILESET_COUNT; ts++) {
		this->loadCHRBank(ts);
	}
}



void GlobalTilesetManager::getNewCHRData(QImage img)
{
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
	for(int ts=0; ts<GTSM_TILESET_COUNT; ts++) {
		for(int p=0; p<PM_SUBPALETTES_MAX; p++) {
			for(int c=0; c<PM_PALETTE_COLOURS_MAX; c++)
				this->vPaletteLists[ts][p].replace(c,pal[ts*(PM_SUBPALETTES_MAX*PM_PALETTE_COLOURS_MAX)+(p*PM_PALETTE_COLOURS_MAX)+c]);
		}
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
	this->gpiTileset->setPixmap(this->pixLocalCache[this->iAnimFrame]);
	emit(newAnimationFrame(this->iAnimFrame));
}

void GlobalTilesetManager::switchToNextAnimBank()
{
	this->iAnimFrame++;
	if(this->iAnimFrame>=4) this->iAnimFrame = 0;
	this->tAnimation.start(GTSM_ANIM_DELAY);
	this->gpiTileset->setPixmap(this->pixLocalCache[this->iAnimFrame]);
	emit(newAnimationFrame(this->iAnimFrame));
}

void GlobalTilesetManager::clearAllTilesetData()
{
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
}



bool GlobalTilesetManager::openTilesetFile(QString filename)
{
	QFile file(filename);
	if(!file.open(QIODevice::ReadOnly|QIODevice::Text)) {
		QMessageBox::warning(this,tr(GTSM_FILE_OPEN_ERROR_TITLE),tr(GTSM_FILE_OPEN_ERROR_BODY),QMessageBox::NoButton);
		return false;
	}
	QVector<QByteArray> inputbytes;
	while(!file.atEnd()) {
		QString line = file.readLine();

		QRegularExpression bytes(",?\\$([0-9a-fA-F]+)");
		QRegularExpressionMatchIterator bytesiter = bytes.globalMatch(line);
		QByteArray bytesin;
		while(bytesiter.hasNext()) {
			QRegularExpressionMatch bytesmatch = bytesiter.next();
			bytesin.append(quint8(bytesmatch.captured(1).toUInt(NULL,16)));
		}
		if(!bytesin.isEmpty() && bytesin.size()==this->iBankDivider) {
			inputbytes.append(bytesin);
		}
	}

	file.close();
	if(inputbytes.isEmpty()) {
		if(!file.open(QIODevice::ReadOnly)) {
			QMessageBox::warning(this,tr(GTSM_FILE_OPEN_ERROR_TITLE),tr(GTSM_FILE_OPEN_ERROR_BODY),QMessageBox::NoButton);
			return false;
		}
		while(!file.atEnd()) {
			inputbytes.append(file.read(this->iBankDivider));
		}
	}

	this->importTilesetBinaryData(inputbytes);
	this->loadCHRBank();
	return true;
}

bool GlobalTilesetManager::importTilesetBinaryData(QVector<QByteArray> inputbytes)
{
	for(int ts=0; ts<GTSM_TILESET_COUNT && ts<inputbytes.size(); ts++) {
		if(inputbytes[ts].size()!=this->iBankDivider) {
			QMessageBox::critical(this,tr(GTSM_FILE_OPEN_ERROR_TITLE),tr("The file is too ")+((inputbytes[ts].size()<this->iBankDivider)?tr("short"):tr("long"))+tr(" to be tileset data."),QMessageBox::NoButton);
			return false;
		}
		for(int j=0; j<this->iBankDivider; j++) {
			this->iBankLists[ts][j] = inputbytes[ts][j];
		}
	}
	this->getGlobalTileset(this->iGlobalTileset);
	return true;
}

QString GlobalTilesetManager::createTilesetASMData(QString labelprefix)
{
	QString asmlabel = labelprefix.isEmpty()?"emptylabel":labelprefix;
	asmlabel += "_tileset";
	QString databytes;

	for(int ts=0; ts<GTSM_TILESET_COUNT; ts++) {
		QString countedlabel = asmlabel+QString("_%1").arg(ts,2,16,QChar('0')).toUpper();
		databytes += countedlabel+":\n\t.byte ";
		for(int bank=0; bank<this->iBankDivider; bank++)
			databytes += QString("$%1").arg(this->iBankLists[ts][bank],2,16,QChar('0')).append(",").toUpper();
		databytes = databytes.left(databytes.length()-1);
		databytes += "\n";
	}

	return databytes;
}

QByteArray GlobalTilesetManager::createTilesetBinaryData()
{
	QByteArray indices;
	for(int ts=0; ts<GTSM_TILESET_COUNT; ts++) {
		for(int bank=0; bank<this->iBankDivider; bank++)
			indices.append(this->iBankLists[ts][bank]);
	}
	return indices;
}
