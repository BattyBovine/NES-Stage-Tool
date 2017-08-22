#include "objectmodel.h"

ObjectModel::ObjectModel(QObject *parent) : QAbstractTableModel(parent)
{
	this->settingsObjects.beginGroup("Objects");
	this->reload();
}

QVariant ObjectModel::headerData(int segment, Qt::Orientation, int role) const
{
	switch(role) {
	case Qt::DisplayRole:
		return QString("%1").arg(QString::number(segment,16).toUpper(),2,'0');
	case Qt::FontRole:
		return QFont("Courier");
	}
	return QVariant();
}

QVariant ObjectModel::data(const QModelIndex &index, int role) const
{
	Object obj = ObjectCache::find(index.row());

	switch(role) {
	case Qt::DisplayRole:
		if(index.column()==ObjectModel::ColumnName) {
			if(index.row()==0)
				return tr(OM_INVALID_OBJECT_NAME);
			else if(!obj.name.isEmpty())
				return obj.name;
			else
				return tr(OM_EMPTY_OBJECT_NAME);
		}
	case Qt::DecorationRole:
		if(index.column()==ObjectModel::ColumnImage)
			if(!obj.img.isNull())
				return obj.img.scaled(OM_OBJECT_IMG_DIM,OM_OBJECT_IMG_DIM,Qt::KeepAspectRatio);
			else
				return QPixmap(":/icons/noicon");
		break;
	case Qt::FontRole:
		if(!obj.name.isEmpty()) {
			QFont bold;
			bold.setBold(true);
			return bold;
		}
		break;
	}
	return QVariant();
}

bool ObjectModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	Object obj = ObjectCache::find(index.row());

	if(index.isValid() && role==Qt::EditRole) {
		switch(index.column()) {
		case ObjectModel::ColumnName:
			if(!value.toString().isEmpty()) {
				obj.name = value.toString();
				this->settingsObjects.setValue(QString("Name%1").arg(QString::number(index.row(),16),2,'0'),value);
			}
			break;
		case ObjectModel::ColumnImage:
			if(!value.toString().isEmpty()) {
				obj.img = value.toString();
				this->settingsObjects.setValue(QString("Image%1").arg(QString::number(index.row(),16),2,'0'),value);
			}
			break;
		}
		emit(dataChanged(index,index));
		ObjectCache::replace(index.row(),obj);
		return true;
	}
	return false;
}

Qt::ItemFlags ObjectModel::flags(const QModelIndex &index) const
{
	if(index.isValid() && index.row()!=0)
		return QAbstractTableModel::flags(index) | Qt::ItemIsEditable | Qt::ItemIsDragEnabled;
	return QAbstractTableModel::flags(index);
}

QStringList ObjectModel::mimeTypes() const
{
	QStringList types;
	types << OM_MIME_TYPE;
	return types;
}

QMimeData *ObjectModel::mimeData(const QModelIndexList &indices) const
{
	QMimeData *mimedata = new QMimeData();
	QByteArray encodeddata;
	QDataStream stream(&encodeddata,QIODevice::WriteOnly);
	foreach(QModelIndex i, indices) {
		if(i.isValid()) stream << i.row();
	}
	mimedata->setData(OM_MIME_TYPE,encodeddata);
	return mimedata;
}



void ObjectModel::clear()
{
//	this->lObjects.clear();
	for(int i=0; i<OM_OBJECT_COUNT; i++) {
		Object obj;
		obj.name = QString("");
		obj.img = QString("");
		ObjectCache::replace(i,obj);
		this->settingsObjects.remove(QString("Name%1").arg(QString::number(i,16),2,'0'));
		this->settingsObjects.remove(QString("Image%1").arg(QString::number(i,16),2,'0'));
	}
}

void ObjectModel::reload()
{
//	this->lObjects.clear();
	for(int i=0; i<OM_OBJECT_COUNT; i++) {
		Object obj;
		obj.name = this->settingsObjects.value(QString("Name%1").arg(QString::number(i,16),2,'0')).toString();
		obj.img = this->settingsObjects.value(QString("Image%1").arg(QString::number(i,16),2,'0')).toString();
		ObjectCache::replace(i,obj);
	}
}
