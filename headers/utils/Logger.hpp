#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <iostream>
#include <cstdarg>

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
		if (logType < logLevel) {
			return;
		}

		char logSym = 'I';
		if (logType == LogType::LOG_TYPE_WARNING) {
			logSym = 'W';
		}
		else if (logType == LogType::LOG_TYPE_ERROR) {
			logSym = 'E';
		}

		fprintf(fileOut, "[%c] %s.%d: ", logSym, fileBaseName(filename).c_str(), line);

		va_list args;
		va_start(args, format);
		vfprintf(fileOut, format, args);
		va_end(args);

		fputc('\n', fileOut);
	}
};

}

#ifdef NDEBUG
#define LOG_I(format, ...)
#define LOG_W(format, ...)
#define LOG_E(format, ...)
#else
#define LOG_I(format, ...) Utils::Logger::Log(__FILE__, __LINE__, Utils::LogType::LOG_TYPE_INFO,	format, __VA_ARGS__)
#define LOG_W(format, ...) Utils::Logger::Log(__FILE__, __LINE__, Utils::LogType::LOG_TYPE_WARNING,	format, __VA_ARGS__)
#define LOG_E(format, ...) Utils::Logger::Log(__FILE__, __LINE__, Utils::LogType::LOG_TYPE_ERROR,	format, __VA_ARGS__)
#endif

#endif
