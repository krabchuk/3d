#-------------------------------------------------
#
# Project created by QtCreator 2015-05-28T23:14:35
#
#-------------------------------------------------

QT       += core gui
QT       += opengl

TARGET = 3d_task
TEMPLATE = app
LIBS += -lGLU

SOURCES += main.cpp\
        mainwindow.cpp \
    matrix.cpp \
    solver.cpp

HEADERS  += mainwindow.h
CONFIG += "debug"
