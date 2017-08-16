#ifndef OBJECTDELEGATE_H
#define OBJECTDELEGATE_H

#include <QObject>
#include <QPushButton>
#include <QStyledItemDelegate>
#include <QItemEditorFactory>
#include <QFileDialog>

#include "objectmodel.h"

class ObjectDelegate : public QStyledItemDelegate
{
	Q_OBJECT

public:
	ObjectDelegate(QWidget *parent=0) : QStyledItemDelegate(parent) {}

	QWidget *createEditor(QWidget*,const QStyleOptionViewItem&,const QModelIndex&) const override;
	void setModelData(QWidget *,QAbstractItemModel*,const QModelIndex&) const override;

public slots:
	void getNewImageFile(QString);
};

#endif // OBJECTDELEGATE_H
