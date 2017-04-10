#-------------------------------------------------
#
# Project created by QtCreator 2015-01-13T11:10:14
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = NESStageTool
TEMPLATE = app

INCLUDEPATH += widgets/ui \
	widgets/metatile \
	widgets/palette \
	widgets/sprite \
	widgets/stage \
	widgets/tileset


SOURCES += main.cpp \
	nesstagetool.cpp \
	widgets/ui/lineeditasm.cpp \
	widgets/stage/stagemanager.cpp \
	widgets/palette/palettemanager.cpp \
	widgets/sprite/spritepaletteview.cpp \
	widgets/tileset/tilesetmanager.cpp \
	widgets/tileset/globaltilesetmanager.cpp \
	widgets/metatile/metatilemanager.cpp \
	widgets/metatile/metatileitem.cpp \
	widgets/tileset/chrthread.cpp \
    widgets/tileset/tilesetcache.cpp

HEADERS  += nesstagetool.h \
	widgets/ui/lineeditasm.h \
	widgets/stage/stagemanager.h \
	widgets/palette/palettemanager.h \
	widgets/sprite/spritepaletteview.h \
	widgets/tileset/tilesetmanager.h \
	widgets/tileset/globaltilesetmanager.h \
	widgets/metatile/metatilemanager.h \
	widgets/metatile/metatileitem.h \
	widgets/tileset/chrthread.h \
    widgets/tileset/tilesetcache.h

FORMS    += nesstagetool.ui

RESOURCES += \
    res/chr.qrc \
    res/palettes.qrc


win32:RC_ICONS += res/icon.ico
ICON = res/icon.icns

QMAKE_TARGET_PRODUCT = "NES Stage Tool"
QMAKE_TARGET_COMPANY = "Batty Bovine Productions, LLC"
QMAKE_TARGET_COPYRIGHT = "(c) 2016-2017 Batty Bovine Productions, LLC. All Rights Reserved."
GENERATED_VERSION_NUMBER = $$system(perl versionup.pl -get)
VERSION = $${GENERATED_VERSION_NUMBER}
