#include "checkpointitem.h"
#include "globaltilesetmanager.h"

CheckpointItem::CheckpointItem(QGraphicsItem *parent) : QGraphicsItem(parent)
{
	this->setZValue(500);
	this->svg = new QSvgRenderer(QString(":/icons/cp"));
}

CheckpointItem::CheckpointItem(int id, QGraphicsItem *parent) : QGraphicsItem(parent)
{
	this->setZValue(500);
	this->svg = new QSvgRenderer(QString(":/icons/cp"));
	this->iId = id;
}

CheckpointItem::CheckpointItem(CheckpointItem *i, QGraphicsItem *parent) : QGraphicsItem(parent)
{
	this->copy(i);
	this->svg = new QSvgRenderer(QString(":/icons/cp"));
}

CheckpointItem::~CheckpointItem()
{
	this->svg->deleteLater();
}



QRectF CheckpointItem::boundingRect() const {
	QSize size = this->svg->defaultSize();
	return QRectF(-qFloor(size.width()/2),-size.height(),
				  size.width(),size.height());
}

void CheckpointItem::paint(QPainter *p, const QStyleOptionGraphicsItem*, QWidget*)
{
	if(this->isEnabled()) {
		this->svg->render(p,this->boundingRect());
		QFont font;
		font.setFamily("Helvetica");
		font.setStyleHint(QFont::SansSerif);
		p->setFont(font);
		p->setPen(QColor(Qt::white));
		QRectF box = this->boundingRect();
		p->drawText(box.x()+5,box.y()+9,QString("%1").arg(this->iId));

		if(this->isSelected()) {
			font.setFamily("Courier");
			font.setStyleHint(QFont::Monospace);
			p->setFont(font);
			p->setPen(QColor(Qt::white));
			p->drawText(qFloor(this->boundingRect().width()/2.0f)+4,
						-this->boundingRect().height()+6,
						QObject::tr("Checkpoint %1").arg(this->iId));
			p->drawText(qFloor(this->boundingRect().width()/2.0f)+4,-this->boundingRect().height()+17,QString("%1 (%2,%3)")
						.arg(QString::number(this->screen(),16).toUpper(),2,'0')
						.arg(QString::number(this->screenX()),3,'0')
						.arg(QString::number(this->screenY()),3,'0'));

			p->setPen(QPen(Qt::black, 0, Qt::SolidLine));
			p->setBrush(Qt::NoBrush);
			p->drawRect(this->boundingRect());

			p->setPen(QPen(Qt::white, 0, Qt::DashLine));
			p->setBrush(Qt::NoBrush);
			p->drawRect(this->boundingRect());
		}
	}
}



void CheckpointItem::copy(CheckpointItem *i)
{
	this->setZValue(i->zValue());
	this->setId(i->id());
}
