#include <QLocalSocket>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <unistd.h>

QString socket_address() {
  // TODO: Get location of /tmp dir from environment (check with Go code first).
  return QString{"/tmp/tilo%1/notify"}.arg(getuid());
}

int main(int argc, char *argv[]) {
  QLocalSocket *socket = new QLocalSocket;
  QTextStream out(stdout);
  socket->connectToServer(socket_address(), QIODevice::ReadOnly);
  while (socket->waitForReadyRead()) {
    out << socket->readLine(1024);
    out.flush();
  }
}
