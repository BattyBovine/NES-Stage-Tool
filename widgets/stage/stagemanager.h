#ifndef STAGEMANAGER_H
#define STAGEMANAGER_H

#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsItem>
#include <QScrollBar>
#include <QUndoStack>

#include <QDragMoveEvent>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QDragLeaveEvent>
#include <QFileInfo>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QMimeData>
#include <QMessageBox>

#include <QtMath>

#include "objectmodel.h"
#include "checkpointmodel.h"
#include "palettemanager.h"
#include "metatileitem.h"
#include "objectitem.h"
#include "checkpointitem.h"

#include "undocommands.h"


#define SM_SCREEN_TILES_W_DEFAULT 16
#define SM_SCREEN_TILES_H_DEFAULT 12
#define SM_SCREENS_W_DEFAULT      8
#define SM_SCREENS_H_DEFAULT      4
#define SM_DEFAULT_ZOOM           1
#define SM_MAX_ZOOM               8
#define SM_CHECKPOINT_LIMIT       8
#define SM_OBJECT_LIMIT           64

#define SM_GLOBAL_PALETTE_MAX 8
#define SM_THICK_GRID_LINES   1.0
#define SM_THIN_GRID_LINES    0.5
#define SM_THICK_SEL_LINES    1.0

#define SM_FILE_OPEN_ERROR_TITLE   "Error opening stage file"
#define SM_FILE_OPEN_ERROR_BODY    "Could not open stage file. Please make sure you have the necessary permissions to access files in this location."
#define SM_INVALID_STAGE_TITLE     "Invalid Data"
#define SM_INVALID_STAGE_BODY      "Error reading stage data: Data is not a valid ASM tile data file."
#define SM_EOF_ERROR_TITLE         SM_INVALID_STAGE_TITLE
#define SM_EOF_ERROR_BODY          "Error reading stage data: Unexpected end of file."
#define SM_COUNT_ERROR_TITLE       SM_INVALID_STAGE_TITLE
#define SM_COUNT_ERROR_BODY        "Error reading stage data: Tile counts do not match length of data."
#define SM_OBJ_LIMIT_ERROR_TITLE   "Too many objects"
#define SM_OBJ_LIMIT_ERROR_BODY    "You have reached the object limit of %1."


struct ScreenProperties
{
	quint8 Song;
	bool ScrollBlockLeft;
	bool ScrollBlockRight;
};
typedef QVector<ScreenProperties> ScreenPropList;


class StageManager : public QGraphicsView
{
	Q_OBJECT
public:
	explicit StageManager(QWidget *parent = 0);
	~StageManager();

	qreal scale(){return this->iScale;}
	void changePalette(int);

	enum{ToolMetatiles,ToolObjects};

signals:
	void stageFileDropped(QString);
	void objectDropped(int);
	void sendTileData(QString);

	void requestSelectedMetatile(MetatileItem*);
	void requestTileUpdate(MetatileItem*);

	void setStageLabel(QString);
	void bankDividerChanged(quint16);

	void updateTileset(quint8,quint8);
	void updateAnimationFrame();
	void updateSpriteBank(quint16);

	void objectDropped(quint8);

	void sendScreenProperties(int,int,bool,bool);
	void sendSelectionProperties(int,bool,bool);

public slots:
	void cut();
	void copy();
	void paste();
	void undo();
	void redo();
	void clearUndoHistory();

	void setScale(qreal s){this->iScale=s;}
	void setSelectionMode(bool);
	void setScreenProperties(int,int,bool,bool);
	void setPropertySong(int i){this->vScreenProperties[this->iSelectedScreen].Song=i;}
	void setPropertyScrollBlockLeft(bool b){this->vScreenProperties[this->iSelectedScreen].ScrollBlockLeft=b;}
	void setPropertyScrollBlockRight(bool b){this->vScreenProperties[this->iSelectedScreen].ScrollBlockRight=b;}

	void clearAllMetatileData();
	void clearAllObjectData();
	void clearAllCheckpointData();

	void toggleShowScreenGrid(bool b){this->bShowScreenGrid=b;this->drawGridLines();}
	void toggleShowTileGrid(bool b){this->bShowTileGrid=b;this->drawGridLines();}

	void getNewTile(MetatileItem*, MetatileItem*);
	void getUpdatedTile(MetatileItem*);
	void getSelectedTileset(quint8);
	void getNewAnimationFrame(int);

	void setScreenTileset(quint8,quint8);
	void setToolMode(int);

	void openStageFile(QString);
	void openScreenPropertiesFile(QString);
	void openCheckpointsFile(QString);
	void openObjectsFile(QString);
	void importStageBinaryData(QVector<QByteArray>);
	void importScreenPropertiesBinaryData(QByteArray);
	void importCheckpointsBinaryData(QVector<QByteArray>);
	void importObjectsBinaryData(QVector<QByteArray>);
	QVector<QByteArray> createStageBinaryData();
	QString createStageASMData(QString);
	QString createScreenPropertiesASMData(QString);
	QString createCheckpointsASMData(QString);
	QString createObjectsASMData(QString);

	void getChangeTileCommand(ChangeStageTile*);

	void updateStageView();

protected:
	void resizeEvent(QResizeEvent*);
	void dragMoveEvent(QDragMoveEvent*e){e->accept();}
	void dragEnterEvent(QDragEnterEvent*e){e->acceptProposedAction();}
	void dropEvent(QDropEvent*);
	void dragLeaveEvent(QDragLeaveEvent*e){e->accept();}
	void mousePressEvent(QMouseEvent*);
	void mouseMoveEvent(QMouseEvent*);
	void mouseReleaseEvent(QMouseEvent*);
	void mouseDoubleClickEvent(QMouseEvent*);
	void wheelEvent(QWheelEvent*);
	void keyPressEvent(QKeyEvent*);

private:
	void populateBlankTiles();
	void drawGridLines();
	void drawSelectionBox();
	void drawTileSelectionBox();
	void replaceStageTiles();
	void replaceScreenTileset(QPointF);
	void replaceAllScreenTiles(QPointF);
	void getHoveredTile(QPointF);

	qreal iScale;
    quint8 iScreensW, iScreensH;
	quint8 iScreenTilesW, iScreenTilesH;
	quint8 iSelectedScreen;
	QPointF pSceneTranslation;
	QPointF pSelection;
	QRectF rTileSelection;

	bool bShowObjects, bTileSelectMode, bScreenSelectMode, bShowScreenGrid, bShowTileGrid;
	ScreenList vScreens;
	ScreenPropList vScreenProperties;
	QGraphicsScene *gsMetatiles;
	QGraphicsItemGroup *groupMetatiles;
	ObjectList lObjects;
	CheckpointList lCheckpoints;
	QList<QGraphicsLineItem*> lGrid;
	QGraphicsRectItem *griSelectionBox;
	QGraphicsRectItem *griTileSelector;
	quint8 iSelectedTileset;
	QList<QPointF> lItemMove;

	ObjectList lClipboard;
	QUndoStack *undoTiles;
	QUndoStack *undoObjects;
};

#endif // STAGEMANAGER_H
