//    Copyright 2019 Jan-Eric Schober

//    Licensed under the Apache License, Version 2.0 (the "License");
//    you may not use this file except in compliance with the License.
//    You may obtain a copy of the License at

//        http://www.apache.org/licenses/LICENSE-2.0

//    Unless required by applicable law or agreed to in writing, software
//    distributed under the License is distributed on an "AS IS" BASIS,
//    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//    See the License for the specific language governing permissions and
//    limitations under the License.

#ifndef STREAMCOMMANDER_HPP
#define STREAMCOMMANDER_HPP

// Arduino Standard Libraries
#include <Arduino.h>
#include <EEPROM.h>
#include <MessageTypes.hpp>

class StreamCommander
{
private:
    // Types
    typedef void (*CommandCallbackFunction)( String arguments, StreamCommander * instance );
    typedef void (*DefaultCallbackFunction)( String command, String arguments, StreamCommander * instance );

    // Structs
    struct CommandContainer
    {
        String * command;
        CommandCallbackFunction callbackFunction;

        ~CommandContainer()
        {
            // Destroy this pointer, since it is not safe that it will be deleted otherwise.
            delete command;
        }
    };

    // Constants
    static const long STREAM_BUFFER_TIMEOUT  = 100;
    static const char COMMAND_EOL_CR = '\r';
    static const char COMMAND_EOL_NL = '\n';
    static const char COMMAND_DELIMITER = ' ';
    static const char MESSAGE_DELIMITER = ':';
    static const int ID_MAX_LENGTH = 32;

    static const String COMMAND_ACTIVATE;
    static const String COMMAND_DEACTIVATE;
    static const String COMMAND_ISACTIVE;
    static const String COMMAND_SETECHO;
    static const String COMMAND_SETID;
    static const String COMMAND_GETID;
    static const String COMMAND_PING;
    static const String COMMAND_GETSTATUS;
    static const String COMMAND_LISTCOMMANDS;

    // Variables
    Stream * streamInstance;
    String status = "";
    bool active;
    bool echoCommands;
    bool addStandardCommands;
    long streamBufferTimeout;
    String id = "";
    char commandDelimiter = COMMAND_DELIMITER;
    char messageDelimiter = MESSAGE_DELIMITER;
    CommandContainer * commands;
    DefaultCallbackFunction defaultCallbackFunction;
    int numCommands;

    // Private Methods
    void setStreamInstance( Stream * streamInstance ); // Sets the streamInstance of the StreamCommander.
    Stream * getStreamInstance(); // Gets the current streamInstance of the StreamCommander.
    void setAddStandardCommands( bool addStandardCommands ); // Sets whether the standard commands should be added or not (true/false).
    bool shouldAddStandardCommands(); // Returns whether the standard commadns should be added or not.
    void saveIdToEEPROM( String id ); // Saves and ID to the EEPROM if it differs from the old one.
    void loadIdFromEEPROM(); // Loads the ID from the EEPROM.
    void deleteCommands(); // Deletes all registered commands.
    void setNumCommands( int numCommands ); // Sets the number of the currently registered commands.
    void incrementNumCommands(); // Increments the number of the currently registered commands.
    CommandContainer * getCommandContainer( String command ); // Gets the container containing all commands.
    int getCommandContainerNum( String command ); // Returns the number (position) of a specific command in the command container.

    static void commandActivate( String arguments, StreamCommander * instance );// Definition of the command COMMAND_ACTIVATE.
    static void commandDeactivate( String arguments, StreamCommander * instance ); // Definition of the command COMMAND_DEACTIVATE.
    static void commandIsActive( String arguments, StreamCommander * instance ); // Definition of the command COMMAND_ISACTIVE.
    static void commandSetEcho( String arguments, StreamCommander * instance ); // Definition of the command COMMAND_SETECHO.
    static void commandSetId(  String id, StreamCommander * instance ); // Definition of the command COMMAND_SETID.
    static void commandGetId( String arguments, StreamCommander * instance ); // Definition of the command COMMAND_GETID.
    static void commandPing( String arguments, StreamCommander * instance ); // Definition of the command COMMAND_PING.
    static void commandGetStatus( String arguments, StreamCommander * instance ); // Definition of the command COMMAND_GETSTATUS.
    static void commandListCommands( String arguments, StreamCommander * instance ); // Definition of the command COMMAND_LISTCOMMANDS.
    void addAllStandardCommands(); // Registers all the above commands.
    static void defaultCommand( String command, String arguments, StreamCommander * instance ); // Definition of the default callback.

public:
    // Constructor
    StreamCommander( Stream * streamInstance = &Serial ); // Constructor, instance of a Stream object as argument.
    
