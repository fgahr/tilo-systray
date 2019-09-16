#include <sstream>
#include <stdexcept>

#include "tray.hpp"

namespace tray {

Icon::Icon(QObject *parent) : QSystemTrayIcon{parent} {}

void Icon::init() {
  react(tilo::state::Disconnected, "Not connected",
        QDateTime::currentDateTime());
}

QString description(const QString &state, const QString &task) {
  if (state == tilo::state::Idle) {
    return "Idle";
  } else if (state == tilo::state::Disconnected) {
    return "Disconnected";
  } else {
    return task;
  }
}

void Icon::react(const QString &state, const QString &task,
                 const QDateTime &since) {
  auto taskDescription = description(state, task);
  auto toolTip =
      QString{"Now: %1\nSince: %2"}.arg(taskDescription, since.toString());
  setToolTip(toolTip);
  setIcon(getIcon(state));
}

QIcon Icon::getIcon(const QString &state) {
  if (state == tilo::state::Active) {
    return QIcon{":/icons/busy.png"};
  } else if (state == tilo::state::Idle) {
    return QIcon{":/icons/idle.png"};
  } else if (state == tilo::state::Error) {
    return QIcon{":/icons/fail.png"};
  } else if (state == tilo::state::Disconnected) {
    return QIcon{":/icons/disc.png"};
  } else {
    throw std::logic_error{"No icon for unexpected state."};
  }
}

} // namespace tray
