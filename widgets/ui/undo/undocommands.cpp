#include "undocommands.h"


ChangeMetatile::ChangeMetatile(MetatileItem *mt, quint8 subtile, quint8 tile, quint8 pal, QUndoCommand *parent) : QUndoCommand(parent)
{
	this->mtiEditPtr = mt;
	this->iSubtileIndex = subtile;
	this->iOldTile = this->mtiEditPtr->tileIndex(this->iSubtileIndex);
	this->iOldPalette = this->mtiEditPtr->palette();
	this->iNewTile = tile;
	this->iNewPalette = pal;
	this->bTileChanged = true;
	this->setText(QObject::tr("Replace tile %1 subtile %2 palette %3 with subtile %4 palette %5")
				  .arg(mt->metatileIndex())
				  .arg(this->iOldTile).arg(this->iOldPalette)
				  .arg(this->iNewTile).arg(this->iNewPalette));
}
ChangeMetatile::ChangeMetatile(MetatileItem *mt, quint8 pal, QUndoCommand *parent) : QUndoCommand(parent)
{
	this->mtiEditPtr = mt;
	this->iOldPalette = this->mtiEditPtr->palette();
	this->iNewPalette = pal;
	this->bTileChanged = false;
	this->setText(QObject::tr("Replace tile %1 palette %2 with palette %3")
				  .arg(mt->metatileIndex())
				  .arg(this->iOldPalette)
				  .arg(this->iNewPalette));
}
void ChangeMetatile::undo()
{
	if(this->bTileChanged)
		this->mtiEditPtr->setTileIndex(this->iSubtileIndex,this->iOldTile);
	this->mtiEditPtr->setPalette(this->iOldPalette);
}
void ChangeMetatile::redo()
{
	if(this->bTileChanged)
		this->mtiEditPtr->setTileIndex(this->iSubtileIndex,this->iNewTile);
	this->mtiEditPtr->setPalette(this->iNewPalette);
}



ChangeStageTile::ChangeStageTile(MetatileItem *tile, MetatileItem *replacement, QUndoCommand *parent) : QUndoCommand(parent)
{
	this->mtiEditPtr = tile;
	this->mtiTileOld.copy(tile);
	this->mtiTileNew.copy(replacement);
	this->setText(QObject::tr("Replace metatile %1 with %2")
				  .arg(tile->metatileIndex())
				  .arg(replacement->metatileIndex()));
}
void ChangeStageTile::undo()
{
	this->mtiEditPtr->setMetatileIndex(this->mtiTileOld.metatileIndex());
	this->mtiEditPtr->setPalette(this->mtiTileOld.palette());
	this->mtiEditPtr->setTileIndices(this->mtiTileOld.tileIndices());
}
void ChangeStageTile::redo()
{
	this->mtiEditPtr->setMetatileIndex(this->mtiTileNew.metatileIndex());
	this->mtiEditPtr->setPalette(this->mtiTileNew.palette());
	this->mtiEditPtr->setTileIndices(this->mtiTileNew.tileIndices());
}



AddObject::AddObject(ObjectItem *obj, quint8 id, QPointF pos, QUndoCommand *parent) : QUndoCommand(parent)
{
	this->objPtr = obj;
	this->objPtr->setId(id);
	this->iId = id;
	this->iSlot = this->objPtr->slot();
	this->pPos = pos;
	this->objPtr->setId(id);
	this->setText(QObject::tr("Add object $%1 to stage at (%2,%3)")
				  .arg(QString::number(this->objPtr->id(),16),2,'0')
				  .arg(this->pPos.x())
				  .arg(this->pPos.y()));
}
void AddObject::undo()
{
	this->objPtr->setEnabled(false);
}
void AddObject::redo()
{
	this->objPtr->setId(this->iId);
	this->objPtr->setEnabled(true);
	this->objPtr->setX(this->pPos.x());
	this->objPtr->setY(this->pPos.y());
	this->objPtr->setFlags(QGraphicsItem::ItemIsMovable|QGraphicsItem::ItemIsSelectable);
	this->objPtr->setSelected(true);
}

