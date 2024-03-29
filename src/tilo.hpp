#ifndef TILO_SYSTRAY_TILO_H
#define TILO_SYSTRAY_TILO_H

#include <QDateTime>
#include <QDir>
#include <QJsonDocument>
#include <QLocalSocket>
#include <QMetaType>
#include <QThread>

#include <memory>
#include <string>
#include <utility>

/** Namespace for all types and functions interacting with tilo. */
namespace tilo {

/**
 * A command for the server.
 */
class Command {
public:
  Command(QString op);
  /** Serialize this command as a JSON object. */
  QByteArray toJson();

  static const QString OPERATION_FIELD;
  static const QString LISTEN_OPERATION;

private:
  /** The operation for the server to perform. */
  QString operation;
};

/**
 * A response from the server.
 */
class Response {
public:
  Response(QString status, QString error = "");
  /** Whether the interaction failed. */
  bool failed();
  /** Deserialize raw JSON data as a Response object. */
  static Response fromJson(QByteArray data);

  static const QString STATUS_FIELD;
  static const QString ERROR_FIELD;

  static const QString SUCCESS_STATUS;
  static const QString ERROR_STATUS;

private:
  /** The status returned from the server. */
  QString status;
  /** Description of any error returned by the server. */
  QString error;
};

/**
 * The configuration for this instance.
 */
class Config {
public:
  /** The default configuration. */
  static Config defaultConfig();
  /** The base directory for temporary folders and files. */
  const QDir tempBaseDir;
  /** The temporary directory used by the server. */
  const QString tiloDirName;
  /** The name of the socket to connect to for notifications. */
  const QString socketName;
  /** The full path to the socket. */
  QString socketPath() const;

private:
  Config(QDir tempDir, QString dirName, QString socketName);
};

/**
 * Server/application states.
 *
 * Uses strings because an enum in a signal requires more work.
 */
namespace state {
const QString Active = "Active";
const QString Idle = "Idle";
const QString Disconnected = "Disconnected";
const QString Error = "Error";
} // namespace state

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
  /** The socket for notification data from the server. */
  std::unique_ptr<QLocalSocket> socket;
  /** Timeout for connection attempts to the socket. */
  uint connectTimeoutMillis = 1000;
  /** The program configuration. This is a pointer because it may be shared. */
  Config *conf;
  /** Establish a connection to the notification socket. */
  void establishConnection();
  /** Register as a listener on the server. */
  Response initiateListening();
  /** Handle the received data. */
  void receiveAndHandleData();
  /** Wait for the notification socket to be created. */
  void waitForSocket();
  /** Whether the notification exists. */
  bool socketExists() const;
};

} // namespace tilo

#endif /* TILO_SYSTRAY_TILO_H */
