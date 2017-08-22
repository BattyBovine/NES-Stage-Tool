#ifndef CHECKPOINTMODEL_H
#define CHECKPOINTMODEL_H

#include <QObject>
#include <QAbstractListModel>
#include <QMimeData>
#include <QFont>
#include <QIcon>
#include <QTextStream>

#define CM_MIME_TYPE        "application/checkpointdata"

class CheckpointModel : public QAbstractListModel
{
	Q_OBJECT

public:
	CheckpointModel(QObject *parent = nullptr) : QAbstractListModel(parent) {this->iNumCheckpoints=0;}
	CheckpointModel(int i, QObject *parent = nullptr) : QAbstractListModel(parent) {this->iNumCheckpoints=i;}

	int rowCount(const QModelIndex&) const override { return this->iNumCheckpoints; }
	QVariant headerData(int, Qt::Orientation, int) const override;
	QVariant data(const QModelIndex&,int role = Qt::DisplayRole) const override;
	Qt::ItemFlags flags(const QModelIndex &index) const override;
	QStringList mimeTypes() const;
	QMimeData *mimeData(const QModelIndexList&) const;

public slots:
	void setCheckpointCount(int i){this->iNumCheckpoints=i;}

private:
	int iNumCheckpoints;
};

#endif // CHECKPOINTMODEL_H
