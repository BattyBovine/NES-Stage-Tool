#include "palettemanager.h"

PaletteManager::PaletteManager(QWidget *parent) : QGraphicsView(parent)
{
	this->gsFullPaletteScene = new QGraphicsScene();
	this->gsFullPaletteScene->setSceneRect(0,0,224,64);
	this->gsFullPaletteScene->setBackgroundBrush(QBrush(QColor(0x82,0x87,0x90)));
	this->iFullPaletteIndex = 0x0D;

	for(int i=0; i<PM_SUBPALETTES_MAX; i++) {
		this->gsSpritePaletteScene[i] = new QGraphicsScene();
		this->gsSpritePaletteScene[i]->setSceneRect(0,0,64,16);
	}

	this->iSpritePaletteSelected = this->iSpritePaletteSelectedIndex = 0;
	this->griSelectionBox[0] = this->griSelectionBox[1] = 0;

	this->setScene(this->gsFullPaletteScene);

	this->clearAllPaletteData();
}

PaletteManager::~PaletteManager()
{
	delete this->gsFullPaletteScene;
	for(int i=0; i<4; i++) {
		delete this->gsSpritePaletteScene[i];
	}
}



void PaletteManager::dropEvent(QDropEvent *e)
{
	e->acceptProposedAction();

	foreach(QUrl url, e->mimeData()->urls()) {
		if(url.toLocalFile().endsWith(".pal",Qt::CaseInsensitive)) {
			this->openPaletteFile(url.toLocalFile());
			return;
		}
	}
}

void PaletteManager::mousePressEvent(QMouseEvent *e)
{
	QPointF p = this->mapToScene(e->pos());
	if((p.x()<0||p.y()<0) || (p.x()>((0x0E*PM_SWATCH_SIZE)-1))||(p.y()>((0x04*PM_SWATCH_SIZE)-1)))  return;
	quint8 pval = (qFloor(p.y()/PM_SWATCH_SIZE)<<4)|(qFloor(p.x()/PM_SWATCH_SIZE));
	this->iFullPaletteIndex = pval;
	if(pval==0x0D)  pval = 0x0F;

	if(this->iSpritePaletteSelectedIndex == 0) {
		for(int i=0; i<4; i++) {
			this->iSpritePaletteIndices[this->iGlobalPalette][i][0] = pval;
		}
	} else {
		this->iSpritePaletteIndices[this->iGlobalPalette][this->iSpritePaletteSelected][this->iSpritePaletteSelectedIndex] = pval;
	}

	this->drawSelectionBox(this->gsFullPaletteScene, this->iFullPaletteIndex);
	this->generateNewSpritePalettes(false);
}

void PaletteManager::setPaletteFile(QString p)
{
	this->sPaletteFile = p;
}

void PaletteManager::setSpritePaletteScene(QObject* o)
{
	((SpritePaletteView*)o)->setScene(this->gsSpritePaletteScene[((SpritePaletteView*)o)->objectName().at(((SpritePaletteView*)o)->objectName().length()-1).digitValue()]);
}

void PaletteManager::spritePaletteSelected(QString s, quint8 i)
{
	quint8 pindex = (s.at(s.length()-1)).digitValue();
	this->iSpritePaletteSelected = pindex;
	this->iSpritePaletteSelectedIndex = i;

	this->iFullPaletteIndex = this->iSpritePaletteIndices[this->iGlobalPalette][pindex][i];
	if(this->iFullPaletteIndex==0x0F)   this->iFullPaletteIndex=0x0D;
//	this->drawFullPaletteColours(this->sPaletteFile);
	this->drawSelectionBox(this->gsFullPaletteScene, this->iFullPaletteIndex);
	this->generateNewSpritePalettes(false);

	QVector<QColor> newspritecolours;
	for(int i=0; i<4; i++)  newspritecolours.append(this->vPaletteColours.at(this->iSpritePaletteIndices[this->iGlobalPalette][pindex][i]));
}

