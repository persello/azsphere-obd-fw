# Commands

The communication is bidirectional and unencrypted (as of now, considering AES encryption) and it is based on messages. Messages are ASCII strings with four initial characters (header) used to identify the kind of command, a variable number of characters for the parameters section and a trailing `\r\n`.\
There are no checksum or security features, thus I recommend to implement them at a lower level (like TCP).

The device should always answer commands sent from the application with the same header and some additional parameters. Some commands are spontaneously sent from the device (like interrupt ones, such as when a button is pressed).

Here is a list of commands that the application can send:

| Description | Header | Parameters | Answer parameters | Notes |
|:---:|:---:|:---:|:---:|:---:|
| Ping | PING | None | None | Future versions of the firmware may close the connection if a ping is not received regularly. |
| Request first use configuration | OOBE | None | None if allowed, `E` if denied. | Allowed only when device is new or has been reset. (**WIP**) |
| Request Wi-Fi scan | WISC | None | `E`: Error, `N`: No networks, `F` + [network list](#special-commands-and-parameters): Found. | Takes some seconds to complete. TCP threads might become unresponsive. |
| Request saved Wi-Fi network list | WISA | None | 


These are the commands that are spontaneously sent by the device:



## Special commands and parameters