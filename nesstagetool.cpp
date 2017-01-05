#include "nesstagetool.h"
#include "ui_nesstagetool.h"

NESStageTool::NESStageTool(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::NESStageTool)
{
	ui->setupUi(this);

	QStringList palettes;
	QDirIterator it(":", QDirIterator::Subdirectories);
	while (it.hasNext()) {
		QString next = it.next();
		if(next.endsWith(".pal",Qt::CaseInsensitive))
			palettes.append(next.replace(":/pal/","").replace(".pal",""));
	}
	palettes.sort(Qt::CaseInsensitive);
	foreach(QString p, palettes) {
		ui->comboPalettes->addItem(p);
	}

	this->sSettings.setDefaultFormat(QSettings::NativeFormat);
	this->restoreSettings();
	connect(ui->chkShowScreenGrid,SIGNAL(toggled(bool)),this,SLOT(saveShowScreenGrid()));
	connect(ui->chkShowTileGrid,SIGNAL(toggled(bool)),this,SLOT(saveShowTileGrid()));
	connect(ui->chkShowGrid16,SIGNAL(toggled(bool)),this,SLOT(saveShowGrid16()));
	connect(ui->chkShowGrid8,SIGNAL(toggled(bool)),this,SLOT(saveShowGrid8()));

	connect(ui->comboPalettes,SIGNAL(currentIndexChanged(int)),this,SLOT(savePaletteSwatch()));
	connect(ui->comboBankSize,SIGNAL(currentIndexChanged(int)),this,SLOT(changeBankSize()));

	connect(ui->gvGlobalTileset,SIGNAL(chrDataChanged(QImage)),this,SLOT(initTilesetManagement(QImage)));

    ui->gvGlobalTileset->loadCHRData(":/chr/blank.chr");
    ui->gvMetatileSelector->setSelectionMode(true);
    ui->gvMetatileSelectorProperties->setSelectionMode(true);

	this->changeBankSize();
	this->sendBankUpdates();
}

NESStageTool::~NESStageTool()
{
	delete ui;
}



void NESStageTool::keyPressEvent(QKeyEvent *e)
{
//	if(ui->tabWidget->currentWidget()==ui->tabStage) {
//		if(e->modifiers()&Qt::ControlModifier) {
//			switch(e->key()) {
//			case Qt::Key_A:
//				(e->modifiers()&Qt::ShiftModifier)?ui->gvStage->deselectAllSprites():ui->gvStage->selectAllSprites();
//				break;
//			case Qt::Key_X:
//			case Qt::Key_C:
//				ui->gvStage->copySpritesToClipboard((e->key()==Qt::Key_X));
//				break;
//			case Qt::Key_V:
//				ui->gvStage->pasteSpritesFromClipboard();
//				break;
//			}
//		} else {
//			switch(e->key()) {
//			case Qt::Key_Left:
//			case Qt::Key_Right:
//				ui->gvStage->moveSelectedX((e->key()==Qt::Key_Right),(e->modifiers()&Qt::ShiftModifier));
//				break;
//			case Qt::Key_Up:
//			case Qt::Key_Down:
//				ui->gvStage->moveSelectedX((e->key()==Qt::Key_Down),(e->modifiers()&Qt::ShiftModifier));
//				break;
//			case Qt::Key_PageUp:
//				ui->gvStage->moveSelectedUp();
//				break;
//			case Qt::Key_PageDown:
//				ui->gvStage->moveSelectedDown();
//				break;
//			case Qt::Key_Delete:
//				ui->gvStage->deleteSelectedTiles();
//				break;
//			case Qt::Key_1:
//			case Qt::Key_2:
//			case Qt::Key_3:
//			case Qt::Key_4:
//				ui->gvStage->changePalette(e->key()-Qt::Key_1);
//			}
//		}
//		return;
//	} else if(ui->tabWidget->currentWidget()==ui->tabMetatiles) {
//		if(e->modifiers()&Qt::ControlModifier) {
//			switch(e->key()) {
//			default:
//				break;
//			}
//		} else {
//			switch(e->key()) {
//			case Qt::Key_Comma:
//			case Qt::Key_Period:
//			case Qt::Key_BracketLeft:
//			case Qt::Key_BracketRight:
//				break;
//            case Qt::Key_Left:
//                ui->spinDelay->setValue(ui->spinDelay->value()-1);
//                break;
//            case Qt::Key_Right:
//                ui->spinDelay->setValue(ui->spinDelay->value()+1);
//                break;
//            case Qt::Key_PageUp:
//                ui->spinDelay->setValue(ui->spinDelay->value()+10);
//                break;
//            case Qt::Key_PageDown:
//                ui->spinDelay->setValue(ui->spinDelay->value()-10);
//                break;
//			}
//		}
//	}

	QMainWindow::keyPressEvent(e);
}



