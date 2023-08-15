#ifndef BIODYNAMOPROCESS_H
#define BIODYNAMOPROCESS_H

#include "biodynamodirectory.h"

#include <QProcess>

class BiodynamoProcess : public QObject
{
    Q_OBJECT;

public:
    explicit BiodynamoProcess(QObject *parent = nullptr);
    explicit BiodynamoProcess(const BiodynamoDirectory &projectDirectory, QObject *parent = nullptr);
    explicit BiodynamoProcess(const BiodynamoDirectory &projectDirectory,
                              const BiodynamoDirectory &platformDirectory,
                              QObject *parent = nullptr);
    BiodynamoProcess();
    ~BiodynamoProcess();

    void loadDefaults();

    BiodynamoDirectory platformDirectory() const;
    void setPlatformDirectory(const BiodynamoDirectory &dir);

    BiodynamoDirectory projectDirectory() const;
    void setProjectDirectory(const BiodynamoDirectory &dir);

    bool isInitialised();
    bool isRunning();
    bool isNotReady();

    void initialise(const QStringList &commands);
    void start();
    void kill();

    bool disconnectFromProcess();
    bool disconnectFromProcess(const QObject *receiver);

    QByteArray readAll();
    QByteArray readAllStandardOutput();
    QByteArray readAllStandardError();

    QProcess *getProcess();

public slots:
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    QProcess *process = nullptr;

    BiodynamoDirectory _projectDirectory;
    BiodynamoDirectory _platformDirectory;
};
#endif // BIODYNAMOPROCESS_H
