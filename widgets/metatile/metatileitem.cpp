#include "metatileitem.h"
#include "globaltilesetmanager.h"

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
	this->iAnimFrame = 0x00;
	this->iTiles[0] = 0x00;
	this->iTiles[1] = 0x00;
	this->iTiles[2] = 0x00;
	this->iTiles[3] = 0x00;
    this->iCollision = 0x00;
    this->bDestructible = false;
    this->bDeadly = false;
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
	QPixmap tileset = TilesetCache::find(this->iTileset,this->iPalette,this->iAnimFrame);
	QRectF tile0 = QRect((this->iTiles[0]%16)*MTI_SUBTILEWIDTH,qFloor(this->iTiles[0]/16)*MTI_SUBTILEWIDTH,MTI_SUBTILEWIDTH,MTI_SUBTILEWIDTH);
	QRectF tile1 = QRect((this->iTiles[1]%16)*MTI_SUBTILEWIDTH,qFloor(this->iTiles[1]/16)*MTI_SUBTILEWIDTH,MTI_SUBTILEWIDTH,MTI_SUBTILEWIDTH);
	QRectF tile2 = QRect((this->iTiles[2]%16)*MTI_SUBTILEWIDTH,qFloor(this->iTiles[2]/16)*MTI_SUBTILEWIDTH,MTI_SUBTILEWIDTH,MTI_SUBTILEWIDTH);
	QRectF tile3 = QRect((this->iTiles[3]%16)*MTI_SUBTILEWIDTH,qFloor(this->iTiles[3]/16)*MTI_SUBTILEWIDTH,MTI_SUBTILEWIDTH,MTI_SUBTILEWIDTH);
	p->drawPixmap(QRectF(0,0,MTI_SUBTILEWIDTH,MTI_SUBTILEWIDTH),tileset,tile0);
	p->drawPixmap(QRectF(MTI_SUBTILEWIDTH,0,MTI_SUBTILEWIDTH,MTI_SUBTILEWIDTH),tileset,tile1);
	p->drawPixmap(QRectF(0,MTI_SUBTILEWIDTH,MTI_SUBTILEWIDTH,MTI_SUBTILEWIDTH),tileset,tile2);
	p->drawPixmap(QRectF(MTI_SUBTILEWIDTH,MTI_SUBTILEWIDTH,MTI_SUBTILEWIDTH,MTI_SUBTILEWIDTH),tileset,tile3);
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
