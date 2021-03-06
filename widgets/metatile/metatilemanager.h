#ifndef METATILEMANAGER_H
#define METATILEMANAGER_H

#include <QWidget>
#include <QGraphicsView>

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

#include "globaltilesetmanager.h"
#include "tilesetmanager.h"
#include "palettemanager.h"
#include "metatileitem.h"

#include "undocommands.h"


#define MTM_CANVAS_SIZE      256
#define MTM_METATILES_W      16
#define MTM_METATILES_H      16
#define MTM_DEFAULT_ZOOM     1
#define MTM_MAX_ZOOM         2
#define MTM_THICK_GRID_LINES 1.0
#define MTM_THIN_GRID_LINES  0.5

#define MTM_FILE_OPEN_ERROR_TITLE   "Error opening metatile file"
#define MTM_FILE_OPEN_ERROR_BODY    "Could not open metatile file. Please make sure you have the necessary permissions to access files in this location."
#define MTM_INVALID_SPRITES_TITLE   "Invalid Data"
#define MTM_INVALID_SPRITES_BODY    "Error reading metatile data: Data is not a valid ASM sprite data file."
#define MTM_EOF_ERROR_TITLE         "Invalid data"
#define MTM_EOF_ERROR_BODY          "Error reading metatile data: Unexpected end of file."
#define MTM_COUNT_ERROR_TITLE       "Invalid data"
#define MTM_COUNT_ERROR_BODY        "Error reading metatile data: Subtile counts do not match length of data."


class MetatileManager : public QGraphicsView
{
	Q_OBJECT
public:
	explicit MetatileManager(QWidget *parent = 0);
	~MetatileManager();

	qreal scale(){return this->iScale;}

	void selectAllSprites();
	void deselectAllSprites();
	void copySpritesToClipboard(bool);
	void pasteSpritesFromClipboard();
	void moveSelectedX(bool,bool);
	void moveSelectedY(bool,bool);

signals:
	void requestNewTile(QPointF);
	void requestNewSubtile(quint8,MetatileItem*);
	void getMetatileUpdate(MetatileItem*);
	void getPaletteUpdate(MetatileItem*);
	void requestPaletteUpdates(quint8);
	void metatileUpdated(MetatileItem*,MetatileItem*);
	void metatileUpdated(MetatileItem*);
	void sendMetatileToSelector(MetatileItem*);
	void sendSelectedTileset(quint8);

	void setMetaspriteLabel(QString);
	void bankDividerChanged(quint16);
	void tilesetChangedDelta(int);
    void sendMetatileProperties(int,int,bool,bool);
    void sendSelectionProperties(int,bool,bool);

#ifdef METASPRITETILEITEM_H
	void updateList(GraphicsItemList,GraphicsItemList);
#endif

	void changeStageTile(ChangeStageTile*);

public slots:
	void undo();
	void redo();
	void clearUndoHistory();

	void setScale(qreal s){this->iScale=s;}
	void setSelectionMode(bool);
	void clearAllMetatileData();

	void setNewTileColours(PaletteVector,quint8,bool);
	void getEditorMetatile(MetatileItem*);
	void getSelectedStageTile(MetatileItem*);
	void updateStageMetatile(MetatileItem*);
	void getNewAnimationFrame(int);

	void toggleShowGrid8(bool b){this->bShowGrid8=b;this->drawGridLines();}
	void toggleShowGrid16(bool b){this->bShowGrid16=b;this->drawGridLines();}
	void setGlobalTileset(int);
    void setSelectedSubtile(int);

    void setMetatileProperties(int,int,bool,bool);
    void setPropertyCollision(int i){this->mtlMetatiles[this->iSelectedTile]->setCollision(i);}
    void setPropertyDestructible(bool b){this->mtlMetatiles[this->iSelectedTile]->setDestructible(b);}
    void setPropertyDeadly(bool b){this->mtlMetatiles[this->iSelectedTile]->setDeadly(b);}

	void openMetatileFile(QString);
	void importMetatileBinaryData(QVector<QByteArray>);
	QVector<QByteArray> createMetatileBinaryData();
	QString createMetatileASMData(QString);

	void updateScreen();

protected:
    void resizeEvent(QResizeEvent*);
	void dragMoveEvent(QDragMoveEvent*e){e->accept();}
	void dragEnterEvent(QDragEnterEvent*e){e->acceptProposedAction();}
	void dropEvent(QDropEvent*);
	void dragLeaveEvent(QDragLeaveEvent*e){e->accept();}
	void mousePressEvent(QMouseEvent*);
	void mouseDoubleClickEvent(QMouseEvent*);
	void mouseMoveEvent(QMouseEvent*);
	void wheelEvent(QWheelEvent*);
//	void keyPressEvent(QKeyEvent*);

private:
	void populateBlankTiles();
	void addNewSubtile(QPointF);
	void applySelectedPalette(QPointF);
	bool drawSelectionBox();
	void drawGridLines();
	MetatileList createFrame(quint8, qreal s=0);

	qreal iScale;

	bool bSelectionMode;
	QPointF pSelection;
	QPoint pMouseTranslation;
	QGraphicsRectItem *griSelection[2];

	bool bShowGrid8, bShowGrid16;
	quint16 iBankDivider;
	quint8 iGlobalTileset;
	quint8 iSelectedTile;
	quint8 iSelectedSubtile;
    quint8 iSelectedPalette;

	QGraphicsScene *gsMetatiles;
	QGraphicsItemGroup *groupMetatiles;
	QList<QGraphicsLineItem*> lGrid;
	MetatileList mtlMetatiles;

	QUndoStack *undoMetatiles;
};

#endif // METATILEMANAGER_H
