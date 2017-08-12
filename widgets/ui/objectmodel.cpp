#include "objectmodel.h"

ObjectModel::ObjectModel(QObject *parent) : QAbstractTableModel(parent)
{
	for(int i=0; i<256; i++) {
		Object obj;
		obj.name = QString("");
		obj.imgdata = this->sNoImgBase64;
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
		if(index.column()==1) {
			if(index.row()==0)
				return tr("Invalid");
			else if(!this->lObjects[index.row()].name.isEmpty())
				return this->lObjects[index.row()].name;
			else
				return tr("Unused");
		}
		break;
	case Qt::DecorationRole:
		if(index.column()==0)
			return QImage::fromData(QByteArray::fromBase64(this->lObjects[index.row()].imgdata.toLocal8Bit()),"PNG");
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
		case 1:
			this->lObjects[index.row()].name = value.toString();
			break;
		}
		emit(dataChanged(index,index));
		return true;
	}
	return false;
}

Qt::ItemFlags ObjectModel::flags(const QModelIndex &index) const
{
	if(index.isValid() && index.row()!=0 && index.column()!=0)
		return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
	return Qt::ItemIsEnabled;
}
