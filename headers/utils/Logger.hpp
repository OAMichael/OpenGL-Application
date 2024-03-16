#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <iostream>
#include <cstdarg>

#ifdef __ANDROID__
#include <android/log.h>
#endif

namespace Utils {

static inline std::string fileBaseName(const std::string& path)
{
	return path.substr(path.find_last_of("/\\") + 1);
}

static inline std::string fileBaseDir(const std::string& path)
{
	return path.substr(*path.begin() - path[0], path.find_last_of("/\\") + 1);
}

enum LogType {
	LOG_TYPE_INFO = 0,
	LOG_TYPE_WARNING = 1,
	LOG_TYPE_ERROR = 2
};

enum LogLevel {
	LOG_LEVEL_VERBOSE = 0,
	LOG_LEVEL_WARN_ERROR = 1,
	LOG_LEVEL_ONLY_ERROR = 2,
	LOG_LEVEL_NONE = 3
};

class Logger {
private:
	static FILE* fileOut;
	static LogLevel logLevel;

public:
	static void setLogLevel(LogLevel ll) {
		logLevel = ll;
	}

	static void setFileout(FILE* f) {
		fileOut = f;
	}

	static void Log(const char* filename, int line, LogType logType, const char* format, ...) {
		if (logType < static_cast<LogType>(logLevel)) {
			return;
		}

#ifdef __ANDROID__
		android_LogPriority prio = ANDROID_LOG_UNKNOWN;
		switch (logType) {
			case LOG_TYPE_INFO: {
				prio = ANDROID_LOG_INFO;
				break;
			}
			case LOG_TYPE_WARNING: {
				prio = ANDROID_LOG_WARN;
				break;
			}
			case LOG_TYPE_ERROR: {
				prio = ANDROID_LOG_ERROR;
				break;
			}
			default: {
				break;
			}
		}

		va_list args;
		va_start(args, format);
		__android_log_vprint(prio, "native-activity", format, args);
		va_end(args);
#else

		const char* reset = "\033[0m";
		const char* yellow = "\033[33m";
		const char* red = "\033[31m";

		char logSym = 'I';
		if (logType == LogType::LOG_TYPE_WARNING) {
			logSym = 'W';
			fprintf(fileOut, "%s", yellow);
		}
		else if (logType == LogType::LOG_TYPE_ERROR) {
			logSym = 'E';
			fprintf(fileOut, "%s", red);
		}

		fprintf(fileOut, "[%c] %s.%d: ", logSym, fileBaseName(filename).c_str(), line);

		va_list args;
		va_start(args, format);
		vfprintf(fileOut, format, args);
		va_end(args);

		fprintf(fileOut, "%s", reset);
		fputc('\n', fileOut);
#endif
	}
};

}

#ifdef NDEBUG
#define LOG_I(format, ...)
#define LOG_W(format, ...)
#define LOG_E(format, ...)
#else
#define LOG_I(format, ...) Utils::Logger::Log(__FILE__, __LINE__, Utils::LogType::LOG_TYPE_INFO,	format, ##__VA_ARGS__)
#define LOG_W(format, ...) Utils::Logger::Log(__FILE__, __LINE__, Utils::LogType::LOG_TYPE_WARNING,	format, ##__VA_ARGS__)
#define LOG_E(format, ...) Utils::Logger::Log(__FILE__, __LINE__, Utils::LogType::LOG_TYPE_ERROR,	format, ##__VA_ARGS__)
#endif

#endif
