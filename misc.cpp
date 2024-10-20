//////////////////////////////////////////////////////////////////////////
//
// pgAgent - PostgreSQL Tools
//
// Copyright (C) 2002 - 2021, The pgAdmin Development Team
// This software is released under the PostgreSQL Licence
//
// misc.cpp - misc functions
//
//////////////////////////////////////////////////////////////////////////

#include "pgAgent.h"
#include "connection.h"

#include <cstring>
#include <algorithm>
#include <string>

#if !_WIN32
#include <unistd.h>
#include <stdlib.h>
#endif

#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX 255
#endif // !HOST_NAME_MAX

#define APPVERSION_STR PGAGENT_VERSION

// In unix.c or win32.c
void usage(const std::string &executable);

std::string getArg(int &argc, char **&argv)
{
	std::string res;

	if (argv[0][2])
		return (argv[0] + 2);

	if (argc >= 1)
	{
		argc--;
		argv++;

		return argv[0];
	}

	// very bad!
	LogMessage("Invalid command line argument", LOG_ERROR);

	return res;
}

void printVersion()
{
	printf("PostgreSQL Scheduling Agent\n");
	printf("Version: %s\n", APPVERSION_STR);
}

void setOptions(int argc, char **argv, const std::string &executable)
{
	while (argc-- > 0)
	{
		if (argv[0][0] == '-')
		{
			switch (argv[0][1])
			{
				case 't':
				{
					int val = atoi((const char*)getArg(argc, argv).c_str());
					if (val > 0)
						shortWait = val;
					break;
				}
				case 'r':
				{
					int val = atoi((const char*)getArg(argc, argv).c_str());
					if (val >= 10)
						longWait = val;
					break;
				}
				case 'l':
				{
					int val = atoi((const char*)getArg(argc, argv).c_str());
					if (val >= 0 && val <= 2)
						minLogLevel = val;
					break;
				}
				case 'v':
				{
					printVersion();
					exit(0);
				}
#if !_WIN32
				case 'f':
				{
					runInForeground = true;
					break;
				}
				case 's':
				{
					logFile = getArg(argc, argv);
					break;
				}
#endif
				default:
				{
					usage(executable);
					exit(1);
				}
			}
		}
		else
		{
			if (!connectString.empty())
				connectString += " ";
			connectString += *argv;
			if (**argv == '"')
				connectString = connectString.substr(1, connectString.length() - 2);
		}
		argv++;
	}
}


void WaitAWhile(const bool waitLong)
{
	int count;
	if (waitLong)
		count = longWait;
	else
		count = shortWait;

	while (count--)
	{
#ifdef WIN32
		CheckForInterrupt();
		Sleep(1000);
#else
		sleep(1);
#endif
	}
}

std::string NumToStr(const long l)
{
	return std::to_string(l);
}

#if _WIN32
// This function is used to convert const std::str to std::wstring.
std::wstring s2ws(const std::string &nstr)
{
	const char* st = nstr.c_str();
	size_t length = nstr.length();
	if (0 == length || nullptr == st) return std::wstring();
	auto size_needed = MultiByteToWideChar(CP_UTF8, 0, st, static_cast<int> (length), nullptr, 0);
	if (0 == size_needed) return std::wstring();
	auto res = std::wstring();
	res.resize(size_needed);
	auto buf = std::addressof(res.front());
	int chars_copied = MultiByteToWideChar(CP_UTF8, 0, st, static_cast<int> (length), buf, size_needed);
	if (chars_copied != size_needed) return std::wstring();
	return res;
}

// This function is used to convert std::wstring to std::str
std::string ws2s(const std::wstring& wstr)
{
	const wchar_t* wbuf = wstr.c_str();
	size_t length = wstr.length();
	if (0 == length) return std::string();
	int size_needed = WideCharToMultiByte(CP_UTF8, 0, wbuf, static_cast<int> (length), nullptr, 0, nullptr, nullptr);
	if (0 == size_needed) return std::string();
	auto res = std::string();
	res.resize(size_needed);
	auto buf = std::addressof(res.front());
	int bytes_copied = WideCharToMultiByte(CP_UTF8, 0, wbuf, static_cast<int> (length), buf, size_needed, nullptr, nullptr);
	if (bytes_copied != size_needed) return std::string();
	return res;
}
#endif

// Below function will generate random string of given character.
std::string generateRandomString(size_t length)
{
	char *str = new char[length];
	size_t i = 0;
	int r;

	str[length - 1] = '\0';
	srand(static_cast<unsigned int>(time(NULL)));

	for(i = 0; i < length - 1; ++i)
	{
		for(;;)
		{
			// interval between 'A' and 'z'
			r = rand() % 57 + 65;
			if((r >= 65 && r <= 90) || (r >= 97 && r <= 122))
			{
				str[i] = (char)r;
				break;
			}
		}
	}

	std::string result(str);

	if (str != NULL)
	{
		delete []str;
		str = NULL;
	}

	return result;
}

std::string getTemporaryDirectoryPath()
{

#if _WIN32
    std::wstring tmp_dir;

    wchar_t wcharPath[MAX_PATH];

    if (GetTempPathW(MAX_PATH, wcharPath))
		{
        tmp_dir = wcharPath;

				return ws2s(tmp_dir);
		}
    return "";
#else
    // Read this environment variable (TMPDIR, TMP, TEMP, TEMPDIR) and if not found then use "/tmp"
    std::string tmp_dir = "/tmp";
    const char *s_tmp = getenv("TMPDIR");

    if (s_tmp != NULL)
        return s_tmp;

		s_tmp = getenv("TMP");
		if (s_tmp != NULL)
			return s_tmp;

		s_tmp = getenv("TEMP");
		if (s_tmp != NULL)
			return s_tmp;

		s_tmp = getenv("TEMPDIR");
		if (s_tmp != NULL)
			return s_tmp;

		return tmp_dir;
#endif
}


std::string& str_replace_all(std::string& str, const std::string& snippet, const std::string& replacement)
{
	if (snippet.empty())
	{
		return str;
	}
	auto pos = std::string::npos;
	while (std::string::npos != (pos = str.find(snippet)))
	{
		str.replace(pos, snippet.length(), replacement);
	}
	return str;
}

std::string str_trim(const std::string& s)
{
	auto wsfront = std::find_if_not(s.begin(), s.end(), [](int c) {
		return std::isspace(c);
	});
	return std::string(wsfront, std::find_if_not(s.rbegin(), std::string::const_reverse_iterator(wsfront), [](int c) {
		return std::isspace(c);
	}).base());
}

std::string	get_host_name()
{
#ifdef _WIN32
	std::wstring wbuf = std::wstring();
	wbuf.resize(MAX_COMPUTERNAME_LENGTH + 1);
	DWORD len = static_cast<DWORD>(wbuf.length());
	if (GetComputerNameW(wbuf.data(), &len))
	{
		wbuf.resize(len);
		return ws2s(wbuf);
	}
	else
	{
		return "UNKNOWN_HOSTNAME";
	}
#else //!_WIN32
	std::string buf;
	buf.resize(HOST_NAME_MAX + 1);
	if (!gethostname(buf.data(), buf.length())) {
		return std::string(buf.data(), strlen(buf.data()));
	}
	else {
		return "UNKNOWN_HOSTNAME";
	}
#endif //_WIN32
}