#include <iostream>

#include "FileManager.hpp"

#ifdef __ANDROID__
#include <unistd.h>
#else
#include <filesystem>
#endif

namespace FileSystem {

FileManager* FileManager::instancePtr = nullptr;

void FileManager::registerProtocol(const std::string& name, const std::string& dirpath) {
	protocolsDirs_.insert(std::pair<std::string, std::string>(name, dirpath));
}

const std::string FileManager::getAbsolutePath(const std::string& filename) const {
    if (filename.empty()) {
        return getCurrentDirectory();
    }

#ifdef _WIN32
	if (filename.length() > 2) {
		if ((filename[0] == 'C' || filename[0] == 'D') && filename[1] == ':') {
			return filename;
		}
	}
#else
	if (filename[0] == '/') {
		return filename;
	}
#endif

    size_t pos = filename.find_first_of(":");
	if (pos != std::string::npos && (pos + 2) != filename.length()) {
		std::string protocol = filename.substr(0, pos);
		std::string relpath = filename.substr(pos + 2);

		if (auto it = protocolsDirs_.find(protocol); it != protocolsDirs_.end()) {
			return it->second + relpath;
		}
	}

	return getCurrentDirectory() + "/" + filename;
}

const std::string FileManager::getCurrentDirectory() const {
#ifdef __ANDROID__
    char buff[FILENAME_MAX];
    getcwd(buff, FILENAME_MAX);
    return std::string(buff);
#else
    return std::filesystem::current_path().string();
#endif
}

}