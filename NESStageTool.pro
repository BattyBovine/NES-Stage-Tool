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
#	widgets/metasprite \
	widgets/metatile \
	widgets/palette \
	widgets/sprite \
	widgets/stage \
	widgets/tileset \
#	widgets/animation


SOURCES += main.cpp \
	nesstagetool.cpp \
	widgets/ui/lineeditasm.cpp \
	widgets/ui/graphicsviewslider.cpp \
	widgets/ui/spritelistwidget.cpp \
#	widgets/ui/framelistwidget.cpp \
	widgets/stage/stagemanager.cpp \
	widgets/palette/palettemanager.cpp \
	widgets/sprite/spritepaletteview.cpp \
	widgets/tileset/tilesetmanager.cpp \
	widgets/tileset/globaltilesetmanager.cpp \
#	widgets/metasprite/metaspritemanager.cpp \
#	widgets/metasprite/metaspritetileitem.cpp \
	widgets/metatile/metatilemanager.cpp \
	widgets/metatile/metatileitem.cpp \
#	widgets/animation/animationmanager.cpp \
#	widgets/animation/animationframemanager.cpp \
#	widgets/animation/animationframeitem.cpp \
	widgets/tileset/chrthread.cpp \
    widgets/tileset/tilesetcache.cpp

HEADERS  += nesstagetool.h \
	widgets/ui/lineeditasm.h \
	widgets/ui/graphicsviewslider.h \
	widgets/ui/spritelistwidget.h \
#	widgets/ui/framelistwidget.h \
	widgets/stage/stagemanager.h \
	widgets/palette/palettemanager.h \
	widgets/sprite/spritepaletteview.h \
	widgets/tileset/tilesetmanager.h \
	widgets/tileset/globaltilesetmanager.h \
#	widgets/metasprite/metaspritemanager.h \
#	widgets/metasprite/metaspritetileitem.h \
	widgets/metatile/metatilemanager.h \
	widgets/metatile/metatileitem.h \
#	widgets/animation/animationmanager.h \
#	widgets/animation/animationframemanager.h \
#	widgets/animation/animationframeitem.h \
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
QMAKE_TARGET_COPYRIGHT = "(c) 2015 Batty Bovine Productions, LLC. Some Rights Reserved."
GENERATED_VERSION_NUMBER = $$system(perl versionup.pl -get)
VERSION = $${GENERATED_VERSION_NUMBER}
