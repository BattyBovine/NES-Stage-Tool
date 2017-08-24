#ifndef CHECKPOINTITEM_H
#define CHECKPOINTITEM_H

#include <QGraphicsItem>
#include <QSvgRenderer>
#include <QStyleOptionGraphicsItem>
#include <QPainter>
#include <QPen>
#include <QGraphicsSceneMouseEvent>

#include <QMessageBox>

#include <QtMath>

class CheckpointItem : public QGraphicsItem
{
public:
	CheckpointItem(QGraphicsItem *parent = 0);
	CheckpointItem(int,QGraphicsItem *parent = 0);
	CheckpointItem(CheckpointItem*,QGraphicsItem *parent = 0);
	~CheckpointItem();

	enum { Type = UserType+5 };
	int type() const {return Type;}
	void copy(CheckpointItem*);

	bool isNull(){return this->svg->isValid();}

	quint8 id(){return this->iId;}
	void setId(quint8 i){this->iId=i;}

	int screen() {return (qFloor(this->x()/256)%256)+((qFloor(this->y()/192)%192)*8);}
	int screenX() {return (qFloor(qAbs(this->x()))%256);}
	int screenY() {return (qFloor(qAbs(this->y()))%192);}

protected:
	QRectF boundingRect() const;
	void paint(QPainter*,const QStyleOptionGraphicsItem*,QWidget*);

private:
	QSvgRenderer	*svg;
	quint8			iId;
};

typedef QList<CheckpointItem*>  CheckpointList;

#endif // CHECKPOINTITEM_H
