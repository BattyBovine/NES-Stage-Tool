#ifndef METATILEITEM_H
#define METATILEITEM_H

#include <QGraphicsItem>
#include <QPixmapCache>
#include <QPaintEvent>
#include <QStyleOptionGraphicsItem>
#include <QPainter>
#include <QPen>
#include <QTimer>
#include <QGraphicsSceneMouseEvent>

#include <QMessageBox>

#include <QtMath>

#include "tilesetcache.h"

#define MTI_SUBTILEWIDTH 8
#define MTI_TILEWIDTH    16

class MetatileItem : public QGraphicsItem
{
public:
	MetatileItem(QGraphicsItem *parent = 0);
	MetatileItem(QImage,QGraphicsItem *parent = 0);
	MetatileItem(MetatileItem*,QGraphicsItem *parent = 0);

	enum { Type = UserType+3 };
	int type() const {return Type;}
	void copy(MetatileItem*);

	qreal realX(){return this->iX;}
	qreal realY(){return this->iY;}
	qreal scale(){return this->iScale;}
	void setRealX(qreal x){QGraphicsItem::setX(x*this->iScale);this->iX=x;}
	void setRealY(qreal y){QGraphicsItem::setY(y*this->iScale);this->iY=y;}
	void setScale(qreal s){QGraphicsItem::setScale(s);this->iScale=s;this->setRealX(this->iX);this->setRealY(this->iY);}
	qreal width(){return this->iW;}
	qreal height(){return this->iH;}

	void enableAnimation(bool);

	QImage img(){return this->imgTile;}
	void setImg(QImage i){this->imgTile=i;}
	quint8 screen(){return this->iScreen;}
	void setScreen(quint8 i){this->iScreen=i;}
	quint8 metatileIndex(){return this->iMetatile;}
	void setMetatileIndex(quint8 i){this->iMetatile=i;}
	quint8 palette(){return this->iPalette;}
	void setPalette(quint8 p){this->iPalette=p;}
	quint8 tileset(){return this->iTileset;}
	quint8* tileIndices(){return this->iTiles;}
	quint8 tileIndex(int i){return this->iTiles[i%4];}
	void setTileset(quint8 t){this->iTileset=t;}
	void setTileIndices(quint8 t[4]){for(int i=0;i<4;i++) this->iTiles[i]=t[i];}
	void setTileIndex(int i, quint8 t){this->iTiles[i%4]=t;}
	quint8 animFrame(){return this->iAnimFrame;}
	void setAnimFrame(int i){this->iAnimFrame=i;}
	QRgb getPaletteColour(quint8 i){return this->imgTile.color(i);}
    quint8 collision(){return this->iCollision;}
    void setCollision(quint8 i){this->iCollision=i;}
    bool destructible(){return this->bDestructible;}
    void setDestructible(bool b){this->bDestructible=b;}
    bool deadly(){return this->bDeadly;}
    void setDeadly(bool b){this->bDeadly=b;}

protected:
	QRectF boundingRect() const {return QRectF(0,0,MTI_TILEWIDTH,MTI_TILEWIDTH);}
	void paint(QPainter*,const QStyleOptionGraphicsItem*,QWidget*);

private:
	QImage  imgTile;
	QPixmap pixPixmap;
    quint8  iScreen,iMetatile,iPalette,iTileset,iAnimFrame,iCollision;
    bool    bDestructible,bDeadly;
	quint8  iTiles[4];
	qreal   iX,iY,iW,iH,iScale;
};

typedef QList<MetatileItem*>  MetatileList;
typedef QVector<MetatileList> ScreenList;

#endif // METATILEITEM_H
