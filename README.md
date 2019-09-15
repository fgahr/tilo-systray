# tilo-systray
A simple system tray application for the tilo time logging system. It registers
itself as a listener to current events and adjusts the system tray icon
accordingly. Even more so than `tilo`, this is not a finished product yet!
Lots of bugs to fix and features to add.

After implementing this program in Go proved unwise, I hope that the C++ version
is suitable.

# Installation
Make sure you have QT5 installed and all relevant development tools. Then run
```
qmake && make
```
This will place the tilo-systray program in the `bin/` directory.

# TODOs
- Attempt to reconnect if the server dies or is not up yet
- Actual error handling
- Add menu with shutdown option, implement proper shutdown procedure
- ...
