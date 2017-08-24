#ifndef UNDOCOMMANDS_H
#define UNDOCOMMANDS_H

#include <QUndoCommand>
#include <QGraphicsScene>

#include "metatileitem.h"
#include "objectitem.h"
#include "checkpointitem.h"


// Tile modification commands
class ChangeStageTile : public QUndoCommand
{
public:
	ChangeStageTile(MetatileItem*,MetatileItem*,QUndoCommand *parent = NULL);
	void undo();
	void redo();
	MetatileItem *tile(){return this->mtiEditPtr;}
private:
	MetatileItem *mtiEditPtr;
	MetatileItem mtiTileOld;
	MetatileItem mtiTileNew;
};

// Object modification commands
class AddObject : public QUndoCommand
{
public:
	AddObject(ObjectItem*,quint8,QPointF,QUndoCommand *parent = NULL);
	void undo();
	void redo();
private:
	ObjectItem *objPtr;
	quint8 iId,iSlot;
	QPointF pPos;
};
class MoveObject : public QUndoCommand
{
public:
	MoveObject(ObjectItem*,QPointF,QPointF,QUndoCommand *parent = NULL);
	void undo();
	void redo();
//	bool mergeWith(const QUndoCommand*) override;
	QString generateText();
//	int id() const override {return Id;}
//	enum{Id=43971};
private:
	ObjectItem *objPtr;
	quint8 iId,iSlot;
	QPointF pOldPos,pNewPos;
};
class DeleteObject : public QUndoCommand
{
public:
	DeleteObject(ObjectItem*,QUndoCommand *parent = NULL);
	void undo();
	void redo();
private:
	ObjectItem *objPtr;
	quint8 iId,iSlot;
	QPointF pPos;
};

// Checkpoint modification commands
class MoveCheckpoint : public QUndoCommand
{
public:
	MoveCheckpoint(CheckpointItem*,QPointF,QPointF,QUndoCommand *parent = NULL);
	void undo();
	void redo();
	bool mergeWith(const QUndoCommand*) override;
	QString generateText();
	int id() const override {return Id;}
	enum{Id=21867};
private:
	CheckpointItem *chkPtr;
	quint8 iId;
	bool bEnabled;
	QPointF pOldPos;
	QPointF pNewPos;
};
class DeleteCheckpoint : public QUndoCommand
{
public:
	DeleteCheckpoint(CheckpointItem*,QUndoCommand *parent = NULL);
	void undo();
	void redo();
private:
	CheckpointItem *chkPtr;
	QPointF pPos;
};

#endif // UNDOCOMMANDS_H
