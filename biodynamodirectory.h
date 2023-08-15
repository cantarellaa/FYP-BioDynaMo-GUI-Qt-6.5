#ifndef BIODYNAMODIRECTORY_H
#define BIODYNAMODIRECTORY_H

#include <QDir>

class BiodynamoDirectory : protected QDir
{
public:
    enum class DirectoryType : char { Platform, Project, Build, New };

    BiodynamoDirectory(DirectoryType type, const QString &newPath = QString());
    BiodynamoDirectory(DirectoryType type, const QDir &dir);
    BiodynamoDirectory(const QString &path = QString())
        : BiodynamoDirectory(DirectoryType::New, path){};
    BiodynamoDirectory(const QDir &dir)
        : BiodynamoDirectory(DirectoryType::New, dir){};

    using QDir::absoluteFilePath;
    using QDir::absolutePath;
    using QDir::dirName;
    using QDir::exists;
    using QDir::filePath;
    using QDir::mkdir;
    using QDir::path;
    using QDir::removeRecursively;

    bool isValid();

    static BiodynamoDirectory platformDirectory();
    static BiodynamoDirectory lastProjectDirectory();
    static BiodynamoDirectory lastProjectBuildDirectory();

    QString getCMakeListsTxtPath();
    QString getWorkingDirectory();
    DirectoryType getType();

    bool fileExists(const QString &fileName);

protected:
private:
    DirectoryType type;
    QString errorMessage();
};

#endif // BIODYNAMODIRECTORY_H
