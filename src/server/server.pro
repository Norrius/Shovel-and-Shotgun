#-------------------------------------------------
#
# Shovel&Shotgun by Norrius
# Created:
# 2011-09-16 19:33:39
# Separated:
# 2012-08-16 00:22:17
#
#-------------------------------------------------

QT       += core gui network

TEMPLATE = app

TARGET = SnSserver

SOURCES += main.cpp\
    ../entity.cpp \
    ../player.cpp \
    ../gamecore.cpp \
    ../projectile.cpp \
    ../io.cpp \
    ../thread.cpp \
    ../tile.cpp \
    ../global.cpp \
    ../weapon.cpp

HEADERS  += \
    ../declarations.h \
    ../entity.h \
    ../player.h \
    ../gamecore.h \
    ../projectile.h \
    ../io.h \
    ../thread.h \
    ../tile.h \
    ../global.h \
    ../weapon.h

win32:LIBS += -lws2_32
