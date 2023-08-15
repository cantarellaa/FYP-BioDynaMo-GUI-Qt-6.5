#include "biodynamodirectory.h"
#include "appsettings.h"

#include <QMetaEnum>
#include <QSettings>

BiodynamoDirectory::BiodynamoDirectory(DirectoryType type, const QString &newPath)
    : QDir(newPath)
    , type(type)
{
    if (!isValid()) {
        throw std::invalid_argument(errorMessage().toStdString());
    }
}

BiodynamoDirectory::BiodynamoDirectory(DirectoryType type, const QDir &dir)
    : QDir(dir), type(type) {
  if (!isValid()) {
    throw std::invalid_argument(errorMessage().toStdString());
  }
}

bool BiodynamoDirectory::isValid() {
  switch (type) {
  case DirectoryType::Build:
    return dirName().endsWith("build");
  case DirectoryType::New:
    return !exists(); // && fileExists("..");
  case DirectoryType::Platform:
    return fileExists("thisbdm.sh");
  case DirectoryType::Project:
    return fileExists("CMakeLists.txt");
  }
  return false;
}

BiodynamoDirectory BiodynamoDirectory::platformDirectory() {
  QSettings settings;

  return BiodynamoDirectory(DirectoryType::Platform,
                            settings
                                .value(AppSettings::enumToString(
                                    AppSettings::SettingsKeys::Platform_Path))
                                .toString());
}

BiodynamoDirectory BiodynamoDirectory::lastProjectDirectory() {
  QSettings settings;

  return BiodynamoDirectory(DirectoryType::Project,
                            settings.value(AppSettings::enumToString(AppSettings::SettingsKeys::Last_Project_Path)).toString());
}

BiodynamoDirectory BiodynamoDirectory::lastProjectBuildDirectory()
{
  return BiodynamoDirectory(DirectoryType::Build,
                            QSettings()
                                .value(AppSettings::enumToString(AppSettings::SettingsKeys::Last_Project_Path))
                                .toString()
                                .append("/build"));
}

QString BiodynamoDirectory::getCMakeListsTxtPath()
{
  switch (type) {
  case BiodynamoDirectory::DirectoryType::Project:
    return absoluteFilePath("CMakeLists.txt");
  case BiodynamoDirectory::DirectoryType::Build:
    return absoluteFilePath("../CMakeLists.txt");
  default:
    return "";
  }
}

QString BiodynamoDirectory::getWorkingDirectory()
{
  switch (type) {
  case BiodynamoDirectory::DirectoryType::New:
    return cleanPath(absolutePath().append("/.."));
  default:
    return cleanPath(absolutePath());
  }
}

bool BiodynamoDirectory::fileExists(const QString &fileName)
{
  return QFileInfo(absoluteFilePath(fileName)).exists();
}

QString BiodynamoDirectory::errorMessage() {
  switch (type) {
  case BiodynamoDirectory::DirectoryType::New:
    if (exists()) {
        return QString("Directory %1 must not exist").arg(absolutePath());
    }
    // no break
  case BiodynamoDirectory::DirectoryType::Build:
    return QString("Directory %1 must exist").arg(QFileInfo(absolutePath()).dir().absolutePath());
  case BiodynamoDirectory::DirectoryType::Platform:
    return QString("Directory %1 must contain file thisbdm.sh").arg(absolutePath());
  case BiodynamoDirectory::DirectoryType::Project:
    return QString("Directory %1 must contain file CMakeLists.txt").arg(absolutePath());
  default:
    return "";
  }
}
