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

#include "StreamCommander.hpp"

const String StreamCommander::COMMAND_ACTIVATE = "activate";
const String StreamCommander::COMMAND_DEACTIVATE = "deactivate";
const String StreamCommander::COMMAND_ISACTIVE = "isactive";
const String StreamCommander::COMMAND_SETECHO = "echo";
const String StreamCommander::COMMAND_SETID = "setid";
const String StreamCommander::COMMAND_GETID = "getid";
const String StreamCommander::COMMAND_PING = "ping";
const String StreamCommander::COMMAND_GETSTATUS = "status";
const String StreamCommander::COMMAND_LISTCOMMANDS = "commands";

StreamCommander::StreamCommander( Stream * streamInstance )
{
    setStreamInstance( streamInstance );
}

StreamCommander::~StreamCommander()
{
    deleteCommands();
}

void StreamCommander::init( bool active, char commandDelimiter, char messageDelimiter, bool echoCommands, bool addStandardCommands, long streamBufferTimeout )
{
    loadIdFromEEPROM();
    setCommandDelimiter( commandDelimiter );
    setMessageDelimiter( messageDelimiter );
    setStreamBufferTimeout( streamBufferTimeout );
    setActive( active );
    setEchoCommands( echoCommands );
    setAddStandardCommands( addStandardCommands );

    Stream * streamInstance = getStreamInstance();

    while ( !streamInstance ); // Wait for the Stream to get initialized
    streamInstance->flush(); // Flush the buffer, in case we got any junk in there

    // Check whether we should add our standard commands, as soon as a stream connection is established (Cause we're making stream output here)
    if ( shouldAddStandardCommands() )
    {
        addAllStandardCommands();
        setAddStandardCommands( false ); // This is to prevent further insert-attempts, if init() gets called again
    }

    setDefaultCallback( defaultCommand );

    sendInfo( "Device with ID '" + getId() + "' is ready." );
}

void StreamCommander::setStreamInstance( Stream * streamInstance )
{
    if ( streamInstance != nullptr )
    {
        this->streamInstance = streamInstance;
    }
    else // If a nullptr has been passed, try to fall back to our standard "Serial" instance.
    {
        this->streamInstance = &Serial;
    }
}

Stream * StreamCommander::getStreamInstance()
{
    return this->streamInstance;
}

void StreamCommander::setActive( bool active )
{
    if ( isActive() != active ) // Only set our active-status if it's differing
    {
        this->active = active;

        sendIsActive();
    }
}

bool StreamCommander::isActive()
{
    return this->active;
}

void StreamCommander::setCommandDelimiter( char commandDelimiter )
{
    this->commandDelimiter = commandDelimiter;
}

char StreamCommander::getCommandDelimiter()
{
    return this->commandDelimiter;
}

void StreamCommander::setMessageDelimiter( char messageDelimiter )
{
    this->messageDelimiter = messageDelimiter;
}

char StreamCommander::getMessageDelimiter()
{
    return this->messageDelimiter;
}

void StreamCommander::setEchoCommands( bool echoCommands )
{
    this->echoCommands = echoCommands;
}

bool StreamCommander::shouldEchoCommands()
{
    return this->echoCommands;
}

void StreamCommander::setAddStandardCommands( bool addStandardCommands )
{
    this->addStandardCommands = addStandardCommands;
}

bool StreamCommander::shouldAddStandardCommands()
{
    return this->addStandardCommands;
}

void StreamCommander::setStreamBufferTimeout( long streamBufferTimeout )
{
    // Check for Input-Errors
    if ( streamBufferTimeout < 0 )
    {
        sendError( F("Timeout has to be >= 0.") );

        return;
    }

    getStreamInstance()->setTimeout( streamBufferTimeout );
    this->streamBufferTimeout = streamBufferTimeout;
}

long StreamCommander::getStreamBufferTimeout()
{
    return this->streamBufferTimeout;
}

void StreamCommander::saveIdToEEPROM( String id )
{
    // Since EEPROM.put can't handle strings, we have to convert it to a c_str
    char idBuffer[ID_MAX_LENGTH];
    id.toCharArray( idBuffer, ID_MAX_LENGTH );

    EEPROM.put( 0, idBuffer );
}

void StreamCommander::loadIdFromEEPROM()
{
    char id[ID_MAX_LENGTH];
    EEPROM.get( 0, id );

    setId( String( id ) );
}

void StreamCommander::setId( String id )
{
    // Check for Errors
    if ( id.length() > ID_MAX_LENGTH )
    {
        sendError( "ID '" + id + "' too long (ID_MAX_LENGTH = " + String( ID_MAX_LENGTH ) + ")." );

        return;
    }

    if ( id.equals( getId() ) )
    {
        sendResponse( "ID is already '" + id + "'." );

        return;
    }

    saveIdToEEPROM( id );
    this->id = id;
    sendId();
}

