#include <QDateTime>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <unistd.h>

#include "tilo.hpp"

namespace tilo {

const QString Command::OPERATION_FIELD = "operation";
const QString Command::LISTEN_OPERATION = "listen";

Command::Command(QString op) : operation(std::move(op)) {}

QByteArray Command::toJson() {
  QJsonObject object{QPair<QString, QString>{Command::OPERATION_FIELD,
                                             Command::LISTEN_OPERATION}};
  QJsonDocument document{object};
  return document.toJson();
}

const QString Response::STATUS_FIELD = "status";
const QString Response::ERROR_FIELD = "error";

const QString Response::SUCCESS_STATUS = "success";
const QString Response::ERROR_STATUS = "error";

Response::Response(QString status, QString error)
    : status(std::move(status)), error(std::move(error)) {}

Response Response::fromJson(QByteArray data) {
  auto object = QJsonDocument::fromJson(data).object();
  auto status = object[Response::STATUS_FIELD].toString();
  auto error = object[Response::ERROR_FIELD].toString();
  return Response{status, error};
}

bool Response::failed() { return status == Response::ERROR_STATUS; }

Config Config::defaultConfig() {
  return Config{QDir::temp(), QString{"tilo%1"}.arg(getuid()), "server"};
}

Config::Config(QDir tempDir, QString dirName, QString socketName)
    : tempBaseDir(std::move(tempDir)), tiloDirName(std::move(dirName)),
      socketName(std::move(socketName)) {}

QString Config::socketPath() const {
  return QDir{tempBaseDir.filePath(tiloDirName)}.absoluteFilePath(socketName);
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
      establishConnection();
    }

    auto resp = initiateListening();
    if (resp.failed()) {
      emit notified(tilo::state::Error, "", QDateTime::currentDateTime());
      break;
    }

    while (socket->waitForReadyRead(-1)) {
      receiveAndHandleData();
    }

    socket->close();
    socket.reset(nullptr);
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

void Listener::establishConnection() {
  if (socket != nullptr) {
    return;
  }
  if (!socketExists()) {
    waitForSocket();
  }
  socket = std::make_unique<QLocalSocket>();
  socket->connectToServer(conf->socketPath(), QIODevice::ReadWrite);
  if (!socket->waitForConnected(connectTimeoutMillis)) {
    socket.reset(nullptr);
  }
}

Response Listener::initiateListening() {
  Command cmd{Command::LISTEN_OPERATION};
  socket->write(cmd.toJson());
  if (socket->waitForReadyRead(connectTimeoutMillis)) {
    auto data = socket->readLine();
    return Response::fromJson(data);
  }
  return Response{Response::ERROR_STATUS,
                  "Failed to receive connection confirmation."};
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

void Listener::waitForSocket() {
  if (!conf->tempBaseDir.exists(conf->tiloDirName)) {
    if (!conf->tempBaseDir.mkdir(conf->tiloDirName)) {
      qWarning() << "Failed to create temp dir "
                 << conf->tempBaseDir.filePath(conf->tiloDirName);
    }
  }

  while (!socketExists()) {
    // TODO: Find way to do this without sleeping.
    msleep(connectTimeoutMillis);
  }
}

bool Listener::socketExists() const {
  return QFile::exists(conf->socketPath());
}

} // namespace tilo
