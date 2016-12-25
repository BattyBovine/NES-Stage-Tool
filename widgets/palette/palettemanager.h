#ifndef PALETTEMANAGER_H
#define PALETTEMANAGER_H

#include <QGraphicsView>
#include <QMouseEvent>

#include <QMimeData>
#include <QMessageBox>

#include <QtMath>

//#include "metaspritetileitem.h"
#include "metatileitem.h"
#include "spritepaletteview.h"


#define PM_FILE_OPEN_ERROR_TITLE    "Error opening palette file"
#define PM_FILE_OPEN_ERROR_BODY     "Could not open palette file. Please make sure you have the necessary permissions to access files in this location."

#define PM_GLOBAL_PALETTE_COUNT 8
#define PM_SWATCH_SIZE          16
#define PM_PALETTE_COLOURS_MAX  4
#define PM_SUBPALETTES_MAX      4

typedef QVector<QRgb> PaletteVector;


class PaletteManager : public QGraphicsView
{
	Q_OBJECT
public:
	explicit PaletteManager(QWidget *parent = 0);
	~PaletteManager();

	bool drawFullPaletteColours(QString);

	bool drawSelectionBox(QGraphicsScene*, quint8);
	bool openPaletteFile(QString);
	bool importPaletteBinaryData(QVector<QByteArray>);
	QString createPaletteASMData(QString,int tileset=0);
	QByteArray createPaletteBinaryData(int tileset=0);

signals:
	void paletteSelected(quint8);
	void newSpritePalette0(QGraphicsScene*);
	void newSpritePalette1(QGraphicsScene*);
	void newSpritePalette2(QGraphicsScene*);
	void newSpritePalette3(QGraphicsScene*);
	void newSpriteColours(PaletteVector,quint8,bool);
	void newGlobalPalette(PaletteVector c);
	void metatileUpdated(MetatileItem*);

public slots:
	void setPaletteFile(QString);
	void setSpritePaletteScene(QObject*);
	void clearAllPaletteData();

	void spritePaletteSelected(QString, quint8);
	QVector<QRgb> createPaletteColours();
	void generateNewSpritePalettes(bool changeselected = false);
	void setNewGlobalPalette(int);
	void generateNewGlobalPalette();
#ifdef METASPRITETILEITEM_H
	void setNewSpritePalette(MetaspriteTileItem*);
#endif
	void setNewMetatilePalette(MetatileItem*);
	void sendRequestedPaletteUpdate(quint8);

protected:
	void dragMoveEvent(QDragMoveEvent*e){e->accept();}
	void dragEnterEvent(QDragEnterEvent*e){e->acceptProposedAction();}
	void dropEvent(QDropEvent*);
	void dragLeaveEvent(QDragLeaveEvent*e){e->accept();}
	void mousePressEvent(QMouseEvent*);


private:
	QString sPaletteFile;
	QVector<QColor> vPaletteColours;
	QGraphicsScene *gsFullPaletteScene;
	quint8 iFullPaletteIndex;
	QGraphicsRectItem *griSelectionBox[2];

	QGraphicsScene *gsSpritePaletteScene[4];
	quint8 iGlobalPalette;
	quint8 iSpritePaletteIndices[PM_GLOBAL_PALETTE_COUNT][PM_SUBPALETTES_MAX][PM_PALETTE_COLOURS_MAX];
	quint8 iSpritePaletteSelected;
	quint8 iSpritePaletteSelectedIndex;
};

#endif // PALETTEMANAGER_H
