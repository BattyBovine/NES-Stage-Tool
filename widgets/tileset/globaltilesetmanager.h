#ifndef GLOBALTILESETMANAGER_H
#define GLOBALTILESETMANAGER_H

#include <QWidget>
#include <QGraphicsView>
#include <QTimer>

#include <QDragMoveEvent>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QDragLeaveEvent>
#include <QMouseEvent>

#include <QFile>
#include <QFileSystemWatcher>
#include <QMimeData>
#include <QMessageBox>
#include <QPixmap>
#include <QGraphicsPixmapItem>

#include <QtMath>

#include "metatileitem.h"
#include "palettemanager.h"
#include "chrthread.h"


#define GTSM_FILE_OPEN_ERROR_TITLE    "Error opening tileset file"
#define GTSM_FILE_OPEN_ERROR_BODY     "Could not open tileset file. Please make sure you have the necessary permissions to access files in this location."

#define GTSM_SCALE         1
#define GTSM_TILEWIDTH	   8
#define GTSM_TILESET_COUNT 8
#define GTSM_BANKS_COUNT   8
#define GTSM_ANIM_FRAMES   4
#define GTSM_ANIM_DELAY    67


class GlobalTilesetManager : public QGraphicsView
{
	Q_OBJECT
public:
	explicit GlobalTilesetManager(QWidget *parent = 0);
	~GlobalTilesetManager();

	bool openTilesetFile(QString);
	bool importTilesetBinaryData(QVector<QByteArray>);
	QString createTilesetASMData(QString);
	QByteArray createTilesetBinaryData();

signals:
	void tilesetChanged(QImage);
	void sendNewTile(QPointF,QImage,quint32,quint8);
	void sendNewMetatile(MetatileItem*);
	void chrBankChanged(quint16);
	void chrDataChanged(QImage);
	void checkTilesBank(quint16,quint16);
	void metatileUpdated(MetatileItem*);

	void banksChanged(int,int,int,int,int,int,int,int);
	void setGlobalTilesetSelectedIndex(int);
	void newAnimationFrame(int);

public slots:
	void loadCHRData(QString);
	void loadCHRBank(int set);
	void loadCHRBank();

	void getNewCHRData(QImage);
	void getCHRError(QString,QString);
	void getBankSize(int);
	void getGlobalTileset(int);
	void getGlobalTilesetDelta(int);
	void getGlobalPalette(PaletteVector,quint8);
	void getBankSelections(int,int,int,int,int,int,int,int);
	void enableAnimation(bool);
	void getAnimBank(int i){this->iAnimFrame=i;}
	void switchToNextAnimBank();
	void clearAllTilesetData();

	void reloadCurrentTileset();

protected:
	void resizeEvent(QResizeEvent*);
	void dragMoveEvent(QDragMoveEvent *e){e->accept();}
	void dragEnterEvent(QDragEnterEvent *e){e->acceptProposedAction();}
	void dropEvent(QDropEvent*);
	void dragLeaveEvent(QDragLeaveEvent *e){e->accept();}
	void mousePressEvent(QMouseEvent*);
	void wheelEvent(QWheelEvent*);

private:
	bool drawBankDivider();
	bool drawSelectionBox();

	QGraphicsScene *gsTileset;
	QString sCurrentTilesetFile;
	QFileSystemWatcher fswCHR;
	CHRThread *threadCHR;

	QImage imgTileset;
	QGraphicsPixmapItem *gpiTileset;
	QPixmap pixLocalCache[GTSM_ANIM_FRAMES];
	quint32 iSelectedTile;
	quint8 iSelectedPalette;
	quint8 iBankDivider;
	quint8 iGlobalTileset;
	quint16 iBankLists[GTSM_TILESET_COUNT][GTSM_BANKS_COUNT];
	PaletteVector vPaletteLists[GTSM_TILESET_COUNT][PM_SUBPALETTES_MAX];

	QTimer tAnimation;
	bool bAnimation;
	quint8 iAnimFrame;

	QGraphicsRectItem *griSelection[2];
	QPointF pSelection;
};

#endif // GLOBALTILESETMANAGER_H
