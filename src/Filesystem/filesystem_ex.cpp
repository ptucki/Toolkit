#include <filesystem>
#include <string_view>

#include "filesystem_ex.h"
#include <algorithm>
#include "string_ex.h"


Toolkit::Filesystem::Filesystem()
  : path_{ std::filesystem::current_path() }
{

}

Toolkit::Filesystem::Filesystem(const std::string& path_str)
  : path_{ path_str }
{

}

void Toolkit::Filesystem::LeaveDir()
{
  namespace fs = std::filesystem;

  path_ = path_.parent_path();
}

bool Toolkit::Filesystem::VisitDir(const std::string& dir_name)
{
  namespace fs = std::filesystem;

  path_ /= Utf8ToPath(dir_name);

  if (!fs::exists(path_) || !fs::is_directory(path_))
  {
    this->LeaveDir();
    return false;
  }

  return true;
}

Toolkit::Filesystem::StrList Toolkit::Filesystem::GetDirs()
{
  namespace fs = std::filesystem;

  StrList dirs;

  if (fs::exists(path_) && fs::is_directory(path_))
  {
    for (auto const& entry : fs::directory_iterator{ path_ })
    {
      if (fs::is_directory(entry))
      {
        dirs.push_back(PathToUtf8(entry.path().filename()));
      }
    }
  }

  return dirs;
}

Toolkit::Filesystem::StrList Toolkit::Filesystem::GetFiles()
{
  namespace fs = std::filesystem;

  StrList files;

  if (fs::exists(path_) && fs::is_directory(path_))
  {
    for (auto const& entry : fs::directory_iterator{ path_ })
    {
      if (!fs::is_directory(entry))
      {
        files.push_back(PathToUtf8(entry.path().filename()));
      }
    }
  }

  return files;
}

Toolkit::Filesystem::Entries Toolkit::Filesystem::GetEntries()
{
  namespace fs = std::filesystem;

  Entries entries;

  if (fs::exists(path_) && fs::is_directory(path_))
  {
    for (auto const& entry : fs::directory_iterator{ path_ })
    {
      if (!fs::is_directory(entry)) entries.files.push_back(PathToUtf8(entry.path().filename()));
      else entries.dirs.push_back(PathToUtf8(entry.path().filename()));
    }
  }

  return entries;
}

void Toolkit::Filesystem::SetPath(const std::string& path_str)
{
  path_ = Utf8ToPath(path_str);
}

bool Toolkit::Filesystem::PathExists(const std::string& path_str)
{
  auto path = Utf8ToPath(path_str);

  return std::filesystem::exists(path);
}

std::string Toolkit::Filesystem::PathToUtf8(const std::filesystem::path& path)
{
  return (const char*)path.generic_u8string().c_str();
}

std::filesystem::path Toolkit::Filesystem::Utf8ToPath(const std::string& path)
{
  return std::filesystem::path((const char8_t*) path.c_str());
}

std::string Toolkit::Filesystem::GetCurrentPath() const
{
  return PathToUtf8(path_);
}