MoveObject::MoveObject(ObjectItem *obj, QPointF oldpos, QPointF newpos, QUndoCommand *parent) : QUndoCommand(parent)
{
	this->objPtr = obj;
	this->iId = this->objPtr->id();
	this->iSlot = this->objPtr->slot();
	this->pOldPos = oldpos;
	this->pNewPos = newpos;
}
void MoveObject::undo()
{
	this->objPtr->setId(this->iId);
	this->objPtr->setX(this->pOldPos.x());
	this->objPtr->setY(this->pOldPos.y());
}
void MoveObject::redo()
{
	this->objPtr->setId(this->iId);
	this->objPtr->setX(this->pNewPos.x());
	this->objPtr->setY(this->pNewPos.y());
	this->setText(this->generateText());
}
bool MoveObject::mergeWith(const QUndoCommand *command)
{
	const MoveObject *newcmd = static_cast<const MoveObject*>(command);
	if(this->id()!=newcmd->id() || this->iSlot!=newcmd->iSlot)
		return false;

	this->pNewPos = newcmd->pNewPos;
	this->setText(this->generateText());
	return true;
}
QString MoveObject::generateText()
{
	return QObject::tr("Move object $%1 from (%2,%3) to (%4,%5)")
			.arg(QString::number(this->objPtr->id(),16),2,'0')
			.arg(this->pOldPos.x()).arg(this->pOldPos.y())
			.arg(this->pNewPos.x()).arg(this->pNewPos.y());
}

DeleteObject::DeleteObject(ObjectItem *obj, QUndoCommand *parent) : QUndoCommand(parent)
{
	this->objPtr = obj;
	this->iId = this->objPtr->id();
	this->iSlot = this->objPtr->slot();
	this->pPos.setX(this->objPtr->x());
	this->pPos.setY(this->objPtr->y());
	this->setText(QObject::tr("Delete object $%1 from stage")
				  .arg(QString::number(this->objPtr->id(),16),2,'0'));
}
void DeleteObject::undo()
{
	this->objPtr->setId(this->iId);
	this->objPtr->setEnabled(true);
	this->objPtr->setX(this->pPos.x());
	this->objPtr->setY(this->pPos.y());
}
void DeleteObject::redo()
{
	this->objPtr->setEnabled(false);
}



MoveCheckpoint::MoveCheckpoint(CheckpointItem *chk, QPointF oldpos, QPointF newpos, QUndoCommand *parent) : QUndoCommand(parent)
{
	this->chkPtr = chk;
	this->iId = this->chkPtr->id();
	this->bEnabled = this->chkPtr->isEnabled();
	this->pOldPos = oldpos;
	this->pNewPos = newpos;
	return;
}
void MoveCheckpoint::undo()
{
	this->chkPtr->setId(this->iId);
	this->chkPtr->setEnabled(this->bEnabled);
	this->chkPtr->setX(this->pOldPos.x());
	this->chkPtr->setY(this->pOldPos.y());
}
void MoveCheckpoint::redo()
{
	this->chkPtr->setId(this->iId);
	this->chkPtr->setEnabled(true);
	this->chkPtr->setX(this->pNewPos.x());
	this->chkPtr->setY(this->pNewPos.y());
	this->setText(this->generateText());
}
bool MoveCheckpoint::mergeWith(const QUndoCommand *command)
{
	const MoveCheckpoint *newcmd = static_cast<const MoveCheckpoint*>(command);
	if(this->id()!=newcmd->id() || this->iId!=newcmd->iId || this->bEnabled!=newcmd->bEnabled)
		return false;

	this->pNewPos = newcmd->pNewPos;
	this->setText(this->generateText());
	return true;
}
QString MoveCheckpoint::generateText()
{
	return QObject::tr("Move checkpoint %1 from (%2,%3) to (%4,%5)")
			.arg(QString::number(this->iId,16),2,'0')
			.arg(this->pOldPos.x()).arg(this->pOldPos.y())
			.arg(this->pNewPos.x()).arg(this->pNewPos.y());
}

DeleteCheckpoint::DeleteCheckpoint(CheckpointItem *obj, QUndoCommand *parent) : QUndoCommand(parent)
{
	this->chkPtr = obj;
	this->pPos.setX(this->chkPtr->x());
	this->pPos.setY(this->chkPtr->y());
	this->setText(QObject::tr("Delete checkpoint $%1 from stage")
				  .arg(QString::number(this->chkPtr->id())));
}
void DeleteCheckpoint::undo()
{
	this->chkPtr->setX(this->pPos.x());
	this->chkPtr->setY(this->pPos.y());
	this->chkPtr->setEnabled(true);
}
void DeleteCheckpoint::redo()
{
	this->chkPtr->setEnabled(false);
}