QVector<QRgb> PaletteManager::createPaletteColours()
{
	QVector<QRgb> p;
	for(int ts=0; ts<PM_GLOBAL_PALETTE_COUNT; ts++) {
		for(int sp=0; sp<PM_SUBPALETTES_MAX; sp++) {
			for(int c=0; c<PM_PALETTE_COLOURS_MAX; c++) {
				p.append(this->vPaletteColours.at(this->iSpritePaletteIndices[ts][sp][c]).rgb());
			}
		}
	}
	return p;
}

void PaletteManager::generateNewSpritePalettes(bool changeselected)
{
	for(int spritepal=0; spritepal<PM_SUBPALETTES_MAX; spritepal++) {
		this->gsSpritePaletteScene[spritepal]->clear();
		for(int i=0; i<PM_PALETTE_COLOURS_MAX; i++) {
			quint8 index = this->iSpritePaletteIndices[this->iGlobalPalette][spritepal][i];
			if(index==0x0F) index=0x0D;
			this->gsSpritePaletteScene[spritepal]->addRect(QRectF((i*PM_SWATCH_SIZE),0,16,16),Qt::NoPen,this->vPaletteColours.at(index));
		}
	}

	this->drawSelectionBox(this->gsSpritePaletteScene[this->iSpritePaletteSelected], this->iSpritePaletteSelectedIndex);

	emit(newSpritePalette0(this->gsSpritePaletteScene[0]));
	emit(newSpritePalette1(this->gsSpritePaletteScene[1]));
	emit(newSpritePalette2(this->gsSpritePaletteScene[2]));
	emit(newSpritePalette3(this->gsSpritePaletteScene[3]));

	emit(newSpriteColours(this->createPaletteColours(), this->iSpritePaletteSelected, changeselected));
}

void PaletteManager::setNewGlobalPalette(int p)
{
	this->iGlobalPalette = p;
	this->generateNewSpritePalettes();
}

void PaletteManager::generateNewGlobalPalette()
{
	emit(newGlobalPalette(this->createPaletteColours()));
}

#ifdef METASPRITETILEITEM_H
void PaletteManager::setNewSpritePalette(MetaspriteTileItem *t)
{
	quint8 p = t->palette();
	QVector<QRgb> c = this->createPaletteColours();
	t->setNewColours(c.at((p*PM_PALETTE_COLOURS_MAX)+1),
					 c.at((p*PM_PALETTE_COLOURS_MAX)+2),
					 c.at((p*PM_PALETTE_COLOURS_MAX)+3),
					 p);
}
#endif

void PaletteManager::setNewMetatilePalette(MetatileItem *t)
{
	t->setPalette(this->iSpritePaletteSelected);
//	emit(metatileUpdated(t));
}

void PaletteManager::sendRequestedPaletteUpdate(quint8 p)
{
	emit(newSpriteColours(this->createPaletteColours(), p, true));
}



bool PaletteManager::drawFullPaletteColours(QString palfile)
{
	this->sPaletteFile = palfile;

	this->gsFullPaletteScene->clear();

	QFile pal(":/pal/"+this->sPaletteFile+".pal");
	if(!pal.open(QIODevice::ReadOnly)) return false;
	QByteArray baPaletteBlob = pal.readAll();
	pal.close();

	this->vPaletteColours.clear();
	for(int i=0; i<baPaletteBlob.length(); i+=3) {
		quint8 r = quint8(baPaletteBlob.at(i));
		quint8 g = quint8(baPaletteBlob.at(i+1));
		quint8 b = quint8(baPaletteBlob.at(i+2));
		this->vPaletteColours.append(QColor(r,g,b));
	}

	for(int i=0; i<4; i++) {
		for(int j=0; j<14; j++) {
			int p = ((i<<4)|j);
			this->gsFullPaletteScene->addRect(QRectF((j*16),(i*16),16,16),Qt::NoPen,this->vPaletteColours.at(p));
		}
	}

	this->drawSelectionBox(this->gsFullPaletteScene, this->iFullPaletteIndex);

    this->generateNewSpritePalettes(false);

	return true;
}

