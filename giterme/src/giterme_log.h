#ifndef GITERME_LOG_H
	#define GITERME_LOG_H

	#ifdef _DEBUG
	#include <stdio.h>
	#include <stdarg.h>
	#include <stdlib.h>
	#include <string.h>
	#ifndef WIN32_LEAN_AND_MEAN
		#define WIN32_LEAN_AND_MEAN
		#include <windows.h>
	#endif
	#endif

	#ifndef _FILE_DEFINED
		struct FILE;
	#endif

	#ifndef _VA_LIST
		typedef char * va_list;
	#endif

	void LoggerInit(void);
	static void _Log(
		FILE *stream,
		const char *tag,
		const char *file,
		const char *func,
		int line,
		const char *format,
		va_list args);
	void _LogInfo(const char *file, const char *func, int line, const char *format, ...);
	void _LogError(const char *file, const char *func, int line, const char *format, ...);

	#define	__LOG_PARAMS__ __FILENAME__, __func__, __LINE__
	#define LogInfo(format, ...) _LogInfo(__LOG_PARAMS__, format, ##__VA_ARGS__)
	#define LogError(format, ...) _LogError(__LOG_PARAMS__, format, ##__VA_ARGS__)
#endif

#ifdef LOGGER_IMPL
	#ifdef _DEBUG
		#define LINE_LENGTH 512

		void LoggerInit(void)
		{
			FILE *consoleStream = nullptr;
			AllocConsole();
			freopen_s(&consoleStream, "CONIN$", "r", stdin);
			freopen_s(&consoleStream, "CONOUT$", "w", stdout);
			freopen_s(&consoleStream, "CONOUT$", "w", stderr);
		}

		static void _Log(
			FILE *stream,
			const char *tag,
			const char *file,
			const char *func,
			const int line,
			const char *format,
			va_list args)
		{
			char logString[LINE_LENGTH] = "\0";
			sprintf_s(logString, LINE_LENGTH, "[ %s ][ %s ][ %s : %s : %d ] %s\n\0", tag, __TIME__, file, func, line, format);
			vfprintf(stream, logString, args);
		}

		void _LogInfo(const char *file, const char *func, int line, const char *format, ...)
		{
			#ifdef LOGGER_LEVEL_INFO
			va_list args;
			va_start(args, format);
			_Log(stdout, "INFO", file, func, line, format, args);
			va_end(args);
			#endif
		}

		void _LogError(const char *file, const char *func, int line, const char *format, ...)
		{
			#ifdef LOGGER_LEVEL_ERROR
			va_list args;
			va_start(args, format);
			_Log(stderr, "ERROR", file, func, line, format, args);
			va_end(args);
			#endif
		}
	#else
		void LoggerInit(void) {}
		static void _Log(
			FILE *stream,
			const char *tag,
			const char *file,
			const char *func,
			const int line,
			const char *format,
			va_list args) {}
		void _LogInfo(const char *file, const char *func, int line, const char *format, ...) {}
		void _LogError(const char *file, const char *func, int line, const char *format, ...) {}
		#define LogInfo(format, ...)
		#define LogError(format, ...)
	#endif
#endif