#ifndef OBJECTMODEL_H
#define OBJECTMODEL_H

#include <QObject>
#include <QAbstractTableModel>
#include <QFont>
#include <QImage>
#include <QTextStream>

struct Object
{
	QString name;
	QString imgdata;
};
typedef QList<Object> ObjectData;

class ObjectModel : public QAbstractTableModel
{
	Q_OBJECT

public:
	ObjectModel(QObject *parent = NULL);
	int rowCount(const QModelIndex&) const override { return this->lObjects.count(); }
	int columnCount(const QModelIndex&) const override { return 2; }
	QVariant headerData(int, Qt::Orientation, int) const override;
	QVariant data(const QModelIndex&,int role = Qt::DisplayRole) const override;
	bool setData(const QModelIndex&,const QVariant&,int role=Qt::EditRole) override;
	Qt::ItemFlags flags(const QModelIndex &index) const override;

private:
	ObjectData lObjects;

	const QString sNoImgBase64 = "iVBORw0KGgoAAAANSUhEUgAAACAAAAAgCAYAAABzenr0AAAABmJLR0QA/wD/AP+gvaeTAAAACXBIWXMAAAsTAAALEwEAmpwYAAAAB3RJTUUH4QgMBjMSaDX+NAAAAAxpVFh0Q29tbWVudAAAAAAAvK6ymQAAAJBJREFUWMPtlsEOgDAIQyn//8/zZLwYN6DYGNejgTcna6fZ1t+Fu4fDxrgKgMoCMxaeGqovscLyLIjV42xgtBZRwGwc0V6wQNke2m6yXw2VeZ7gysjQ5YJV+6LjZEeyAx3ejgQX2AETTU03sb4zAukhlNpQGkTSKJZeRtLruLJ4huFsW0VrnQ2k5MCbv+VbWwcN52kezy1siAAAAABJRU5ErkJggg==";
};

#endif // OBJECTMODEL_H
