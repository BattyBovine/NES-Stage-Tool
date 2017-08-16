#include "objectdelegate.h"

QWidget *ObjectDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem&, const QModelIndex &index) const
{
	switch(index.column()) {
	case ObjectModel::ColumnName:
		return QItemEditorFactory::defaultFactory()->createEditor(index.data(Qt::DisplayRole).userType(),parent);
	case ObjectModel::ColumnImage:
		QFileDialog *replace = new QFileDialog(nullptr,tr("Open Object Sprite"),"",tr("PNG Image (*.png)"));
		replace->setFileMode(QFileDialog::ExistingFile);
		replace->setOption(QFileDialog::DontUseNativeDialog,true);
		connect(replace,SIGNAL(fileSelected(QString)),this,SLOT(getNewImageFile(QString)));
		return replace;
	}
	return nullptr;
}

void ObjectDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
	switch(index.column()) {
	case ObjectModel::ColumnName:
		QStyledItemDelegate::setModelData(editor,model,index);
		break;
	case ObjectModel::ColumnImage:
		QFileDialog *replace = qobject_cast<QFileDialog*>(editor);
		model->setData(index,replace->whatsThis());
		break;
	}
}

void ObjectDelegate::getNewImageFile(QString file)
{
	if(!file.isEmpty()) {
		QFileDialog *replace = qobject_cast<QFileDialog*>(sender());
		replace->setWhatsThis(file);
		replace->saveState();
		emit(commitData(replace));
		emit(closeEditor(replace));
	}
}
