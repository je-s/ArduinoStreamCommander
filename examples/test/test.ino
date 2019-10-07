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

#include <StreamCommander.hpp>

// Switch to either test with SoftwareSerial or HardwareSerial
#define SOFTSERIAL 0

#if SOFTSERIAL
#include <SoftwareSerial.h>
SoftwareSerial softwareSerial( 10, 11 ); // RX, TX
StreamCommander commander = StreamCommander( &softwareSerial );
#else
StreamCommander commander = StreamCommander();
#endif

const String COMMAND_TEST = "test";
const String COMMAND_HELLO = "hello";
const String COMMAND_POINTER = "pointer";
const String COMMAND_LED = "led";

void setup()
{
    pinMode( LED_BUILTIN, OUTPUT );

    #if SOFTSERIAL
    softwareSerial.begin( 9600 );
    #else
    Serial.begin( 9600 );
    #endif

    commander.init();
    commander.setEchoCommands( true );
    commander.addCommand( COMMAND_TEST, cmdTest );
    commander.addCommand( COMMAND_HELLO, cmdHello );
    commander.addCommand( COMMAND_POINTER, cmdPointer );
    commander.addCommand( COMMAND_LED, cmdLed );
    commander.setDefaultCallback( cmdDefault );
}

void loop()
{
    commander.fetchCommand();
}

void cmdTest( String arguments, StreamCommander * instance )
{
    arguments.trim();
    commander.updateStatus( arguments );
}

void cmdHello( String arguments, StreamCommander * instance )
{
    commander.sendResponse( "Hi!" );
}

void cmdPointer( String arguments, StreamCommander * instance )
{
    instance->sendResponse( "Pointer address is 0x" + String( (unsigned int)instance, HEX )  );
}

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

void cmdDefault( String command, String arguments, StreamCommander * instance )
{
    instance->sendResponse( "Command '" + command + "' with arguments '" + arguments + "' not registered." );
}