void NESStageTool::newProject()
{
	int retval = QMessageBox::warning(this,"Clear current data?",
									  "All unsaved data will be lost. Are you sure you wish to create a new project?",
									  QMessageBox::Yes | QMessageBox::No,
									  QMessageBox::No);
	switch(retval) {
	case QMessageBox::Yes:
		ui->gvStage->clearAllMetatileData();
		ui->gvMetatileEditor->clearAllMetatileData();
        ui->gvMetatileSelectorProperties->clearAllMetatileData();
		ui->gvPaletteManager->clearAllPaletteData();
		ui->gvGlobalTileset->clearAllTilesetData();
        ui->comboCollision->setCurrentIndex(0);
        ui->chkDestructible->setChecked(false);
        ui->chkDeadly->setChecked(false);
		ui->spinBank0->setValue(0);
		ui->spinBank1->setValue(0);
		ui->spinBank2->setValue(0);
		ui->spinBank3->setValue(0);
		ui->spinBank4->setValue(0);
		ui->spinBank5->setValue(0);
		ui->spinBank6->setValue(0);
		ui->spinBank7->setValue(0);
		ui->lineASMLabel->setText("");
		break;
	}
}

void NESStageTool::openProject()
{
    QString foldername = QFileDialog::getExistingDirectory(this, ui->actionOpenProject->text().replace("&",""), "");
	if(!foldername.isEmpty())   QMessageBox::information(this,"Folder selected",foldername,QMessageBox::NoButton);
}

void NESStageTool::saveProject()
{
	QFileDialog saveproj(this);
	saveproj.setFileMode(QFileDialog::AnyFile);
	saveproj.setOptions(QFileDialog::ShowDirsOnly);
	saveproj.setViewMode(QFileDialog::Detail);
	connect(&saveproj,SIGNAL(fileSelected(QString)),this,SLOT(saveProjectToFolder(QString)));
	saveproj.exec();
}
void NESStageTool::saveProjectToFolder(QString f)
{
	QDir folder(f);
	if(!folder.exists()) {
		folder.mkdir(f);
	}
}

void NESStageTool::showWikiPage()
{
    QDesktopServices::openUrl(QUrl(NESST_WIKI_URL));
}

void NESStageTool::showAboutDialogue()
{
    QString abouttext = qApp->applicationName()+QString(" v")+qApp->applicationVersion()+QString("\n\n")+
            tr("©2016 BattyBovine. All rights reserved.");
    QMessageBox::question(this,qApp->applicationName(),
                          abouttext,
                          QMessageBox::Ok);
}



void NESStageTool::initTilesetManagement(QImage img)
{
	int bankdiv = qPow(2,ui->comboBankSize->currentIndex());
	int max = bankdiv*qFloor(img.height()/img.width())-1;
	ui->spinBank0->setMaximum(max);
	ui->spinBank1->setMaximum(max);
	ui->spinBank2->setMaximum(max);
	ui->spinBank3->setMaximum(max);
	ui->spinBank4->setMaximum(max);
	ui->spinBank5->setMaximum(max);
	ui->spinBank6->setMaximum(max);
	ui->spinBank7->setMaximum(max);
}



void NESStageTool::toggleAnimation(bool)
{
	ui->gvGlobalTileset->enableAnimation(ui->chkAnimation->isChecked());
	this->sSettings.setValue("ShowAnimation", ui->chkAnimation->isChecked());
}



