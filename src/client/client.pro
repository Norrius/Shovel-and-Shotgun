#-------------------------------------------------
#
# Shovel&Shotgun by Norrius
# Created:
# 2011-09-16 19:33:39
# Separated:
# 2012-08-16 00:21:43
#
#-------------------------------------------------

QT       += core gui network

TEMPLATE = app

TARGET = SnS

SOURCES += main.cpp\
    ../entity.cpp \
    ../player.cpp \
    ../gameui.cpp \
    ../projectile.cpp \
    ../io.cpp \
    ../tile.cpp \
    ../global.cpp \
    ../weapon.cpp \
    guimainmenu.cpp

HEADERS  += \
    ../declarations.h \
    ../entity.h \
    ../player.h \
    ../gameui.h \
    ../projectile.h \
    ../io.h \
    ../tile.h \
    ../global.h \
    ../weapon.h \
    guimainmenu.h

win32:LIBS += -lws2_32

RESOURCES = ../qt.qrc \
            ui.qrc

FORMS += \
    gamewindow.ui \
    guimainmenu.ui
