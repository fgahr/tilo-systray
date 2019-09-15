#include <QApplication>
#include <QMetaType>
#include <cstdlib>
#include <iostream>
#include <string>
#include <unistd.h>

#include "tilo.hpp"
#include "tray.hpp"

int main(int argc, char *argv[]) {
  QApplication app{argc, argv};

  tilo::Config conf = tilo::Config::defaultConfig();
  tray::Icon icon;
  tilo::Listener listener{&conf};
  icon.init();
  icon.show();
  QObject::connect(&listener, &tilo::Listener::notified, &icon,
                   &tray::Icon::react);
  listener.start();

  return app.exec();
}
