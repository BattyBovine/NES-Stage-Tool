#include "checkpointmodel.h"

QVariant CheckpointModel::headerData(int segment, Qt::Orientation, int role) const
{
	switch(role) {
	case Qt::DisplayRole:
		return QString("%1").arg(QString::number(segment,16).toUpper(),2,'0');
	case Qt::FontRole:
		return QFont("Courier");
	}
	return QVariant();
}

QVariant CheckpointModel::data(const QModelIndex &index, int role) const
{
	switch(role) {
	case Qt::DisplayRole:
		return tr("Checkpoint %1").arg(QString::number(index.row()));
	case Qt::DecorationRole:
		QString file = QString(":/icons/cp%1").arg(QString::number(index.row()));
		QImage icon(file);
		return icon;
	}
	return QVariant();
}

Qt::ItemFlags CheckpointModel::flags(const QModelIndex &index) const
{
	return QAbstractListModel::flags(index);
}
