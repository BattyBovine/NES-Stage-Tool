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


#define NESMT_WIKI_URL				"https://github.com/BattyBovine/NES-Stage-Tool/wiki"

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

signals:
	void chrFileOpened(QString);
	void metaspriteFileOpened(QString);
	void sendBankUpdates(int,int,int,int,int,int,int,int);

private slots:
	void newProject();
	void openProject();
	void saveProject();
	void saveProjectToFolder(QString);
	void showWikiPage();

	void initTilesetManagement(QImage img);

	void toggleAnimation(bool);

	void saveShowScreenGrid();
	void saveShowTileGrid();
	void saveShowGrid16();
	void saveShowGrid8();
	void savePaletteSwatch();
	void changeBankSize();
	void restoreSettings();

	void sendBankUpdates();

	void openStage();
	void saveASMStage(QString path="");
	void saveBinaryStage(QString path="");

	void openCHR();
	void openPalette();
	void savePalette(QString path="");

	void saveASMAnimation(QString path="");

	void setNewPaletteFile(QString);

protected:
	void keyPressEvent(QKeyEvent*);

private:
	Ui::NESStageTool *ui;

	QSettings sSettings;
};

#endif // NESSTAGETOOL_H
