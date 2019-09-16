#include <QDateTime>
#include <QFile>
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
  if (socket != nullptr) {
    socket->close();
  }
}

void Listener::run() {
  while (!isInterruptionRequested()) {
    while (socket == nullptr) {
      establishConnection(connectTimeoutMs);
    }

    while (socket->waitForReadyRead(-1)) {
      receiveAndHandleData();
    }

    socket->close();
    socket.reset();
    emit notified(tilo::state::Disconnected, "", QDateTime::currentDateTime());
  }
}

QString taskStatus(QString &taskName) {
  if (taskName == "--shutdown") {
    return state::Disconnected;
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
    emit notified(tilo::state::Error, "Error: invalid data",
                  QDateTime::currentDateTime());
  }
  auto task = json["task"].toString();
  auto sinceStr = json["since"].toString();
  QDateTime since = QDateTime::fromString(sinceStr, Qt::ISODate);
  auto state = taskStatus(task);
  emit notified(state, task, since);
}

void Listener::establishConnection(uint timeout) {
  if (socket != nullptr) {
    return;
  }
  if (!QFile::exists(conf->notificationSocket())) {
    // TODO: Find a better way instead of sleeping. File system watcher?
    msleep(timeout);
    // Server is still down
    if (!QFile::exists(conf->notificationSocket())) {
      return;
    }
  }
  socket = std::make_unique<QLocalSocket>();
  socket->connectToServer(conf->notificationSocket(), QIODevice::ReadOnly);
  if (!socket->waitForConnected(timeout)) {
    socket.reset();
  }
}

} // namespace tilo
