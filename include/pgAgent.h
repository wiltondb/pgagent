//////////////////////////////////////////////////////////////////////////
//
// pgAgent - PostgreSQL Tools
//
// Copyright (C) 2002 - 2021, The pgAdmin Development Team
// This software is released under the PostgreSQL Licence
//
// pgAgent.h - main include
//
//////////////////////////////////////////////////////////////////////////


#ifndef PGAGENT_H
#define PGAGENT_H

#if _MSC_VER
#ifndef UNICODE
#define UNICODE
#endif // UNICODE
#ifndef _UNICODE
#define _UNICODE
#endif // _UNICODE
#ifndef NOMINMAX
#define NOMINMAX
#endif // NOMINMAX
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include "misc.h"
#include "connection.h"
#include "job.h"

extern long        longWait;
extern long        shortWait;
extern long        minLogLevel;
extern std::string connectString;
extern std::string backendPid;

#if !_WIN32
extern bool        runInForeground;
extern std::string logFile;
#endif

// Log levels
enum
{
	LOG_ERROR = 0,
	LOG_WARNING,
	LOG_DEBUG,
	// NOTE:
	//     "STARTUP" will be used to log messages for any LogLevel
	//     Use it for logging database connection errors which we
	//     don't want to abort the whole shebang.
	LOG_STARTUP = 15
};

// Prototypes
void LogMessage(const std::string &msg, const int &level);
void MainLoop();

#if _WIN32
void CheckForInterrupt();
HANDLE win32_popen_r(const WCHAR *command, HANDLE &handle);
#endif

#endif // PGAGENT_H

