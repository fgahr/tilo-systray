TEMPLATE = app
TARGET = tilo-systray

DESTDIR = $$PWD/bin
OBJECTS_DIR = $$PWD/obj

QT += gui network widgets

CONFIG += c++17
HEADERS += src/tilo.hpp src/tray.hpp
SOURCES += src/main.cpp src/tilo.cpp src/tray.cpp
RESOURCES = $$PWD/resources.qrc
