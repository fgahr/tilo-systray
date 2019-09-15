#ifndef TILO_SYSTRAY_TILO_H
#define TILO_SYSTRAY_TILO_H

#include <QDateTime>
#include <QDir>
#include <QJsonDocument>
#include <QLocalSocket>
#include <QMetaType>
#include <QThread>

#include <string>
#include <utility>

/** Namespace for all types and functions interacting with tilo. */
namespace tilo {

/** Server/application states. */
namespace state {
const QString Active = "Active";
const QString Idle = "Idle";
const QString Disconnected = "Disconnected";
const QString Error = "Error";
const QString Shutdown = "Shutdown";
} // namespace state

/**
 * The configuration for this instance.
 */
class Config {
public:
  /** The default configuration. */
  static Config defaultConfig();
  /** The absolute path of the notification socket. */
  QString notificationSocket();
  /** How long to wait for a notification at each iteration. */
  int readTimeoutMs();

private:
  QDir tmpDir;
  QString ntfSock;
  Config(QDir tmpDir, QString ntfSock)
      : tmpDir(std::move(tmpDir)), ntfSock(std::move(ntfSock)) {}
};

/**
 * A listener on the notification socket. Waits for status notifications and
 * emits signals on receiving them.
 */
class Listener : public QThread {
  Q_OBJECT

public:
  Listener(Config *conf, QObject *parent = nullptr);
  ~Listener();

signals:
  /** Signal that a status update was received. */
  void notified(const QString &state, const QString &task,
                const QDateTime &since);

protected:
  void run() override;

private:
  QIODevice *socket;
  /** The program configuration. This is a pointer because it may be shared. */
  Config *conf;
  /** Establish a connection to the notification socket. */
  QIODevice *establishConnection();
  /** Handle the received data. */
  void receiveAndHandleData();
};

} // namespace tilo

#endif /* TILO_SYSTRAY_TILO_H */
