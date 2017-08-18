#include "objectitem.h"
#include "globaltilesetmanager.h"

ObjectItem::ObjectItem(QGraphicsItem *parent) : QGraphicsItem(parent)
{
	this->bShowInfo = true;
	this->setZValue(0);
}

ObjectItem::ObjectItem(int id, QGraphicsItem *parent) : QGraphicsItem(parent)
{
	this->bShowInfo = true;
	this->setZValue(0);
	this->iId = id;
}

ObjectItem::ObjectItem(ObjectItem *i, QGraphicsItem *parent) : QGraphicsItem(parent)
{
	this->copy(i);
}



QRectF ObjectItem::boundingRect() const {
	Object obj = ObjectCache::find(this->iId);
	return QRectF(-qFloor(obj.img.width()/2.0f)+1,-obj.img.height()+1,
				  obj.img.width(),obj.img.height());
}

void ObjectItem::paint(QPainter *p, const QStyleOptionGraphicsItem*, QWidget*)
{
	Object obj = ObjectCache::find(this->iId);
	p->drawPixmap(-(obj.img.width()/2.0f)+1,-obj.img.height()+1,obj.img);

	if(this->isSelected()) {
		if(this->bShowInfo) {
			QFont font;
			font.setStyleStrategy(QFont::PreferBitmap);
			p->setFont(font);
			p->setPen(QColor(Qt::white));
			p->drawText(qFloor(obj.img.width()/2.0f)+4,-obj.img.height()+7,QString("%1 %2")
						.arg(QString::number(this->iId,16),2,'0')
						.arg(obj.name));
			p->drawText(qFloor(obj.img.width()/2.0f)+4,-obj.img.height()+18,QString("%1 (%2,%3)")
						.arg(QString::number(this->screen(),16).toUpper(),2,'0')
						.arg(QString::number(this->screenX()),3,'0')
						.arg(QString::number(this->screenY()),3,'0'));
		}

		p->setPen(QPen(Qt::black, 0, Qt::SolidLine));
		p->setBrush(Qt::NoBrush);
		p->drawRect(this->boundingRect());

		p->setPen(QPen(Qt::white, 0, Qt::DashLine));
		p->setBrush(Qt::NoBrush);
		p->drawRect(this->boundingRect());
	}
}

void ObjectItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent*)
{
	this->bShowInfo = !this->bShowInfo;
}



void ObjectItem::copy(ObjectItem *i)
{
	this->bShowInfo = i->showInfo();
	this->setZValue(i->zValue());
	this->setId(i->id());
}