String StreamCommander::getId()
{
    return this->id;
}

void StreamCommander::setStatus( String status )
{
    this->status = status;
}

void StreamCommander::updateStatus( String status )
{
    // Only update our status if it has actually changed
    if ( !getStatus().equals( status ) )
    {
        setStatus( status );

        // Only send a Status-Update if our device is set active
        if ( isActive() )
        {
            sendStatus();
        }
    }
}

String StreamCommander::getStatus()
{
    return this->status;
}

void StreamCommander::addCommand( String commandName, CommandCallbackFunction commandCallback )
{
    // Check for Input-Errors
    if ( commandName.length() == 0 )
    {
        sendError( F("Command-Name must not be empty.") );

        return;
    }

    if ( commandCallback == nullptr )
    {
        sendError( F("Command-Callbackfunction must not be empty.") );

        return;
    }

    int currentCommandNum = getNumCommands();
    bool commandFound = false;
    CommandContainer * container = getCommandContainer( commandName );

    // Check if the command has already been added
    if ( container != nullptr )
    {
        commandFound = true;
        currentCommandNum = getCommandContainerNum( commandName ); // Set the currentCommandNum to the array-position where it has been found
    }

    // If the command has not been added yet, incease the array size and create a pointer to our commandName. Set the Callback in the next step.
    // If it has already been added, just replace the old callback with the new one in the next step.
    if ( !commandFound ) 
    {
        commands = (CommandContainer*) realloc( commands, ( currentCommandNum + 1 ) * sizeof( CommandContainer ) );
        incrementNumCommands();

        // Create a pointer to our command-name. On destruction of the corresponding CommandContainer, it will get deleted.
        String * commandNamePointer = new String( commandName );
        commands[currentCommandNum].command = commandNamePointer;
    }
    else
    {
        sendInfo( "Command '" + commandName + "' already found. Replacing with new command." );
    }

    // Set the Callback-Function
    commands[currentCommandNum].callbackFunction = commandCallback;
}

StreamCommander::CommandContainer * StreamCommander::getCommandContainer( String command )
{
    for ( int i = 0; i < getNumCommands(); i++ )
    {
        if ( commands[i].command->equals( command ) )
        {
            return &commands[i];
        }
    }

    return nullptr;
}

int StreamCommander::getCommandContainerNum( String command )
{
    for ( int i = 0; i < getNumCommands(); i++ )
    {
        if ( commands[i].command->equals( command ) )
        {
            return i;
        }
    }

    return -1;
}

void StreamCommander::deleteCommands()
{
    delete[] commands;
    setNumCommands( 0 );
}

void StreamCommander::setNumCommands( int numCommands )
{
    this->numCommands = numCommands;
}

void StreamCommander::incrementNumCommands()
{
    this->numCommands++;
}

int StreamCommander::getNumCommands()
{
    return this->numCommands;
}

String StreamCommander::getCommandList()
{
    String commandList = "";
    String commandSeparator = ", ";

    for ( int i = 0; i < getNumCommands(); i++ )
    {
        commandList = commandList + *(commands[i].command) + commandSeparator;
    }

    // Remove the last command separator
    unsigned int listLength = commandList.length();
    unsigned int separatorLength = commandSeparator.length();

    if ( listLength > 0 )
    {
        commandList.remove( listLength - separatorLength, separatorLength );
    }

    return commandList;
}

void StreamCommander::setDefaultCallback( DefaultCallbackFunction defaultCallbackFunction )
{
    // Check for Input-Errors
    if ( defaultCallbackFunction == nullptr )
    {
        sendError( F("Default-Callbackfunction must not be empty.") );

        return;
    }

    this->defaultCallbackFunction = defaultCallbackFunction;
}

StreamCommander::DefaultCallbackFunction StreamCommander::getDefaultCallback()
{
    return this->defaultCallbackFunction;
}

