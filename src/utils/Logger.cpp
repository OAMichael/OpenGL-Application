#include "Logger.hpp"

FILE* Utils::Logger::fileOut = stdout;
Utils::LogLevel Utils::Logger::logLevel = LogLevel::LOG_LEVEL_VERBOSE;
