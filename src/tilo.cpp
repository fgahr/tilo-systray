#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <unistd.h>

#include "tilo.hpp"

namespace tilo {

Config Config::defaultConfig() {
  QDir tmpDir = QDir::temp();
  tmpDir.cd(QString{"tilo%1"}.arg(getuid()));
  QString ntfSock = QString{"notify"};
  return Config{tmpDir, ntfSock};
}

QString Config::notificationSocket() {
  return tmpDir.absoluteFilePath(ntfSock);
}

Listener::Listener(Config *conf, QObject *parent)
    : QThread{parent}, socket(nullptr), conf(conf) {}

Listener::~Listener() {
  requestInterruption();
  socket->close();
  delete socket;
}

void Listener::run() {
  while (socket == nullptr) {
    socket = establishConnection(connectTimeoutMs);
  }

  while (socket->waitForReadyRead(-1)) {
    receiveAndHandleData();
  }

  if (!isInterruptionRequested()) {
    delete socket;
    // NOTE: Depending on compiler optimization, a tail call may blow the stack.
    run();
  }
}

QString taskStatus(QString &taskName) {
  if (taskName == "--shutdown") {
    return state::Shutdown;
  } else if (taskName == "") {
    return state::Idle;
  } else {
    return state::Active;
  }
}

void Listener::receiveAndHandleData() {
  auto data = socket->readLine();
  QJsonDocument jdoc = QJsonDocument::fromJson(data);
  auto json = jdoc.object();
  if (json.isEmpty() || !json.contains("task") || !json.contains("since")) {
    // TODO: Signal error status instead.
    throw std::logic_error{"Unexpected JSON format."};
  }
  auto task = json["task"].toString();
  auto sinceStr = json["since"].toString();
  QDateTime since = QDateTime::fromString(sinceStr, Qt::ISODate);
  auto state = taskStatus(task);
  emit notified(state, task, since);
}

QIODevice *Listener::establishConnection(uint timeout) {
  auto lsocket = new QLocalSocket;
  lsocket->connectToServer(conf->notificationSocket(), QIODevice::ReadOnly);
  if (lsocket->waitForConnected(timeout)) {
    return lsocket;
  } else {
    delete lsocket;
    return nullptr;
  }
}

} // namespace tilo
