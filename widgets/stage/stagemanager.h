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


#define SM_SCREEN_TILES_W     16
#define SM_SCREEN_TILES_H     12
#define SM_SCREEN_PIX_WIDTH   MTI_TILEWIDTH*SM_SCREEN_TILES_W
#define SM_SCREEN_PIX_HEIGHT  MTI_TILEWIDTH*SM_SCREEN_TILES_H
#define SM_SCREENS_W          8
#define SM_SCREENS_H          4
#define SM_CANVAS_PIX_WIDTH   SM_SCREEN_PIX_WIDTH*SM_SCREENS_W
#define SM_CANVAS_PIX_HEIGHT  SM_SCREEN_PIX_HEIGHT*SM_SCREENS_H
#define SM_DEFAULT_ZOOM       1
#define SM_MAX_ZOOM           8

#define SM_GLOBAL_PALETTE_MAX 8
#define SM_THICK_GRID_LINES   1.0
#define SM_THIN_GRID_LINES    0.5

#define SM_FILE_OPEN_ERROR_TITLE   "Error opening metasprite file"
#define SM_FILE_OPEN_ERROR_BODY    "Could not open metasprite file. Please make sure you have the necessary permissions to access files in this location."
#define SM_INVALID_STAGE_TITLE     "Invalid Data"
#define SM_INVALID_STAGE_BODY      "Error reading metatile data: Data is not a valid ASM tile data file."
#define SM_EOF_ERROR_TITLE         "Invalid data"
#define SM_EOF_ERROR_BODY          "Error reading metasprite data: Unexpected end of file."
#define SM_COUNT_ERROR_TITLE       "Invalid data"
#define SM_COUNT_ERROR_BODY        "Error reading metatile data: Tile counts do not match length of data."


class StageManager : public QGraphicsView
{
	Q_OBJECT
public:
	explicit StageManager(QWidget *parent = 0);
	~StageManager();

	qreal scale(){return this->iScale;}

	void changePalette(int);

signals:
	void stageFileDropped(QString);

	void requestSelectedMetatile(MetatileItem*);
	void requestTileUpdate(MetatileItem*);

	void setStageLabel(QString);
	void bankDividerChanged(quint16);

	void updateAnimationFrame();
	void updateSpriteBank(quint16);

public slots:
	void setScale(qreal s){this->iScale=s;}

	void clearAllMetatileData();

	void toggleShowScreenGrid(bool);
	void toggleShowTileGrid(bool);

	void getNewTile(MetatileItem*, MetatileItem*);
	void getUpdatedTile(MetatileItem*);
	void getSelectedTileset(quint8);
	void getNewAnimationFrame(int);

	void openStageFile(QString);
	void importStageBinaryData(QVector<QByteArray>);
	QVector<QByteArray> createStageBinaryData();
	QString createStageASMData(QString);

	void updateStageView();

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
	void replaceStageTile(QPointF);
	void replaceScreenTileset(QPointF);
	void replaceAllScreenTiles(QPointF);

	qreal iScale;
	QPointF pMouseTranslation;
	QPointF pRightMousePos;
	QPointF pSceneTranslation;

	bool bShowScreenGrid, bShowTileGrid;
	QGraphicsScene *gsMetatiles;
	QGraphicsItemGroup *groupMetatiles;
	QList<QGraphicsLineItem*> lGrid;
	quint8 iSelectedTileset;
	ScreenList vScreens;
};

#endif // STAGEMANAGER_H
