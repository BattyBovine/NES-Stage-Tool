#ifndef STAGEMANAGER_H
#define STAGEMANAGER_H

#include <QWidget>
#include <QGraphicsView>
#include <QScrollBar>

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

#include "palettemanager.h"
#include "metatileitem.h"


#define roundToMult(x,f)      (f*qCeil(x/f))
#define getMultDiff(x,f)      (roundToMult(x,f)-x)

#define SM_SCREEN_TILES_W     16
#define SM_SCREEN_TILES_H     12
#define SM_SCREEN_WIDTH       MTI_TILEWIDTH*SM_SCREEN_TILES_W
#define SM_SCREEN_HEIGHT      MTI_TILEWIDTH*SM_SCREEN_TILES_H
#define SM_SCREENS_W          8
#define SM_SCREENS_H          4
#define SM_CANVAS_WIDTH       SM_SCREEN_WIDTH*SM_SCREENS_W
#define SM_CANVAS_HEIGHT      SM_SCREEN_HEIGHT*SM_SCREENS_H
#define SM_DEFAULT_ZOOM       1
#define SM_MAX_ZOOM           8

#define SM_GLOBAL_PALETTE_MAX 8
#define SM_THICK_GRID_LINES   1.0
#define SM_THIN_GRID_LINES    0.5

#define SM_FILE_OPEN_ERROR_TITLE   "Error opening metasprite file"
#define SM_FILE_OPEN_ERROR_BODY    "Could not open metasprite file. Please make sure you have the necessary permissions to access files in this location."
#define SM_INVALID_SPRITES_TITLE   "Invalid Data"
#define SM_INVALID_SPRITES_BODY    "Error reading metasprite data: Data is not a valid ASM sprite data file."
#define SM_EOF_ERROR_TITLE         "Invalid data"
#define SM_EOF_ERROR_BODY          "Error reading metasprite data: Unexpected end of file."
#define SM_COUNT_ERROR_TITLE       "Invalid data"
#define SM_COUNT_ERROR_BODY        "Error reading metasprite data: Sprite counts do not match length of data."
#define SM_SPRITELIMIT_ERROR_TITLE "Too many sprites"
#define SM_SPRITELIMIT_ERROR_BODY  "You already have 64 sprites on the stage. This is as much as the NES can handle. Any more added sprites will not be visible unless custom flickering is used. Are you sure you wish to continue?"


class StageManager : public QGraphicsView
{
	Q_OBJECT
public:
	explicit StageManager(QWidget *parent = 0);
	~StageManager();

	qreal scale(){return this->iScale;}
	void openMetatileFile(QString);
	void importMetatileBinaryData(QVector<QByteArray>, QByteArray);
	void clearAllMetatileData();

	void changePalette(int);

signals:
	void requestSelectedMetatile(MetatileItem*);
	void getMetatileUpdate(MetatileItem*);
	void getPaletteUpdate(MetatileItem*);
	void requestNewGlobalPalette();
	void requestPaletteUpdates(quint8);

	void setMetaspriteLabel(QString);
	void bankDividerChanged(quint16);

	void updateList(GraphicsItemList,GraphicsItemList);
	void updateAnimationFrame();
	void updateSpriteBank(quint16);

public slots:
	void setScale(qreal s){this->iScale=s;}

	void setNewGlobalPalette(PaletteVector);
	void deleteSelectedTiles();

	void updateScreen();

	void toggleShowScreenGrid(bool);
	void toggleShowTileGrid(bool);

	void getUpdatedTile(MetatileItem*,quint8);

	QVector<QByteArray> createMetaspriteBinaryData();
	QString createMetaspriteASMData(QString);

protected:
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
	void replaceScreenTile(QPointF);
	void replaceAllScreenTiles(QPointF);
	void placeUpdatedMetatile(int,int,MetatileItem*);

	qreal iScale;
	QPointF pMouseTranslation;
	QPointF pRightMousePos;
	QPointF pSceneTranslation;
	int iTileQueue;

	bool bShowScreenGrid, bShowTileGrid;
	QGraphicsScene *gsMetatiles;
	QGraphicsItemGroup *groupMetatiles;
	QList<QGraphicsLineItem*> lGrid;
	ScreenList vScreens;
};

#endif // STAGEMANAGER_H
