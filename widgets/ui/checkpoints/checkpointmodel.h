#ifndef CHECKPOINTMODEL_H
#define CHECKPOINTMODEL_H

#include <QObject>
#include <QAbstractListModel>
#include <QFont>
#include <QImage>
#include <QTextStream>

#define CM_CHECKPOINT_COUNT 8

class CheckpointModel : public QAbstractListModel
{
	Q_OBJECT

public:
	CheckpointModel(QObject *parent = nullptr) : QAbstractListModel(parent) {}

	int rowCount(const QModelIndex&) const override { return CM_CHECKPOINT_COUNT; }
	QVariant headerData(int, Qt::Orientation, int) const override;
	QVariant data(const QModelIndex&,int role = Qt::DisplayRole) const override;
	Qt::ItemFlags flags(const QModelIndex &index) const override;
};

#endif // CHECKPOINTMODEL_H
