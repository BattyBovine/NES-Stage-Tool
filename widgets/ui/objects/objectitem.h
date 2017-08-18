#ifndef OBJECTITEM_H
#define OBJECTITEM_H

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

#include "objectcache.h"

class ObjectItem : public QGraphicsItem
{
public:
	ObjectItem(QGraphicsItem *parent = 0);
	ObjectItem(int,QGraphicsItem *parent = 0);
	ObjectItem(ObjectItem*,QGraphicsItem *parent = 0);

	enum { Type = UserType+4 };
	int type() const {return Type;}
	void copy(ObjectItem*);

	int x() const {return qFloor(QGraphicsItem::x());}
	int y() const {return qFloor(QGraphicsItem::y());}
	void setX(qreal x) {QGraphicsItem::setX(qFloor(x));}
	void setY(qreal y) {QGraphicsItem::setY(qFloor(y));}
	qreal width(){return ObjectCache::find(this->iId).img.width();}
	qreal height(){return ObjectCache::find(this->iId).img.height();}

	quint8 id(){return this->iId;}
	void setId(quint8 i){this->iId=i;}
	bool showInfo(){return this->bShowInfo;}

	int screen() {return (qFloor(this->x()/256)%256)+((qFloor(this->y()/192)%192)*8);}
	int screenX() {return (qFloor(qAbs(this->x()))%256);}
	int screenY() {return (qFloor(qAbs(this->y()))%192);}

	bool isNull(){return ObjectCache::find(this->iId).img.isNull();}

protected:
	QRectF boundingRect() const;
	void paint(QPainter*,const QStyleOptionGraphicsItem*,QWidget*);

	void mouseDoubleClickEvent(QGraphicsSceneMouseEvent*);

private:
	quint8  iId;
	bool	bShowInfo;
};

typedef QList<ObjectItem*>  ObjectList;

#endif // OBJECTITEM_H