void StreamCommander::fetchCommand()
{
    Stream * streamInstance = getStreamInstance();
    String commandBuffer = "";

    // Only execute when a command is available
    if ( streamInstance->available() )
    {
        commandBuffer = streamInstance->readString();
    }
    else
    {
        return;
    }

    // Check for the first CR or NL occuring in our string.
    int cr = commandBuffer.indexOf( COMMAND_EOL_CR );
    int nl = commandBuffer.indexOf( COMMAND_EOL_NL );
    int stringEnd = -1;

    if ( cr > 0 && nl > 0 )
    {
        stringEnd = min( cr, nl );
    }
    else if ( cr > 0 )
    {
        stringEnd = cr;
    }
    else if ( nl > 0 )
    {
        stringEnd = nl;
    }
    else // If there's no CR/NL at all or if it's the first character, return -> no valid command found
    {
        return;
    }

    // Parse command from buffer

    int commandEnd = commandBuffer.indexOf( getCommandDelimiter() );
    String command = "";
    String arguments = "";

    // If there is no command-delimiter, we can't parse any arguments
    if ( commandEnd == -1 )
    {
        // Set commandEnd to stringEnd, since our command ends there if no arguments are given
        commandEnd = stringEnd;
    }
    else
    {
        arguments = commandBuffer.substring( commandEnd + 1, stringEnd );
    }

    command = commandBuffer.substring( 0, commandEnd );

    // Send an Echo
    if ( shouldEchoCommands() )
    {
        if ( arguments.length() )
        {
            sendEcho( command + " " + arguments );
        }
        else
        {
            sendEcho( command );
        }
    }

    bool commandFound = false;

    // Try to find our Input-Command and execute it
    CommandContainer * container = getCommandContainer( command );

    // If a container for this command has been found, try to call the callback
    if ( container != nullptr )
    {
        commandFound = true;

        if ( container->callbackFunction != nullptr )
        {
            // Call our Callback-Function with the arguments and our object-instace
            container->callbackFunction( arguments, this );
        }
        else
        {
            sendError( F("Command-Callbackfunction is empty.") );
        }
    }

    if ( !commandFound )
    {
        getDefaultCallback()( command, arguments, this );
    }
}

void StreamCommander::sendMessage( String type, String content )
{
    getStreamInstance()->println( type + getMessageDelimiter() + content );
}

void StreamCommander::sendResponse( String response )
{
    sendMessage( MessageType::RESPONSE, response );
}

void StreamCommander::sendInfo( String info )
{
    sendMessage( MessageType::INFO, info );
}

void StreamCommander::sendError( String error )
{
    sendMessage( MessageType::ERROR, error );
}

void StreamCommander::sendPing()
{
    sendMessage( MessageType::PING, "reply" );
}

void StreamCommander::sendStatus()
{
    sendMessage( MessageType::STATUS, getStatus() );
}

void StreamCommander::sendId()
{
    sendMessage( MessageType::ID, getId() );
}

void StreamCommander::sendIsActive()
{
    sendMessage( MessageType::ACTIVE, String( isActive() ) );
}

void StreamCommander::sendEcho( String echo )
{
    sendMessage( MessageType::ECHO, echo );
}

void StreamCommander::sendCommands()
{
    sendMessage( MessageType::COMMANDS, getCommandList() );
}

void StreamCommander::commandActivate( String arguments, StreamCommander * instance )
{
    instance->setActive( true );
}

void StreamCommander::commandDeactivate( String arguments, StreamCommander * instance )
{
    instance->setActive( false );
}

void StreamCommander::commandIsActive( String arguments, StreamCommander * instance )
{
    instance->sendIsActive();
}

void StreamCommander::commandSetEcho( String arguments, StreamCommander * instance )
{
    arguments.trim();

    if ( arguments.equals( F("on") ) )
    {
        instance->setEchoCommands( true );
    }
    else if ( arguments.equals( F("off") ) )
    {
        instance->setEchoCommands( false );
    }
}

void StreamCommander::commandSetId( String id, StreamCommander * instance )
{
    id.trim();
    instance->setId( id );
}

void StreamCommander::commandGetId( String arguments, StreamCommander * instance )
{
    instance->sendId();
}

void StreamCommander::commandPing( String arguments, StreamCommander * instance )
{
    instance->sendPing();
}

void StreamCommander::commandGetStatus( String arguments, StreamCommander * instance )
{
    instance->sendStatus();
}

void StreamCommander::commandListCommands( String arguments, StreamCommander * instance )
{
    instance->sendCommands();
}

void StreamCommander::addAllStandardCommands()
{
    addCommand( COMMAND_ACTIVATE, commandActivate );
    addCommand( COMMAND_DEACTIVATE, commandDeactivate );
    addCommand( COMMAND_ISACTIVE, commandIsActive );
    addCommand( COMMAND_SETECHO, commandSetEcho );
    addCommand( COMMAND_SETID, commandSetId );
    addCommand( COMMAND_GETID, commandGetId );
    addCommand( COMMAND_PING, commandPing );
    addCommand( COMMAND_GETSTATUS, commandGetStatus );
    addCommand( COMMAND_LISTCOMMANDS, commandListCommands );
}

void StreamCommander::defaultCommand( String command, String arguments, StreamCommander * instance )
{
    instance->sendResponse( "Command '" + command + "' not registered." );
}
