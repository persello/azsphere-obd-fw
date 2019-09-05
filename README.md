# Azure Sphere OBD data logger

This is a data logger for vehicles that combines data from an OBDII click, a GPS and onboard sensors in order to be analyzed by its [companion Android app](https://github.com/persello/azsphere-obd-app) (requires Android Pie or newer).

## Assembling the device
Refer to the [device page](docs/device.md) for details about assembling the device and building your custom click.

## Communication with the mobile app

By default, the device communicates through a TCP socket on the interface `wlan0` on port 15500. This behavior can be changed in the [configuration file](azsphere_obd_fw\config.h).\
You could also create a new driver in order to route the communication to another interface (e.g. serial, Bluetooth...): you'll only need to reimplement the four functions defined in the [application TCP header file](azsphere_obd_fw\appTCP.h). If you do this or other useful changes, a pull request would be appreciated, so we can start integrating multiple communication interfaces into the same project.

If you are interested in expanding the command set or you want to build a command interpreter for this device, you can find a list of all the supported commands in the [commands page](/docs/commands.md).

## Logging

Simple text logs are saved in the microSD card. These logs are numbered sequentially (`0.log`, `1.log` and so on) and the content is easily readable. Each line starts with the date and time from the internal RTC clock and is followed by a ten-character identifier and its value. For more details about the structure of a log message, for the available identifiers and measurement units, please refer to the [log page](docs/log.md).

## TODO: LED indicators