#ifndef FILE_MANAGER_HPP
#define FILE_MANAGER_HPP

#include <unordered_map>
#include <string>

namespace FileSystem {

    class FileManager final {
    private:

        static FileManager* instancePtr;

        FileManager() {};

        std::string basedir_;
        std::unordered_map<std::string, std::string> protocolsDirs_;

    public:

        FileManager(const FileManager& obj) = delete;

        static FileManager* getInstance() {
            if (!instancePtr)
                instancePtr = new FileManager();

            return instancePtr;
        }

        inline void setBasedir(const std::string& basedir) { basedir_ = basedir; }
        const std::string getBasedir() const { return basedir_; }

        void registerProtocol(const std::string& name, const std::string& dirpath);
        const std::string getAbsolutePath(const std::string& filename) const;
        const std::string getCurrentDirectory() const;
    };
}

#endif