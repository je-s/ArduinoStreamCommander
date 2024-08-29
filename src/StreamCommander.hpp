/*
    Copyright 2019 Jan-Eric Schober

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#ifndef STREAMCOMMANDER_HPP
#define STREAMCOMMANDER_HPP

// Arduino Standard Libraries
#include <Arduino.h>
#include <MessageTypes.hpp>

#if __has_include("<EEPROM.h>")
#include <EEPROM.h>
#endif

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
    static const String PING_REPLY;

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
    // Sets the streamInstance of the StreamCommander.
    void setStreamInstance( Stream * streamInstance );

    // Gets the current streamInstance of the StreamCommander.
    Stream * getStreamInstance();

    // Sets whether the standard commands should be added or not (true/false).
    void setAddStandardCommands( bool addStandardCommands );

    // Returns whether the standard commadns should be added or not.
    bool shouldAddStandardCommands();

    // This functions do only get implemented in case an EEPROM is available for the Board.
    #if __has_include("<EEPROM.h>")
    // Saves an ID to the EEPROM if it differs from the old one.
    void saveIdToEeprom( String id );

    // Loads the ID from the EEPROM.
    void loadIdFromEeprom();
    #endif

    // Gets the container containing all commands.
    CommandContainer * getCommandContainer( String command );

    // Returns the index (position) of a specific command in the command container by name.
    int getCommandContainerIndex( String command );

    // Deletes all registered commands.
    void deleteCommands();

    // Sets the number of the currently registered commands.
    void setNumCommands( int numCommands );

    // Increments the number of the currently registered commands.
    void incrementNumCommands();

    // Tries to execute a command with given arguments. Arguments can be empty.
    void executeCommand( String command, String arguments );

    // Definition of the command COMMAND_ACTIVATE.
    static void commandActivate( String arguments, StreamCommander * instance );

    // Definition of the command COMMAND_DEACTIVATE.
    static void commandDeactivate( String arguments, StreamCommander * instance );

    // Definition of the command COMMAND_ISACTIVE.
    static void commandIsActive( String arguments, StreamCommander * instance );

    // Definition of the command COMMAND_SETECHO.
    static void commandSetEcho( String arguments, StreamCommander * instance );

    // Definition of the command COMMAND_SETID.
    static void commandSetId( String id, StreamCommander * instance );

    // Definition of the command COMMAND_GETID.
    static void commandGetId( String arguments, StreamCommander * instance );

    // Definition of the command COMMAND_PING.
    static void commandPing( String arguments, StreamCommander * instance );

    // Definition of the command COMMAND_GETSTATUS.
    static void commandGetStatus( String arguments, StreamCommander * instance );

    // Definition of the command COMMAND_LISTCOMMANDS.
    static void commandListCommands( String arguments, StreamCommander * instance );

    // Registers all the above commands.
    void addAllStandardCommands();

    // Definition of the default callback.
    static void defaultCommand( String command, String arguments, StreamCommander * instance );

public:
    // Constructor
    // Constructor, instance of a Stream object as argument.
    StreamCommander( Stream * streamInstance = &Serial );

    // Destructor
    ~StreamCommander();

    // Public Methods
    // Init function for setting up the StreamCommander correctly, after it has been successfuly constructed.
    void init(
        // Whether the StreamCommander is set to active or not. This only influences the automatic status updates.
        bool active = true,

        // Character which delimits a command from its' arguments.
        char commandDelimiter = COMMAND_DELIMITER,

        // Character which delimits a message from its' contents.
        char messageDelimiter = MESSAGE_DELIMITER,

        // Should commands be echoed at arrival or not?
        bool echoCommands = false,

        // Should the standard commands be added or not?
        bool addStandardCommands = true,

        // Sets the timeout of the specific streams' buffer.
        long streamBufferTimeout = STREAM_BUFFER_TIMEOUT
    );

    // Sets whether the automatic status updates are activated or not (true/false).
    void setActive( bool active );

    // Returns if the automatic status updates are activated.
    bool isActive();

    // Sets the command delimiter, separating the command from a potential argument.
    void setCommandDelimiter( char commandDelimiter );

    // Gets the command delimiter.
    char getCommandDelimiter();

    // Sets the message delimiter, separating the type from the content.
    void setMessageDelimiter( char messageDelimiter );

    // Gets the message delimiter.
    char getMessageDelimiter();

    // Sets whether all incoming commands should be echoed or not (true/false).
    void setEchoCommands( bool echoCommands );

    // Returns if all incomming commands should be echoed.
    bool shouldEchoCommands();

    // Sets the timeout of the specific streams' buffer. 
    void setStreamBufferTimeout( long streamBufferTimeout );

    // Returns the timeout of the specific streams' buffer.
    long getStreamBufferTimeout();

    // Sets the ID of the StreamCommander/Device.
    // The ID gets only saved to an EEPROM if one is available.
    void setId( String id );

    // Gets the ID of the StreamCommander/Device.
    String getId();

    // Update the status of the StreamCommander/Device; updates the status and sends an automatic status message only if the status changed.
    void updateStatus( String status );

    // Sets the current status StreamCommander/Device.
    void setStatus( String status );

    // Gets the current status StreamCommander/Device.
    String getStatus();

    // Registers a new command; a command name tied to a command callback.
    void addCommand( String command, CommandCallbackFunction commandCallback );

    // Gets the number of the registered commands.
    int getNumCommands();

    // Gets a list of all registered commands.
    String getCommandList();

    // Sets the default callback which gets called in case a sent command is not registered.
    void setDefaultCallback( DefaultCallbackFunction defaultCallbackFunction );

    // Gets the default callback.
    DefaultCallbackFunction getDefaultCallback();

    // Fetches and interprets incoming commands, and invokes the corresponding callbacks. This should be called in the loop or after an interrupt/event.
    void fetchCommand();

    // Sends a message with a specific type and content separated by our delimiter.
    void sendMessage( String type, String content );

    // Sends a message of type MessageType::RESPONSE.
    void sendResponse( String response );

    // Sends a message of type MessageType::INFO.
    void sendInfo( String info );

    // Sends a message of type MessageType::ERROR.
    void sendError( String error );

    // Sends a message of type MessageType::PING, contains a "reply".
    void sendPing();

    // Sends a message of type MessageType::STATUS, contains the current status.
    void sendStatus();

    // Sends a message of type MessageType::ID, contains the current ID.
    void sendId();

    // Sends a message of type MessageType::ACTIVE, contains the current active status.
    void sendIsActive();

    // Sends a message of type MessageType::ECHO.
    void sendEcho( String echo );

    // Sends a message of type MessageType::COMMANDS, contains a list of currently registered commands.
    void sendCommands();
};

#endif // STREAMCOMMANDER_HPP