//----------------------------------------------------------------------------------
// File:            libs\jni\nv_log\nv_log.cpp
// Samples Version: Android NVIDIA samples 2 
// Email:           tegradev@nvidia.com
// Forum:           http://developer.nvidia.com/tegra/forums/tegra-forums/android-development
//
// Copyright 2009-2010 NVIDIA® Corporation 
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//----------------------------------------------------------------------------------
//==============================================================================
//    File:        nv_log.cpp
//
//    Copyright 2008, NVIDIA Corporation.
//    ALL RIGHTS RESERVED
//==============================================================================

#include "../nv_global.h"

#include "nv_log.h"

//==============================================================================
//    Includes
//==============================================================================
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

//==============================================================================
//    C Linkage
//==============================================================================
extern "C"
{

//==============================================================================
//    Constants
//==============================================================================
#define NVLOG_MESSAGELENGTH    (16*1024)

//==============================================================================
//    Function Pointers and Fields
//==============================================================================
long                g_NVLogMask =    NVLOGLEVEL_ALL;
void*                g_NVLogFile =    NULL;

// Core Functions
TNVLogFunctionEx    NVLog =            NVDefaultLog;
TNVMessageFunction    NVLogMessage =    NVDefaultLogMessage;
TNVOutputFunction    NVLogOutput =    NVDefaultLogOutput;

// Convenience Functions
TNVLogFunction        NVLogDebug =    NVDefaultLogDebug;
TNVLogFunction        NVLogInfo =        NVDefaultLogInfo;
TNVLogFunction        NVLogWarn =        NVDefaultLogWarn;
TNVLogFunction        NVLogError =    NVDefaultLogError;
TNVLogFunction        NVLogFatal =    NVDefaultLogFatal;

//==============================================================================
//    Macros
//==============================================================================
/// Formatting macro removing code duplication when processing variadic arguments
#define NVLOG_FORMATDETAILS( level )                                \
    va_list        theVAList;                                            \
    char        theFormattedDetails[NVLOG_MESSAGELENGTH] = "";        \
                                                                    \
    if ( 0 == ( (level) & g_NVLogMask ) )                            \
        return;                                                        \
                                                                    \
    va_start( theVAList, inDetails );                                \
    vsprintf( theFormattedDetails, inDetails, theVAList )
//    vsprintf_s( theFormattedDetails, sizeof(theFormattedDetails), inDetails, theVAList )

//==============================================================================
//    Functions
//==============================================================================

//==============================================================================
/**
 *    Set a file path to enable logging to disk.
 *    @param    inFilePath    log file name such as "c:\log.txt"
 */
void NVLogFile( const char* inFilePath )
{
    if ( g_NVLogFile )
        fclose( (FILE*)g_NVLogFile );
        
    if ( inFilePath )
        g_NVLogFile = fopen( inFilePath, "w" );    
}

//==============================================================================
/**
 *    Selectively enable or disable a log level.
 *    @param    inLevel        Log level such as NVLOGLEVEL_WARN
 *    @param    inEnable    true to enable, false to disable 
 */
void NVLogEnable( ENVLogLevel inLevel, NvBool inEnable )
{
    if ( inEnable )
        g_NVLogMask |= inLevel;
    else
        g_NVLogMask &= ~inLevel;
}

//==============================================================================
/**
 *    Default entry method for all logging calls.
 *
 *    Collapse the variadic arguments and format them into a single argument for
 *    the message function to process.
 *
 *    @param inLevel        Severity level of the debug entry ranging from DEBUG(low) to FATAL(high)
 *    @param inTag        Short string describing the subsystem being logged allowing system level filtering
 *    @param inMessage    Simple string describing the log entry
 *    @param inDetails    Longer detailed string providing more information and printf-style formatting
 *    @param ...            Optional variadic arguments formetted into inDetails.
 */
void NVDefaultLog( ENVLogLevel inLevel, const char* inTag, const char* inMessage, const char* inDetails, ... )
{
    // Exit immedately if this type of message is disabled
    // Collapse the variadic arguments using inDetails as format
    NVLOG_FORMATDETAILS( inLevel );

    // Pass on the arguments to the non-variadic message function     
    NVLogMessage( inLevel, inTag, inMessage ,theFormattedDetails );
}

//==============================================================================
/**
 *    Helper method translating log level to its descripive string.
 */
const char* NVLogLevelString( ENVLogLevel inLevel )
{
    static const char    theLevelString[][6] = { "DEBUG", "TRACE", "INFO", "WARN", "ERROR", "FATAL", "???" };    
    switch ( inLevel )
    {
        case NVLOGLEVEL_DEBUG:    return theLevelString[0];
        case NVLOGLEVEL_TRACE:    return theLevelString[1];
        case NVLOGLEVEL_INFO:    return theLevelString[2];
        case NVLOGLEVEL_WARN:    return theLevelString[3];
        case NVLOGLEVEL_ERROR:    return theLevelString[4];
        case NVLOGLEVEL_FATAL:    return theLevelString[5];
        case NVLOGLEVEL_ALL:
        default:                return theLevelString[6];
    }
}

//==============================================================================
/**
 *    Default exit method for all logging calls.
 *
 *    Concatenate all the arguments into a string and forward that string to
 *    printf and OutputDebugStringA on Win32.
 *
 *    @param inLevel        Severity level of the debug entry ranging from DEBUG(low) to FATAL(high)
 *    @param inTag        Short string describing the subsystem being logged allowing system level filtering
 *    @param inMessage    Simple string describing the log entry
 *    @param inDetails    Longer detailed string providing more information and printf-style formatting
 *    @return The formatted log message string
 */
char* NVDefaultLogMessage( ENVLogLevel inLevel, const char* inTag, const char* inMessage, const char* inDetails )
{
    static char    s_FormattedMesage[NVLOG_MESSAGELENGTH] = "";
    size_t theLength = strlen( inTag );
    
    if ( theLength > NVLOG_MESSAGELENGTH - 100 )
        inTag = "(Tag too big)";
    theLength += strlen( inMessage );
    if ( theLength > NVLOG_MESSAGELENGTH - 100 )
        inMessage = "(Message too long)";
    theLength += strlen( inDetails );
    if ( theLength > NVLOG_MESSAGELENGTH - 100 )
        inDetails = "(Details too long)";
            
    // Format the log arguments to a single string
    sprintf( s_FormattedMesage, "%s(%s): %s - %s\n", NVLogLevelString( inLevel ), inTag, inMessage, inDetails );
//    sprintf_s( theFormattedMesage, sizeof(theFormattedMesage), "%d-%s: %s, %s", theLevel, inTag, inMessage, inDetails );
    
    NVLogOutput( s_FormattedMesage );
    
    return s_FormattedMesage;
}

//==============================================================================
/**
 *    XXX
 */
void NVDefaultLogOutput( const char* inMessage )
{
    printf( inMessage );

    // Log to the file if open
    if ( g_NVLogFile )
    {
        fputs( inMessage, (FILE*)g_NVLogFile );
        fflush( (FILE*)g_NVLogFile );
    }

}

//==============================================================================
/**
 *    Convenience function for logging DEBUG entries
 */
void NVDefaultLogDebug( const char* inMessage, const char* inDetails, ... )
{
    NVLOG_FORMATDETAILS( NVLOGLEVEL_DEBUG );
    NVLogMessage( NVLOGLEVEL_DEBUG, "Log", inMessage ,theFormattedDetails );    
}

//==============================================================================
/**
 *    Convenience function for logging DEBUG entries
 */
void NVDefaultLogTrace( const char* inMessage, const char* inDetails, ... )
{
    NVLOG_FORMATDETAILS( NVLOGLEVEL_TRACE );
    NVLogMessage( NVLOGLEVEL_TRACE, "Log", inMessage ,theFormattedDetails );    
}

//==============================================================================
/**
 *    Convenience function for logging INFO entries
 */
void NVDefaultLogInfo( const char* inMessage, const char* inDetails, ... )
{
    NVLOG_FORMATDETAILS( NVLOGLEVEL_INFO );
    NVLogMessage( NVLOGLEVEL_INFO, "Log", inMessage ,theFormattedDetails );    
}

//==============================================================================
/**
 *    Convenience function for logging WARN entries
 */
void NVDefaultLogWarn( const char* inMessage, const char* inDetails, ... )
{
    NVLOG_FORMATDETAILS( NVLOGLEVEL_WARN );
    NVLogMessage( NVLOGLEVEL_WARN, "Log", inMessage ,theFormattedDetails );    
}

//==============================================================================
/**
 *    Convenience function for logging ERROR entries
 */
void NVDefaultLogError( const char* inMessage, const char* inDetails, ... )
{
    NVLOG_FORMATDETAILS( NVLOGLEVEL_ERROR );
    NVLogMessage( NVLOGLEVEL_ERROR, "Log", inMessage ,theFormattedDetails );    
}

//==============================================================================
/**
 *    Convenience function for logging FATAL entries
 */
void NVDefaultLogFatal( const char* inMessage, const char* inDetails, ... )
{
    NVLOG_FORMATDETAILS( NVLOGLEVEL_FATAL );
    NVLogMessage( NVLOGLEVEL_FATAL, "Log", inMessage ,theFormattedDetails );    
}

} // extern "C"
