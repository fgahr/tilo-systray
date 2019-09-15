#ifndef TILO_SYSTRAY_TRAY_H
#define TILO_SYSTRAY_TRAY_H

#include <QSystemTrayIcon>

#include "tilo.hpp"

/** Namespace for all GUI-related definitions. */
namespace tray {
/**
 * System tray icon for the current tilo status.
 */
class Icon : public QSystemTrayIcon {
  Q_OBJECT

public:
  Icon(QObject *parent = nullptr);
  void init();

public slots:
  /** React to a status update. */
  void react(const QString &state, const QString &task, const QTime &since);

private:
  /** Get the appropriate icon for the given application state. */
  QIcon getIcon(const QString &state);
};
} // namespace tray

#endif /* TILO_SYSTRAY_TRAY_H */