    // Destructor
    ~StreamCommander();

    // Public Methods
    void init( // Init function for setting up the StreamCommander correctly, after it has been successfuly constructed.
        bool active = true, // Whether the StreamCommander is set to active or not. This only influences the automatic status updates.
        char commandDelimiter = COMMAND_DELIMITER, // Character which delimits a command from its' arguments.
        char messageDelimiter = MESSAGE_DELIMITER, // Character which delimits a message from its' contents.
        bool echoCommands = false, // Should commands be echoed at arrival or not?
        bool addStandardCommands = true, // Should the standard commands be added or not?
        long streamBufferTimeout = STREAM_BUFFER_TIMEOUT // Sets the timeout of the specific streams' buffer.
    );
    void setActive( bool active ); // Sets whether the automatic status updates are activated or not (true/false).
    bool isActive(); // Returns if the automatic status updates are activated.
    void setCommandDelimiter( char commandDelimiter ); // Sets the command delimiter, separating the command from a potential argument.
    char getCommandDelimiter(); // Gets the command delimiter.
    void setMessageDelimiter( char messageDelimiter ); // Sets the message delimiter, separating the type from the content.
    char getMessageDelimiter(); // Gets the message delimiter.
    void setEchoCommands( bool echoCommands ); // Sets whether all incoming commands should be echoed or not (true/false).
    bool shouldEchoCommands(); // Returns if all incomming commands should be echoed.
    void setStreamBufferTimeout( long streamBufferTimeout ); // Sets the timeout of the specific streams' buffer. 
    long getStreamBufferTimeout(); // Returns the timeout of the specific streams' buffer.
    void setId( String id ); // Sets the ID of the StreamCommander/Device.
    String getId(); // Gets the ID of the StreamCommander/Device.

    void updateStatus( String status ); // Update the status of the StreamCommander/Device; updates the status and sends an automatic status message only if the status changed.
    void setStatus( String status ); // Sets the current status StreamCommander/Device.
    String getStatus(); // Gets the current status StreamCommander/Device.

    void addCommand( String command, CommandCallbackFunction commandCallback ); // Registers a new command; a command name tied to a command callback.
    int getNumCommands(); // Gets the number of the registered commands.
    String getCommandList(); // Gets a list of all registered commands.
    void setDefaultCallback( DefaultCallbackFunction defaultCallbackFunction ); // Sets the default callback which gets called in case a sent command is not registered.
    DefaultCallbackFunction getDefaultCallback(); // Gets the default callback.

    void fetchCommand(); // Fetches and interprets incoming commands, and invokes the corresponding callbacks. This should be called in the loop or after an interrupt/event.

    void sendMessage( String type, String content ); // Sends a message with a specific type and content separated by our delimiter.
    void sendResponse( String response ); // Sends a message of type MessageType::RESPONSE
    void sendInfo( String info ); // Sends a message of type MessageType::INFO
    void sendError( String error ); // Sends a message of type MessageType::ERROR
    void sendPing(); // Sends a message of type MessageType::PING, contains a "reply"
    void sendStatus(); // Sends a message of type MessageType::STATUS, contains the current status
    void sendId(); // Sends a message of type MessageType::ID, contains the current ID
    void sendIsActive(); // Sends a message of type MessageType::ACTIVE, contains the current active status
    void sendEcho( String echo ); // Sends a message of type MessageType::ECHO
    void sendCommands(); // Sends a message of type MessageType::COMMANDS, contains a list of currently registered commands
};

#endif // STREAMCOMMANDER_HPP