bool PaletteManager::drawSelectionBox(QGraphicsScene *s, quint8 i)
{
	quint8 xorigin = (i&0x0F)*PM_SWATCH_SIZE;
	quint8 yorigin = ((i&0xF0)>>4)*PM_SWATCH_SIZE;

	QPen dashes(Qt::black,1,Qt::DashLine);
	QVector<qreal> dp;
	dp << 2 << 2;
	dashes.setDashPattern(dp);

	if(s==this->gsFullPaletteScene) {
		if(this->griSelectionBox[0] && s->items().contains(this->griSelectionBox[0]))
			this->gsFullPaletteScene->removeItem(this->griSelectionBox[0]);
		if(this->griSelectionBox[1] && s->items().contains(this->griSelectionBox[1]))
			this->gsFullPaletteScene->removeItem(this->griSelectionBox[1]);

		this->griSelectionBox[0] = s->addRect(QRectF(xorigin,yorigin,PM_SWATCH_SIZE-1,PM_SWATCH_SIZE-1),QPen(Qt::white),Qt::NoBrush);
		this->griSelectionBox[1] = s->addRect(QRectF(xorigin,yorigin,PM_SWATCH_SIZE-1,PM_SWATCH_SIZE-1),dashes,Qt::NoBrush);
	} else {
		s->addRect(QRectF(xorigin,yorigin,PM_SWATCH_SIZE-1,PM_SWATCH_SIZE-1),QPen(Qt::white),Qt::NoBrush);
		s->addRect(QRectF(xorigin,yorigin,PM_SWATCH_SIZE-1,PM_SWATCH_SIZE-1),dashes,Qt::NoBrush);
	}

	return true;
}

void PaletteManager::clearAllPaletteData()
{
	this->iGlobalPalette = 0;
	for(int p=0; p<PM_GLOBAL_PALETTE_COUNT; p++) {
		for(int i=0; i<PM_SUBPALETTES_MAX; i++) {
			this->iSpritePaletteIndices[p][i][0] = 0x0F;
			this->iSpritePaletteIndices[p][i][1] = 0x00;
			this->iSpritePaletteIndices[p][i][2] = 0x10;
			this->iSpritePaletteIndices[p][i][3] = 0x30;
		}
	}
}



bool PaletteManager::openPaletteFile(QString filename)
{
	QFile file(filename);
	if(!file.open(QIODevice::ReadOnly|QIODevice::Text)) {
		QMessageBox::warning(this,tr(PM_FILE_OPEN_ERROR_TITLE),tr(PM_FILE_OPEN_ERROR_BODY),QMessageBox::NoButton);
		return false;
	}
	QVector<QByteArray> inputbytes;
	bool palettesfound = false;
	while(!file.atEnd()) {
		QString line = file.readLine();

		QRegularExpression bytes(",?\\$([0-9a-fA-F]+)");
		QRegularExpressionMatchIterator bytesiter = bytes.globalMatch(line);
		QByteArray bytesin;
		while(bytesiter.hasNext()) {
			QRegularExpressionMatch bytesmatch = bytesiter.next();
			bytesin.append(quint8(bytesmatch.captured(1).toUInt(NULL,16)));
		}

		QRegularExpression tilesetlabel("^(.*?)_palette(.*?)");
		QRegularExpressionMatch tilesetlabelmatch = tilesetlabel.match(line);
		if(tilesetlabelmatch.hasMatch()) {
			palettesfound = true;
		}

		if(!bytesin.isEmpty() && palettesfound && bytesin.size()==(PM_SUBPALETTES_MAX*PM_SUBPALETTES_MAX)) {
			inputbytes.append(bytesin);
		}
	}

	file.close();
	if(inputbytes.isEmpty()) {
		if(!file.open(QIODevice::ReadOnly)) {
			QMessageBox::warning(this,tr(PM_FILE_OPEN_ERROR_TITLE),tr(PM_FILE_OPEN_ERROR_BODY),QMessageBox::NoButton);
			return false;
		}
		while(!file.atEnd()) {
			inputbytes.append(file.read(PM_SUBPALETTES_MAX*PM_SUBPALETTES_MAX));
		}
	}

	this->importPaletteBinaryData(inputbytes);
	return true;
}

