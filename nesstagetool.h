#ifndef NESSTAGETOOL_H
#define NESSTAGETOOL_H

#include <QMainWindow>
#include <QSpinBox>
#include <QGraphicsRectItem>
#include <QMessageBox>
#include <QFileDialog>
#include <QDesktopServices>
#include <QDirIterator>
#include <QTextStream>
#include <QRegularExpression>
#include <QSettings>
#include <QEvent>

#include "objectmodel.h"
#include "objectdelegate.h"
#include "checkpointmodel.h"

#define NESST_WIKI_URL				"https://github.com/BattyBovine/NES-Stage-Tool/wiki"

#define FILE_OPEN_ERROR_TITLE       "Error opening file"
#define FILE_OPEN_ERROR_BODY        "Could not open file. Please make sure you have the necessary permissions to access files in this location."
#define FILE_SAVE_ERROR_TITLE       "Error saving file"
#define FILE_SAVE_ERROR_BODY        "Could not save file. Please make sure you have the necessary permissions to save files to this location."


namespace Ui {
class NESStageTool;
}

class NESStageTool : public QMainWindow
{
	Q_OBJECT

public:
	explicit NESStageTool(QWidget *parent = 0);
	~NESStageTool();

	enum{EditorStage,EditorMetatiles,EditorProperties,EditorTilesets};

signals:
	void chrFileOpened(QString);
	void metaspriteFileOpened(QString);
	void banksUpdated(int,int,int,int,int,int,int,int);

private slots:
	void newProject();
	void openProject();
	void saveProject();
	void saveProjectToFolder(QString);
	void showWikiPage();
    void showAboutDialogue();

	void initTilesetManagement(QImage img);

	void toggleAnimation(bool);

	void saveShowScreenGrid();
	void saveShowTileGrid();
	void saveShowGrid16();
	void saveShowGrid8();
	void savePaletteSwatch();
	void changeBankSize();
	void restoreSettings();

	void storeCollisionName(QString);
	void storeOpenedChrFile(QString);
	void retrieveCollisionName(int);

	void sendBankUpdates();
	void getBankUpdates(int,int,int,int,int,int,int,int);
	void getMetatileProperties(int,bool,bool);
	void getScreenProperties(int,bool,bool);

	void openStage(QString path="");
	void saveASMStage(QString path="");
	void saveBinaryStage(QString path="");

	void openCHR();
	void openPalette();
	void savePalette(QString path="");

	void clearObjectList();
	void clearCollisionTypes();

	void saveASMAnimation(QString path="");

	void setStageLabelPrefix(QString);
	void setNewPaletteFile(QString);

	void cut();
	void copy();
	void paste();
	void undo();
	void redo();

private:
	Ui::NESStageTool *ui;

	ObjectModel *listObjects;
	CheckpointModel *listCheckpoints;
	QSettings sSettings;
};

#endif // NESSTAGETOOL_H
