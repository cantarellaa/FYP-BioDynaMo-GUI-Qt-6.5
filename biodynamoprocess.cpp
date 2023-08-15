#include "biodynamoprocess.h"
#include "biodynamodirectory.h"

#include <QDir>
#include <QFileInfo>
#include <QMessageBox>
#include <QSettings>

BiodynamoProcess::BiodynamoProcess()
    : QObject()
{
}

BiodynamoProcess::BiodynamoProcess(QObject *parent)
    : QObject(parent)
{}

BiodynamoProcess::BiodynamoProcess(const BiodynamoDirectory &projectDirectory, const BiodynamoDirectory &platformDirectory, QObject *parent)
    : QObject(parent)
    , _projectDirectory(projectDirectory)
    , _platformDirectory(platformDirectory)
{}

BiodynamoProcess::BiodynamoProcess(const BiodynamoDirectory &projectDirectory, QObject *parent)
    : QObject(parent)
    , _projectDirectory(projectDirectory)
{
    setPlatformDirectory(BiodynamoDirectory::platformDirectory());
}

BiodynamoProcess::~BiodynamoProcess()
{
    qDebug() << "destructor";

    if (process != nullptr) {
        process->deleteLater();
        process = nullptr;
    }

    deleteLater();
}

void BiodynamoProcess::loadDefaults()
{
    setProjectDirectory(BiodynamoDirectory::lastProjectDirectory());
    setPlatformDirectory(BiodynamoDirectory::platformDirectory());
}

BiodynamoDirectory BiodynamoProcess::projectDirectory() const {
  return _projectDirectory;
}

void BiodynamoProcess::setProjectDirectory(const BiodynamoDirectory &projectDirectory)
{
  _projectDirectory = projectDirectory;
}

BiodynamoDirectory BiodynamoProcess::platformDirectory() const
{
  return _platformDirectory;
}

void BiodynamoProcess::setPlatformDirectory(const BiodynamoDirectory &platformDirectory)
{
  _platformDirectory = platformDirectory;
}

void BiodynamoProcess::initialise(const QStringList &commands)
{
  if (isRunning()) {
      throw std::runtime_error("Process is still running");
  }

  if (process == nullptr) {
      process->deleteLater();
  }

  process = new QProcess;
  process->setWorkingDirectory(_projectDirectory.getWorkingDirectory());

#ifdef Q_OS_WINDOWS
  process->setProgram("powershell");
  process->setArguments(commands.join(";1337").split("1337", Qt::SkipEmptyParts));
#else
  process->setProgram("bash");
  process->setArguments(
      QStringList() << "-c"
                    << (QString("source %1").arg(_platformDirectory.absoluteFilePath("thisbdm.sh")))
                           .append(" && ")
                           .append(commands.join(" && ")));

  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  env.insert("PATH", env.value("PATH").append(":/usr/local/bin"));
  process->setProcessEnvironment(env);
#endif

  qDebug() << "after init";
  qDebug() << process->workingDirectory();
  qDebug() << process->program();
  qDebug() << process->arguments();
}

bool BiodynamoProcess::isNotReady()
{
  return process == nullptr;
}

bool BiodynamoProcess::isInitialised()
{
  return process != nullptr && process->state() == QProcess::NotRunning;
}

bool BiodynamoProcess::isRunning()
{
  return process != nullptr && process->state() != QProcess::NotRunning;
}

void BiodynamoProcess::start()
{
  if (!isInitialised()) {
      throw std::runtime_error("Process is not initialised");
  }
  if (isRunning()) {
      throw std::runtime_error("Process is still running");
  }

  qDebug() << "trying to start process";
  //QObject::connect(process, &QProcess::finished, this, &BiodynamoProcess::onProcessFinished);

  qDebug() << process->workingDirectory();
  qDebug() << process->program();
  qDebug() << process->arguments();

  process->start();
}

void BiodynamoProcess::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
  // ???
}

void BiodynamoProcess::kill()
{
  if (process != nullptr) {
      process->kill();
  }
}

bool BiodynamoProcess::disconnectFromProcess()
{
  return QObject::disconnect(process) && process->disconnect();
}

bool BiodynamoProcess::disconnectFromProcess(const QObject *receiver)
{
  return disconnectFromProcess() && process->disconnect(receiver);
}

QByteArray BiodynamoProcess::readAll()
{
  return process->readAll();
}

QByteArray BiodynamoProcess::readAllStandardOutput()
{
  return process->readAllStandardOutput();
}

QByteArray BiodynamoProcess::readAllStandardError()
{
  return process->readAllStandardError();
}

QProcess *BiodynamoProcess::getProcess()
{
  return process;
}