bool PaletteManager::importPaletteBinaryData(QVector<QByteArray> inputbytes)
{
	if(inputbytes.size()==1) {
		if(inputbytes[0].size()!=(PM_SUBPALETTES_MAX*PM_SUBPALETTES_MAX)) {
			QMessageBox::critical(this,tr(PM_FILE_OPEN_ERROR_TITLE),tr("The file is too ")+((inputbytes[0].size()<(PM_SUBPALETTES_MAX*PM_SUBPALETTES_MAX))?tr("short"):tr("long"))+tr(" to be palette data."),QMessageBox::NoButton);
			return false;
		}
		for(int i=0; i<PM_SUBPALETTES_MAX; i++) {
			for(int j=0; j<PM_PALETTE_COLOURS_MAX; j++) {
				this->iSpritePaletteIndices[this->iGlobalPalette][i][j] = (inputbytes[0][j+(PM_PALETTE_COLOURS_MAX*i)]&0x3F);
			}
		}
	} else {
		for(int ts=0; ts<PM_GLOBAL_PALETTE_COUNT&&ts<inputbytes.size(); ts++) {
			if(inputbytes[ts].size()!=(PM_SUBPALETTES_MAX*PM_SUBPALETTES_MAX)) {
				QMessageBox::critical(this,tr(PM_FILE_OPEN_ERROR_TITLE),tr("The file is too ")+((inputbytes[ts].size()<(PM_SUBPALETTES_MAX*PM_SUBPALETTES_MAX))?tr("short"):tr("long"))+tr(" to be palette data."),QMessageBox::NoButton);
				return false;
			}
			for(int i=0; i<PM_SUBPALETTES_MAX; i++) {
				for(int j=0; j<PM_PALETTE_COLOURS_MAX; j++) {
					this->iSpritePaletteIndices[ts][i][j] = (inputbytes[ts][j+(PM_PALETTE_COLOURS_MAX*i)]&0x3F);
				}
			}
		}
	}
	this->generateNewSpritePalettes(true);
	this->drawSelectionBox(this->gsFullPaletteScene, this->iSpritePaletteIndices[this->iGlobalPalette][this->iSpritePaletteSelected][this->iSpritePaletteSelectedIndex]);
	return true;
}

QString PaletteManager::createPaletteASMData(QString labelprefix, int tileset)
{
	QString asmlabel = labelprefix.isEmpty()?"emptylabel":labelprefix;
	asmlabel += "_palette";
	QString databytes;

	QString countedlabel = asmlabel+QString("_%1").arg(tileset,2,16,QChar('0')).toUpper();
	databytes += countedlabel+":\n\t.byte ";
	for(int sp=0; sp<PM_SUBPALETTES_MAX; sp++) {
		for(int col=0; col<PM_PALETTE_COLOURS_MAX; col++) {
			databytes += QString("$%1").arg(this->iSpritePaletteIndices[tileset][sp][col],2,16,QChar('0')).append(",").toUpper();
		}
	}
	databytes = databytes.left(databytes.length()-1);
	databytes += "\n";

	return databytes;
}

QByteArray PaletteManager::createPaletteBinaryData(int tileset)
{
	QByteArray indices;
	for(int i=0; i<PM_SUBPALETTES_MAX; i++) {
		for(int j=0; j<PM_PALETTE_COLOURS_MAX; j++) {
			indices.append(this->iSpritePaletteIndices[tileset][i][j]);
		}
	}
	return indices;
}
