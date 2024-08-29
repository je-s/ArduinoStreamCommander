# ArduinoStreamCommander
The ArduinoStreamCommander is a library for interacting with an Arduino over any [Stream](https://www.arduino.cc/reference/en/language/functions/communication/stream/)-based interface via commands, as long as the member functions `setTimeout, available, println, readString, flush` are implemented accordingly.
Those interfaces include [Serial](https://www.arduino.cc/reference/en/language/functions/communication/serial) (for which this library was initially meant for), [SoftwareSerial](https://www.arduino.cc/en/Reference/softwareSerial), [Wire](https://www.arduino.cc/en/Reference/Wire) and [Ethernet](https://www.arduino.cc/en/Reference/Ethernet).

The target was a very lightweight and convenient library, which allows to easily add new commands and send status updates automatically in case the data changed.

Features:
* Execute commands on the device (e.g. to get or set different values of the device).
    * Add custom commands with callback functions.
    * Use a set of basic standard commands.
* Send different kind of messages with a simple and lightweight message format.
    * Send status updates of the device (e.g. for pushing new sensor values via Serial Interface).
    * Send arbitrary types of messages.
* Get/Set an ID from and to the EEPROM (If the target board has one available).
# Folder structure
* `src`  contains the source code.
* `examples` contains an example sketch.
# Installing and using ArduinoStreamCommander with the Arduino IDE
* Before usage, installing [ArduinoStreamCommander-MessageTypes](https://github.com/je-s/ArduinoStreamCommander-MessageTypes) is required. This Lib just contains standard message types, but can be easily extended and customised if required.
* To install the library either clone and ZIP the folder, or download one of the releases. After that follow the instructions [here](https://www.arduino.cc/en/Guide/Libraries#toc2).
* In order to use ArduinoStreamCommander, `<StreamCommander.hpp>` needs to be included.
# Usage
For more detailed explainations on the particular functions/functionalities, please reference to the comments in the source code.
Also, there's an example ([examples/test/test.ino](examples/test/test.ino)) which showcases the core functionalities of the StreamCommander.
## Basic usage
1. Instantiate a new `ArduinoStreamCommander`-Object: `StreamCommander commander = StreamCommander();`.  
The StreamCommander will be initialised with the standard `Serial`-Object by default.
    1. Optionally, instantiate the StreamCommander with an alternative `Stream`-based interface:  
    `StreamCommander commander = StreamCommander( &Serial1 );`
2. Initialise the Stream-interface, e.g.: `Serial.begin( 9600 );`
3. Initialise the StreamCommander: `commander.init();`. (Optionally with arguments, see source code for more information.)
    1. Call further optional functions while setting up.
4. Add custom commands with a name and a matching callback function, e.g.:  
`commander.addCommand( "test", testCallback );`
    1. The callback function has to follow following typedef:  
    `typedef void (*CommandCallbackFunction)( String arguments , StreamCommander * instance )`.
5. Optionally, set a default callback function, e.g.:  
`commander.setDefaultCallback( defaultCallback );`
    1. The callback function has to follow following typedef:  
    `typedef void (*DefaultCallbackFunction)( String command , String arguments , StreamCommander * instance )`
6. Call `commander.fetchCommand();` in every `loop()`. This function catches incoming commands. If the command has been registered and found, the according callback will be called, and (optional) arguments will be parsed and passed to the according callback function. If the command has not been registered, the default callback will be called.
    1. This function can also be called after an hardware interrupt.
    2. **Carriage return ("\r"), Newline ("\n") or Carriage return + Newline ("\r\n") do each signalise the end of a command.**
7. Send status updates with `updateStatus`-function.
    1. If the status has changed since the last update, a new status message will automatically be sent.
    2. If the device is not activated, possible status updates won't be sent out. They can still be queried manually with the `status`-command.
8. In case you need the device to have an ID (for example if you need to adress multiple devices separately), set an id with the `setid`-command the first time you boot the device. If an EEPROM is available on the board:
    1. The ID will be persisted in the EEPROM of the device, and will automatically be loaded on every initialisation of the StreamCommander. No need to hardcode this.
    2. The ID will only be updated in the EEPROM if it really changes, which extends the lifespan of the EEPROM.
## Arguments
A command can be followed by arguments. Those arguments are separated from the rest of the command by the first occurence of the command delimiter (a blank space by default).
The delimiter can be changed with the function `commander.setCommandDelimiter( char delimiter );` or in the `init`-function.
The arguments are parsed as a single string, which can then be processed arbitrarily.

Example:
* Command with arguments: `test 1 2 3`
* Command: `test`
* Arguments: `1 2 3`
## Defining Command-Callbacks
As stated above, a callback for commands follows the `CommandCallbackFunction`-typedef:  
`typedef void (*CommandCallbackFunction)( String arguments , StreamCommander * instance )`  
It has two parameters: a string of arguments, and a pointer to the instance of the StreamCommander calling the callback.

The latter parameter is especially there, in case multiple instances of stream commander are running (for example if we have multiple serial ports connected), in order to distinguish which instance called the callback.

If arguments have been passed with the command, the arguments are parsed and passed as a single string.

Example for turning the built in LED on and off:
```C++
commander.addCommand( "led", cmdLed );
...
void cmdLed( String arguments, StreamCommander * instance )
{
    arguments.trim();
    
    if ( arguments.equals( "on" ) )
    {
        digitalWrite( LED_BUILTIN, HIGH );
    }
    else if ( arguments.equals( "off" ) )
    {
        digitalWrite( LED_BUILTIN, LOW );
    }
}
```
## Defining the Default-Callbacks
This function gets called if an unknown/unregistered command has been delivered to the StreamCommander. As stated above, the default callback follows the `DefaultCallbackFunction`-typedef:  
`typedef void (*DefaultCallbackFunction)( String command , String arguments , StreamCommander * instance )`

In addition to the `CommandCallbackFunction` it has the parameter `command` which contains the name of the command that has been tried to be invoked.

Example:
```C++
commander.setDefaultCallback( cmdDefault );
...
void cmdDefault( String command, String arguments, StreamCommander * instance )
{
    instance->sendResponse( "Command \"" + command + "\" with arguments \"" + arguments + "\" not registered." );
}
```
## Standard Commands
The StreamCommander has several standard commands which implement basic functionalities. Adding those commands can be surpressed by specifing this when calling the `init`-function.

| Command | Purpose | Arguments |
| --------| --------| --------- |
| activate | Activates the automatic publishing of new status-messages | |
| deactivate | Deactivates the automatic publishing of new status-messages | |
| isactive | Returns whether the device is set to active or not | | 
| setecho | Sets the echoing of incoming commands on or off | on / off |
| setid | Sets the ID of the device | &lt;id&gt; |
| getid | Returns the ID of the device | |
| ping | Returns a ping response message (usually a "ping:reply" message) | |
| getstatus | Returns the current status of the device | |
| commands | Returns all registered commands of the device | |
# Message format
To make the communication more easy and consistent, a simple message format has been defined, which is used by the ArduinoStreamCommander.\
The definition can be found here: [SerialMessageFormat](https://github.com/je-s/SerialMessageFormat)
## Standard Message Types
The StreamCommander uses several standard message types, which are defined in [ArduinoStreamCommander-MessageTypes](https://github.com/je-s/ArduinoStreamCommander-MessageTypes).

| Type | Purpose |
| -----| ------- |
| response | Return responses after a command has been executed |
| info | Inform about something like an internal event (e.g. for logging-purposes) |
| error | Inform about an error/problem which occured during the runtime |
| ping | Contains a ping response message (usually containts "reply") |
| status | Send the current/most recent status of the device (Especially used for status updates) |
| id | Contains the id of the device |
| active | Contains whether the device is set to active or not |
| echo | Contains an echo of the last input received |
| commands | Contains a list of all registered commands of a Device |
| command | Contains a command to be passed to an Arduino |
