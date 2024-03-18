#ifndef FILESYSTEM_EX_H
#define FILESYSTEM_EX_H

#include <string>
#include <vector>
#include <filesystem>

namespace Toolkit {

  class Filesystem {
    using StrList = std::vector<std::string>;

  public:

    struct Entries {
      StrList dirs;
      StrList files;
    };

    Filesystem();
    Filesystem(const std::string& path_str);

    void LeaveDir();
    bool VisitDir(const std::string& dir_name);
    std::string GetCurrentPath() const;

    StrList GetDirs();
    StrList GetFiles();
    Entries GetEntries();

    void SetPath(const std::string& path_str);

    static bool PathExists(const std::string& path_str);

  private:

    static std::string PathToUtf8(const std::filesystem::path& path);
    static std::filesystem::path Utf8ToPath(const std::string& path);

    std::filesystem::path path_;
  };
}


#endif // !FILESYSTEM_EX_H
