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
//#include "metatileitem.h"
#include "metatileitem.h"


#define roundToMult(x,f)     (f*qCeil(x/f))
#define getMultDiff(x,f)     (roundToMult(x,f)-x)

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
	void clearAllMetatileData();

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
	void metatileUpdated(MetatileItem*);
	void metatilePaletteUpdated(MetatileItem*);
	void sendMetatileToSelector(MetatileItem*);
	void sendSelectedMetatile(MetatileItem*);
	void stageMetatileReady(MetatileItem*,MetatileItem*);
	void selectedStageTileReady(MetatileItem*,quint8);

	void setMetaspriteLabel(QString);
	void bankDividerChanged(quint16);
	void tilesetChangedDelta(int);

#ifdef METASPRITETILEITEM_H
	void updateList(GraphicsItemList,GraphicsItemList);
#endif

public slots:
	void setScale(qreal s){this->iScale=s;}
	void setSelectionMode(bool);

	void setNewTileColours(PaletteVector,quint8,bool);
	void getEditorMetatile(MetatileItem*);
	void getSelectedStageTile(MetatileItem*);
	void getNewAnimationFrame(int);

	void toggleShowGrid8(bool);
	void toggleShowGrid16(bool);
	void setGlobalTileset(int);
	void setSelectedSubtile(int);

	void openMetatileFile(QString);
	void importMetatileBinaryData(QVector<QByteArray>);
	QVector<QByteArray> createMetatileBinaryData();
	QString createMetatileASMData(QString);

	void updateScreen();

protected:
	void resizeEvent(QResizeEvent*);//{this->fitInView(this->gsMetatiles->itemsBoundingRect(),Qt::KeepAspectRatio);}
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
//	void generatePaletteTileCache();
//	void generateTileCache(int,int,int,int);

	qreal iScale;
	int iMouseTranslateY;

	bool bSelectionMode;
	QPointF pSelection;
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
};

#endif // METATILEMANAGER_H
