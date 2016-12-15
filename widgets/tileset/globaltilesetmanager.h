#ifndef GLOBALTILESETMANAGER_H
#define GLOBALTILESETMANAGER_H

#include <QWidget>
#include <QGraphicsView>

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

#include "palettemanager.h"
#include "chrthread.h"

#define GTSM_SCALE       1
#define GTSM_TILEWIDTH   8

class GlobalTilesetManager : public QGraphicsView
{
	Q_OBJECT
public:
	explicit GlobalTilesetManager(QWidget *parent = 0);
	~GlobalTilesetManager();

signals:
	void tilesetChanged(QImage);
	void sendNewTile(QPointF,QImage,quint32,quint8);
	void sendNewMetatile(MetatileItem*);
	void chrBankChanged(quint16);
	void chrDataChanged(QImage);
	void checkTilesBank(quint16,quint16);
	void metatileUpdated(MetatileItem*);

public slots:
	void loadCHRData(QString);
	void loadCHRBank();
	void setNewSpriteColours(PaletteVector,quint8);
	void setSelectedBank(quint16);

	void getNewTile(QPointF);
	void getNewSubtile(quint8,MetatileItem*);
	void updateMetatile(MetatileItem*);
	void getNewCHRData(QImage);
	void getCHRError(QString,QString);
	void getBankSize(int);
	void getBankSelections(int,int,int,int,int,int,int,int);

	void reloadCurrentTileset();

protected:
	void resizeEvent(QResizeEvent*);
	void dragMoveEvent(QDragMoveEvent *e){e->accept();}
	void dragEnterEvent(QDragEnterEvent *e){e->acceptProposedAction();}
	void dropEvent(QDropEvent*);
	void dragLeaveEvent(QDragLeaveEvent *e){e->accept();}
	void mousePressEvent(QMouseEvent*);
//	void wheelEvent(QWheelEvent*);

private:
	bool drawBankDivider();
	bool drawSelectionBox();
	QImage createNewTile(quint32);

	QGraphicsScene *gsTileset;
	QString sCurrentTilesetFile;
	QFileSystemWatcher fswCHR;
	CHRThread *threadCHR;
	QImage imgTileset;
	QImage imgSelectedBank;
	PaletteVector pvCurrentColours;
	QGraphicsPixmapItem *gpiTileset;
	quint32 iSelectedTile;
	quint8 iPalette;
	bool bTallSprite;
	quint8 iBankDivider;
	int iBankList[8];
	quint16 iSelectedBank;

	QGraphicsRectItem *griSelection[2];
	QPointF pSelection;
};

#endif // GLOBALTILESETMANAGER_H
