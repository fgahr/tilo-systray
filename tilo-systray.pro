TEMPLATE = app
TARGET = tilo-systray

DESTDIR = $$PWD/bin
OBJECTS_DIR = $$PWD/obj

QT += gui network

CONFIG += c++17
SOURCES += src/main.cpp
