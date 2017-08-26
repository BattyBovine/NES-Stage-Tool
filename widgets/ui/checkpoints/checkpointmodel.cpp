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
		QIcon icon(QString(":/icons/cp%1").arg(QString::number(index.row())));
		return icon.pixmap(QSize(24,24));
	}
	return QVariant();
}

Qt::ItemFlags CheckpointModel::flags(const QModelIndex &index) const
{
	return QAbstractListModel::flags(index) | Qt::ItemIsDragEnabled;
}

QStringList CheckpointModel::mimeTypes() const
{
	QStringList types;
	types << CM_MIME_TYPE;
	return types;
}

QMimeData *CheckpointModel::mimeData(const QModelIndexList &indices) const
{
	QMimeData *mimedata = new QMimeData();
	QByteArray encodeddata;
	QDataStream stream(&encodeddata,QIODevice::WriteOnly);
	foreach(QModelIndex i, indices) {
		if(i.isValid()) stream << i.row();
	}
	mimedata->setData(CM_MIME_TYPE,encodeddata);
	return mimedata;
}
