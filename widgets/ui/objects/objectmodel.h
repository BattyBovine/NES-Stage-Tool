#ifndef OBJECTMODEL_H
#define OBJECTMODEL_H

#include <QObject>
#include <QTableView>
#include <QAbstractTableModel>
#include <QMimeData>
#include <QFont>
#include <QPixmap>
#include <QImage>
#include <QTextStream>
#include <QSettings>

#include "objectcache.h"

#define OM_MIME_TYPE			"application/objectdata"
#define OM_OBJECT_COUNT			256
#define OM_OBJECT_IMG_DIM		32
#define OM_INVALID_OBJECT_NAME	"Invalid"
#define OM_EMPTY_OBJECT_NAME	"Unused"



class ObjectModel : public QAbstractTableModel
{
	Q_OBJECT

public:
	ObjectModel(QObject *parent = nullptr);
	int rowCount(const QModelIndex&) const override { return OM_OBJECT_COUNT; }
	int columnCount(const QModelIndex&) const override { return 2; }
	QVariant headerData(int, Qt::Orientation, int) const override;
	QVariant data(const QModelIndex&,int role = Qt::DisplayRole) const override;
	bool setData(const QModelIndex&,const QVariant&,int role=Qt::EditRole) override;
	Qt::ItemFlags flags(const QModelIndex &index) const override;
	QStringList mimeTypes() const;
	QMimeData *mimeData(const QModelIndexList&) const;
	enum { ColumnImage, ColumnName };

	void clear();
	void reload();

private:
	QSettings settingsObjects;
};

#endif // OBJECTMODEL_H
