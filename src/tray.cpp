#include <sstream>
#include <stdexcept>

#include "tray.hpp"

namespace tray {

Icon::Icon(QObject *parent) : QSystemTrayIcon{parent} {}

void Icon::init() {
  react(tilo::state::Disconnected, "Not connected", QTime::currentTime());
}

void Icon::react(const QString &state, const QString &task,
                 const QTime &since) {
  if (state == tilo::state::Shutdown) {
    throw std::logic_error{"Shutdown not yet implemented"};
  }
  auto toolTip = QString{"Now: %1\nSince: %2"}.arg(task, since.toString());
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
