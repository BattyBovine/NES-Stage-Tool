#-------------------------------------------------
#
# Project created by QtCreator 2015-01-13T11:10:14
#
#-------------------------------------------------

QT       += core gui svg

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = NESStageTool
TEMPLATE = app

INCLUDEPATH += widgets/ui \
	widgets/ui/objects \
	widgets/ui/checkpoints \
	widgets/ui/undo \
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
	widgets/tileset/tilesetcache.cpp \
    widgets/ui/objects/objectmodel.cpp \
    widgets/ui/objects/objectdelegate.cpp \
    widgets/ui/checkpoints/checkpointmodel.cpp \
    widgets/ui/objects/objectcache.cpp \
    widgets/ui/objects/objectitem.cpp \
    widgets/ui/checkpoints/checkpointitem.cpp \
    widgets/ui/undo/undocommands.cpp

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
	widgets/tileset/tilesetcache.h \
    widgets/ui/objects/objectmodel.h \
    widgets/ui/objects/objectdelegate.h \
    widgets/ui/checkpoints/checkpointmodel.h \
    widgets/ui/objects/objectcache.h \
    widgets/ui/objects/objectitem.h \
    widgets/ui/checkpoints/checkpointitem.h \
    widgets/ui/undo/undocommands.h

FORMS    += nesstagetool.ui

RESOURCES += \
	res/chr.qrc \
	res/palettes.qrc \
    res/icons.qrc


win32:RC_ICONS += res/icon.ico
ICON = res/icon.icns

QMAKE_TARGET_PRODUCT = "NES Stage Tool"
QMAKE_TARGET_COMPANY = "Batty Bovine Productions, LLC"
QMAKE_TARGET_COPYRIGHT = "(c) 2016-2017 Batty Bovine Productions, LLC. All Rights Reserved."
GENERATED_VERSION_NUMBER = $$system(perl versionup.pl -get)
VERSION = $${GENERATED_VERSION_NUMBER}
