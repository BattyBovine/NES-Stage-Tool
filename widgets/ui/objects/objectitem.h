#ifndef OBJECTITEM_H
#define OBJECTITEM_H

#include <QGraphicsItem>
#include <QPainter>
#include <QPen>
#include <QGraphicsSceneMouseEvent>

#include <QMessageBox>

#include <QtMath>

#include "objectcache.h"

class ObjectItem : public QGraphicsItem
{
public:
	ObjectItem(QGraphicsItem *parent = 0);
	ObjectItem(int,int slot=0,QGraphicsItem *parent = 0);
	ObjectItem(ObjectItem*,QGraphicsItem *parent = 0);

	enum { Type = UserType+4 };
	int type() const {return Type;}
	void copy(ObjectItem*);

	void setX(qreal x) {this->iX=qFloor(x)%256;this->iScreen=(qFloor(x/256)%8)+(qFloor(this->iScreen/8)*8);QGraphicsItem::setX(qFloor(x));}
	void setY(qreal y) {this->iY=qFloor(y)%192;this->iScreen=(this->iScreen%8)+((qFloor(y/192)*8)%(4*8));QGraphicsItem::setY(qFloor(y));}
	void setPos(QPointF p) {this->setX(p.x());this->setY(p.y());}

	qreal width() {return ObjectCache::find(this->iId).img.width();}
	qreal height() {return ObjectCache::find(this->iId).img.height();}

	quint8 id(){return this->iId;}
	void setId(quint8 i){this->iId=i;}
	quint8 slot(){return this->iSlot;}

	quint8 screen() {return this->iScreen;}
	quint8 screenX() {return this->iX;}
	quint8 screenY() {return this->iY;}

	bool isNull(){return ObjectCache::find(this->iId).img.isNull();}

protected:
	QRectF boundingRect() const;
	void paint(QPainter*,const QStyleOptionGraphicsItem*,QWidget*);

private:
	quint8 iId,iSlot;
	quint8 iScreen,iX,iY;
};

typedef QList<ObjectItem*>  ObjectList;

#endif // OBJECTITEM_H
