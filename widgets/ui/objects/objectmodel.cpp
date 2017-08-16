#include "objectmodel.h"

ObjectModel::ObjectModel(QObject *parent) : QAbstractTableModel(parent)
{
	for(int i=0; i<OM_OBJECT_COUNT; i++) {
		Object obj;
		obj.name = QString("");
		obj.imgdata = QString("");
		this->lObjects.append(obj);
	}
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
	switch(role) {
	case Qt::DisplayRole:
		if(index.column()==ObjectModel::ColumnName) {
			if(index.row()==0)
				return tr(OM_INVALID_OBJECT_NAME);
			else if(!this->lObjects[index.row()].name.isEmpty())
				return this->lObjects[index.row()].name;
			else
				return tr(OM_EMPTY_OBJECT_NAME);
		}
	case Qt::DecorationRole:
		if(index.column()==ObjectModel::ColumnImage)
			if(!this->lObjects[index.row()].imgdata.isEmpty())
				return QImage(this->lObjects[index.row()].imgdata).scaled(OM_OBJECT_IMG_DIM,OM_OBJECT_IMG_DIM,Qt::KeepAspectRatio);
			else
				return QImage(":/icons/noicon");
		break;
	case Qt::FontRole:
		if(!this->lObjects[index.row()].name.isEmpty()) {
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
	if(index.isValid() && role==Qt::EditRole) {
		switch(index.column()) {
		case ObjectModel::ColumnName:
			if(!value.toString().isEmpty())
				this->lObjects[index.row()].name = value.toString();
			break;
		case ObjectModel::ColumnImage:
			this->lObjects[index.row()].imgdata = value.toString();
			break;
		}
		emit(dataChanged(index,index));
		return true;
	}
	return false;
}

Qt::ItemFlags ObjectModel::flags(const QModelIndex &index) const
{
	if(index.isValid() && index.row()!=0)
		return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
	return QAbstractTableModel::flags(index);
}
