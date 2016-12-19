#include "metatileitem.h"

MetatileItem::MetatileItem(QGraphicsItem *parent) : QGraphicsItem(parent)
{
	this->setScale(1);
	this->setRealX(0);
	this->setRealY(0);
	this->iW = this->iH = MTI_TILEWIDTH;
	this->iScreen = 0x00;
	this->iMetatile = 0x00;
	this->iPalette = 0x00;
	this->iTileset = 0x00;
	this->iTiles[0] = 0x00;
	this->iTiles[1] = 0x00;
	this->iTiles[2] = 0x00;
	this->iTiles[3] = 0x00;
	this->imgTile = QImage(this->iW,this->iH,QImage::Format_Indexed8);
	this->imgTile.fill(0);
	this->imgTile.setColor(0,qRgba(0x00,0x00,0x00,0x00));
	this->imgTile.setColor(1,qRgb(0x00,0x00,0x00));
	this->imgTile.setColor(2,qRgb(0x00,0x00,0x00));
	this->imgTile.setColor(3,qRgb(0x00,0x00,0x00));
	this->setZValue(-100);
}

MetatileItem::MetatileItem(QImage img, QGraphicsItem *parent) : QGraphicsItem(parent)
{
	MetatileItem();
	this->imgTile = img;
}

MetatileItem::MetatileItem(MetatileItem *i, QGraphicsItem *parent) : QGraphicsItem(parent)
{
	MetatileItem();
	this->copy(i);
}



void MetatileItem::paint(QPainter *p, const QStyleOptionGraphicsItem*, QWidget*)
{
	p->drawPixmap(0,0,MetatileDictionary::find(QString(MTI_PIXMAP_KEY_FORMAT).arg(this->iMetatile)));
}



void MetatileItem::copy(MetatileItem *i)
{
	this->setScale(i->scale());
	this->setRealX(i->realX());
	this->setRealY(i->realY());
	this->iW = i->width();
	this->iH = i->height();
	this->iPalette = i->palette();
	this->iScreen = i->screen();
	this->iMetatile = i->metatileIndex();
	this->iTileset = i->tileset();
	this->iTiles[0] = i->tileIndex(0);
	this->iTiles[1] = i->tileIndex(1);
	this->iTiles[2] = i->tileIndex(2);
	this->iTiles[3] = i->tileIndex(3);
	this->imgTile = i->img();
	this->setZValue(i->zValue());
}



void MetatileItem::setTile(quint8 i, QImage img)
{
	for(int y=0; y<img.height(); y++) {
		for(int x=0; x<img.width(); x++) {
			quint8 xtrans = x+(i&0x01?img.width():0);
			quint8 ytrans = y+(i>=2?img.width():0);
			uint pixel = img.pixelIndex(x,y);
			this->imgTile.setPixel(xtrans,ytrans,pixel);
		}
	}
	MetatileItem::insertPixmap(this->iMetatile,QPixmap::fromImage(this->imgTile));
	this->update(this->imgTile.rect());
}

void MetatileItem::setNewColours(QRgb a, QRgb b, QRgb c, quint8 p)
{
	this->iPalette = p;
	this->imgTile.setColor(1,a);
	this->imgTile.setColor(2,b);
	this->imgTile.setColor(3,c);
	MetatileItem::insertPixmap(this->iMetatile,QPixmap::fromImage(this->imgTile));
	this->update(this->imgTile.rect());
}