void NESStageTool::saveShowScreenGrid()
{
	this->sSettings.setValue("ShowScreenGrid", ui->chkShowScreenGrid->isChecked());
}
void NESStageTool::saveShowTileGrid()
{
	this->sSettings.setValue("ShowTileGrid", ui->chkShowTileGrid->isChecked());
}
void NESStageTool::saveShowGrid16()
{
	this->sSettings.setValue("ShowGrid16", ui->chkShowGrid16->isChecked());
}
void NESStageTool::saveShowGrid8()
{
	this->sSettings.setValue("ShowGrid8", ui->chkShowGrid8->isChecked());
}
void NESStageTool::changeBankSize()
{
	ui->labelBank1->setEnabled(false);ui->spinBank1->setEnabled(false);
	ui->labelBank2->setEnabled(false);ui->spinBank2->setEnabled(false);
	ui->labelBank3->setEnabled(false);ui->spinBank3->setEnabled(false);
	ui->labelBank4->setEnabled(false);ui->spinBank4->setEnabled(false);
	ui->labelBank5->setEnabled(false);ui->spinBank5->setEnabled(false);
	ui->labelBank6->setEnabled(false);ui->spinBank6->setEnabled(false);
	ui->labelBank7->setEnabled(false);ui->spinBank7->setEnabled(false);
	switch(ui->comboBankSize->currentIndex()) {
	case 3:
		ui->labelBank7->setEnabled(true);ui->spinBank7->setEnabled(true);
		ui->labelBank6->setEnabled(true);ui->spinBank6->setEnabled(true);
		ui->labelBank5->setEnabled(true);ui->spinBank5->setEnabled(true);
		ui->labelBank4->setEnabled(true);ui->spinBank4->setEnabled(true);
	case 2:
		ui->labelBank3->setEnabled(true);ui->spinBank3->setEnabled(true);
		ui->labelBank2->setEnabled(true);ui->spinBank2->setEnabled(true);
	case 1:
		ui->labelBank1->setEnabled(true);ui->spinBank1->setEnabled(true);
	default:
		ui->labelBank0->setEnabled(true);ui->spinBank0->setEnabled(true);
		break;
	}

	this->sSettings.setValue("BankSize", ui->comboBankSize->currentIndex());
}
void NESStageTool::savePaletteSwatch()
{
	this->sSettings.setValue("PaletteSwatch", ui->comboPalettes->currentIndex());
}


void NESStageTool::restoreSettings()
{
	ui->chkShowScreenGrid->setChecked(this->sSettings.value("ShowScreenGrid",true).toBool());
	ui->chkShowTileGrid->setChecked(this->sSettings.value("ShowTileGrid",true).toBool());
	ui->chkShowGrid16->setChecked(this->sSettings.value("ShowGrid16",true).toBool());
	ui->chkShowGrid8->setChecked(this->sSettings.value("ShowGrid8",true).toBool());
	ui->comboBankSize->setCurrentIndex(this->sSettings.value("BankSize",0).toInt());
	ui->comboPalettes->setCurrentIndex(this->sSettings.value("PaletteSwatch",0).toInt());
	ui->chkAnimation->setChecked(this->sSettings.value("ShowAnimation",false).toBool());
}



void NESStageTool::sendBankUpdates() {
	emit(banksUpdated(ui->spinBank0->value(),ui->spinBank1->value(),
					  ui->spinBank2->value(),ui->spinBank3->value(),
					  ui->spinBank4->value(),ui->spinBank5->value(),
					  ui->spinBank6->value(),ui->spinBank7->value() ));
}

void NESStageTool::getBankUpdates(int b0, int b1, int b2, int b3, int b4, int b5, int b6, int b7)
{
	ui->spinBank0->setValue(b0);
	ui->spinBank1->setValue(b1);
	ui->spinBank2->setValue(b2);
	ui->spinBank3->setValue(b3);
	ui->spinBank4->setValue(b4);
	ui->spinBank5->setValue(b5);
	ui->spinBank6->setValue(b6);
	ui->spinBank7->setValue(b7);
}

void NESStageTool::getMetatileProperties(int collision, bool destructible, bool deadly)
{
    ui->comboCollision->setCurrentIndex(collision);
    ui->chkDestructible->setChecked(destructible);
    ui->chkDeadly->setChecked(deadly);
}



void NESStageTool::openStage(QString path)
{
    QString filename = path.isEmpty()?QFileDialog::getOpenFileName(this, ui->actionOpenStageFile->text().replace("&",""), "", tr("All files (*.*)")):path;
	if(filename.isEmpty())  return;
	ui->gvPaletteManager->openPaletteFile(filename);
	ui->gvGlobalTileset->openTilesetFile(filename);
	ui->gvMetatileEditor->openMetatileFile(filename);
	ui->gvStage->openStageFile(filename);
}

void NESStageTool::saveASMStage(QString path)
{
	QString filename;
	if(path.isEmpty()) {
        filename = QFileDialog::getSaveFileName(this, ui->actionSaveStageFileASM->text().replace("&",""), "", tr("All files (*.*)"));
		if(filename.isEmpty())  return;
	} else {
		filename = path;
	}

	QFile file(filename);
	if(!file.open(QIODevice::WriteOnly)) {
		QMessageBox::warning(this,tr(FILE_SAVE_ERROR_TITLE),tr(FILE_SAVE_ERROR_BODY),QMessageBox::NoButton);
		return;
	}

	file.write(ui->gvStage->createStageASMData(ui->lineASMLabel->text()).toLocal8Bit());
	file.write("\n");
    file.write(ui->gvMetatileSelectorProperties->createMetatileASMData(ui->lineASMLabel->text()).toLocal8Bit());
	file.write("\n");
	for(int tileset=0; tileset<GTSM_TILESET_COUNT; tileset++)
		file.write(ui->gvPaletteManager->createPaletteASMData(ui->lineASMLabel->text(),tileset).toUtf8());
	file.write("\n");
	file.write(ui->gvGlobalTileset->createTilesetASMData(ui->lineASMLabel->text()).toUtf8());

	file.close();
}

void NESStageTool::saveBinaryStage(QString path)
{
	QString filename;
	if(path.isEmpty()) {
        filename = QFileDialog::getSaveFileName(this, ui->actionSaveStageFileBinary->text().replace("&",""), "", tr("All files (*.*)"));
		if(filename.isEmpty())  return;
	} else {
		filename = path;
	}

	QFile file(filename);
	if(!file.open(QIODevice::WriteOnly)) {
		QMessageBox::warning(this,tr(FILE_SAVE_ERROR_TITLE),tr(FILE_SAVE_ERROR_BODY),QMessageBox::NoButton);
		return;
	}

	QVector<QByteArray> bindata = ui->gvStage->createStageBinaryData();
	foreach(QByteArray bin, bindata)
		if(!bin.isEmpty())  file.write(bin);
	file.close();
}



void NESStageTool::openCHR()
{
    QString filename = QFileDialog::getOpenFileName(this, ui->actionOpenCHR->text().replace("&",""), "", tr("CHR-ROM data (*.chr);;All files (*.*)"));
	if(filename.isEmpty()) return;
	ui->gvGlobalTileset->loadCHRData(filename);
}

void NESStageTool::openPalette()
{
    QString filename = QFileDialog::getOpenFileName(this, ui->actionOpenPalette->text().replace("&",""), "", tr("Palette data (*.pal);;All files (*.*)"));
	if(filename.isEmpty())  return;
	ui->gvPaletteManager->openPaletteFile(filename);
}

void NESStageTool::savePalette(QString path)
{
	QString filename;
	if(path.isEmpty()) {
        filename = QFileDialog::getSaveFileName(this, ui->actionSavePalette->text().replace("&",""), "", tr("Palette data (*.pal);;All files (*.*)"));
		if(filename.isEmpty())  return;
	} else {
		filename = path;
	}

	QFile file(filename);
	if(!file.open(QIODevice::WriteOnly)) {
		QMessageBox::warning(this,tr(FILE_SAVE_ERROR_TITLE),tr(FILE_SAVE_ERROR_BODY),QMessageBox::NoButton);
		return;
	}
	QByteArray pal;
	for(int tileset=0; tileset<GTSM_TILESET_COUNT; tileset++)
		pal.append(ui->gvPaletteManager->createPaletteBinaryData(tileset));
	file.write(pal);
	file.close();
}



void NESStageTool::saveASMAnimation(QString path)
{
	QString filename;
	if(path.isEmpty()) {
        filename = QFileDialog::getSaveFileName(this, ui->actionSaveAnimationASM->text().replace("&",""), "", tr("All files (*.*)"));
		if(filename.isEmpty())  return;
	} else {
		filename = path;
	}

	QFile file(filename);
	if(!file.open(QIODevice::WriteOnly)) {
		QMessageBox::warning(this,tr(FILE_SAVE_ERROR_TITLE),tr(FILE_SAVE_ERROR_BODY),QMessageBox::NoButton);
		return;
	}

//	file.write(ui->gvAnimation->createAnimationASMData(ui->labelMetaspriteName->text()+"_").toLocal8Bit());

	file.close();
}



void NESStageTool::setStageLabelPrefix(QString label)
{
	ui->lineASMLabel->setText(label);
}

void NESStageTool::setNewPaletteFile(QString pal)
{
	ui->gvPaletteManager->drawFullPaletteColours(pal);
    ui->gvGlobalTileset->loadCHRData();
}
