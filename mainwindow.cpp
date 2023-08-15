#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "appsettings.h"
#include "biodynamoprocess.h"
#include "simulationparameter.h"
#include <QDesktopServices>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFileIconProvider>
#include <QFileSystemModel>
#include <QInputDialog>
#include <QJsonArray>
#include <QJsonObject>
#include <QLabel>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QProcess>
#include <QScroller>
#include <QSettings>
#include <QTreeView>
#include <QUrl>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);
  readSettings();
  // ui->projectDirView->setAttribute(Qt::WidgetAttribute::WA_AcceptTouchEvents,
  // false);
  // ui->projectDataView->setAttribute(Qt::WidgetAttribute::WA_AcceptTouchEvents,
  // false);

  //ui->filePreviewFrame->hide();
  ui->codeEditorFrame->hide();

  initialised = true;
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::closeEvent(QCloseEvent *event) {
  writeSettings();
  event->accept();
}

void MainWindow::readSettings() {
  QSettings settings;

  QString biodynamoPath = settings
                              .value(AppSettings::enumToString(
                                  AppSettings::SettingsKeys::Platform_Path))
                              .toString();
  if (biodynamoPath.isEmpty() || biodynamoPath.isNull()) {
    changeBiodynamoPath();
  } else {
    platformDirectory = new BiodynamoDirectory(
        BiodynamoDirectory::DirectoryType::Platform, biodynamoPath);
  }

  QString lastProjectPath =
      settings
          .value(AppSettings::enumToString(
              AppSettings::SettingsKeys::Last_Project_Path))
          .toString();
  qDebug() << lastProjectPath;
  if (lastProjectPath.isEmpty() || lastProjectPath.isNull()) {
    // do nothing
  } else {
    setProjectDirectory(lastProjectPath);
  }

  fontFamily = settings
                   .value(AppSettings::enumToString(
                              AppSettings::SettingsKeys::Font_Family),
                          "Helvetica")
                   .toString();
  fontSize = settings
                 .value(AppSettings::enumToString(
                            AppSettings::SettingsKeys::Font_Size),
                        12)
                 .toInt();

  QApplication::setFont(QFont(
      QStringList({fontFamily, "Helvetica", "Arial", "Sans-Serif"}), fontSize));
}

void MainWindow::writeSettings() {
  QSettings settings;

  if (platformDirectory != nullptr) {
    settings.setValue(
        AppSettings::enumToString(AppSettings::SettingsKeys::Platform_Path),
        platformDirectory->absolutePath());
  } else {
    settings.remove(
        AppSettings::enumToString(AppSettings::SettingsKeys::Platform_Path));
  }
  if (projectDirectory != nullptr) {
    settings.setValue(
        AppSettings::enumToString(AppSettings::SettingsKeys::Last_Project_Path),
        projectDirectory->absolutePath());
  } else {
    settings.remove(AppSettings::enumToString(
        AppSettings::SettingsKeys::Last_Project_Path));
  }

  settings.setValue(
      AppSettings::enumToString(AppSettings::SettingsKeys::Font_Size),
      fontSize);
  settings.setValue(
      AppSettings::enumToString(AppSettings::SettingsKeys::Font_Family),
      fontFamily.first(1));
}

void MainWindow::on_actionNew_Project_triggered() {
  bool ok;

  QStringList projectTypes = AppSettings::ProjectTypes.values();
  projectTypes.prepend("New");
  QString type =
      QInputDialog::getItem(this, tr("Choose project type"), tr("Project type"),
                            projectTypes, 0, false, &ok);

  if (!ok || type.isEmpty()) {
    return;
  }

  QString projectPath = QFileDialog::getExistingDirectory(
      this, tr("Choose directory where to store new project"));

  if (projectPath.isEmpty() || projectPath.isNull()) {
    return;
  }

  QInputDialog pathDialog = new QInputDialog(this);
  pathDialog.resize(500, 300);
  pathDialog.setLabelText("Project name");
  pathDialog.setWindowTitle("Enter project name:");
  pathDialog.setInputMode(QInputDialog::TextInput);
  pathDialog.exec();

  QString newProjectName = pathDialog.textValue();
  if (newProjectName.isEmpty() || newProjectName.isNull()) {
    QMessageBox::warning(this, "Warning", "Project name can not be empty");
    return;
  }

  qDebug() << "trying to init biodynamo directory for path" << projectPath;

  BiodynamoDirectory dir = BiodynamoDirectory(
      BiodynamoDirectory::DirectoryType::New,
      projectPath.append(QString("/%1").arg(newProjectName)));

  newProjectProcess = new BiodynamoProcess(dir, *platformDirectory);

#ifdef Q_OS_WINDOWS
  newProjectProcess->initialise(
      {QString("New-Item -ItemType Directory -Path %1").arg(projectPath),
       QString("New-Item -ItemType File -Path %1/CMakeLists.txt").arg(projectPath),
       QString("Set-Content -Path ./%1/CMakeLists.txt -Value \"project(%2)\"").arg(newProjectName).arg(newProjectName),
       QString("New-Item -ItemType File -Path \"%1/%2.txt\"").arg(newProjectName).arg(newProjectName)});
#else
  if (projectTypes.indexOf(type) == -1) {
    newProjectProcess->initialise({QString("biodynamo new %1").arg(newProjectName)});
  } else {
    int index = projectTypes.indexOf(type) - 1;

    QString demoName = AppSettings::ProjectTypes.keys().at(index);

    qDebug() << "trying to create demo at " << dir.path() << "using name"
             << demoName;

    if (!demoName.isEmpty()) {
        newProjectProcess->initialise({QString(" mkdir %1 ").arg(newProjectName),
                                       QString(" cd %1 ").arg(newProjectName),
                                       QString(" biodynamo demo %1 ").arg(demoName),
                                       QString(" cp -r ./%1/* . ").arg(demoName),
                                       QString(" rm -r %1 ").arg(demoName)});
    }
  }
#endif

  connect(newProjectProcess->getProcess(), &QProcess::readyReadStandardOutput,
          this, &MainWindow::onNewProjectProcessStdOutput);
  connect(newProjectProcess->getProcess(), &QProcess::readyReadStandardError,
          this, &MainWindow::onNewProjectProcessStdError);
  connect(newProjectProcess->getProcess(), &QProcess::finished, this, [this, dir](int exitCode, QProcess::ExitStatus exitStatus) {
      onNewProjectProcessFinished(exitCode, exitStatus, dir.absolutePath());
  });
  connect(newProjectProcess->getProcess(), &QProcess::errorOccurred, this,
          &MainWindow::onNewProjectProcessErrorOccured);
  newProjectProcess->start();
}

void MainWindow::onNewProjectProcessErrorOccured(QProcess::ProcessError error) {
  qDebug() << "error occured";
  qDebug() << error;

  newProjectProcess->kill();
  newProjectProcess->disconnectFromProcess(this);
}

void MainWindow::onNewProjectProcessStdOutput()
{
  ui->consoleOutput->append(newProjectProcess->readAllStandardOutput());
}

void MainWindow::onNewProjectProcessStdError()
{
  ui->consoleOutput->append(newProjectProcess->readAllStandardError());
}

void MainWindow::onNewProjectProcessFinished(int exitCode,
                                             QProcess::ExitStatus exitStatus,
                                             QString newProjectPath) {
  if (newProjectProcess != nullptr) {
    if (exitStatus == QProcess::NormalExit && exitCode == 0) {
      setProjectDirectory(newProjectPath, projectDirectory != nullptr
                                              ? projectDirectory->absolutePath()
                                              : "");
    } else {
      QMessageBox::warning(this, "Warning",
                           QString("Exit status: %1 | Exit code: %2")
                               .arg(exitStatus)
                               .arg(exitCode));
    }

    newProjectProcess->disconnectFromProcess();
  }
}

void MainWindow::loadProjectName()
{
  static QRegularExpression regex("\\((.*?)\\)");
  QFile cmakeFile(projectDirectory->getCMakeListsTxtPath());
  cmakeFile.open(QFile::ReadOnly | QFile::Text);
  QTextStream in(&cmakeFile);
  QString line;
  do {
    line = in.readLine();
    if (line.contains("project(", Qt::CaseSensitive)) {
      currentProjectName = regex.match(line).captured(1);
      break;
    }
  } while (!line.isNull());
  cmakeFile.close();

  if (currentProjectName.isEmpty() || currentProjectName.isNull()) {
    currentProjectName = projectDirectory->dirName();
  }
}

void MainWindow::openProjectDirectoryInTreeView()
{
  projectDirModel->setIconProvider(iconProvider);
  projectDirModel->setFilter(QDir::NoDotAndDotDot | QDir::AllDirs |
                             QDir::Files);
  projectDirModel->setRootPath(projectDirectory->absolutePath());

  ui->projectDirView->setModel(projectDirModel);
  ui->projectDirView->setAnimated(true);
  ui->projectDirView->setIndentation(20);
  ui->projectDirView->setSortingEnabled(false);
  ui->projectDirView->setRootIndex(
      projectDirModel->index(projectDirectory->absolutePath()));
  ui->projectDirView->hideColumn(1);
  ui->projectDirView->hideColumn(2);
  ui->projectDirView->hideColumn(3);
  ui->projectDirView->setHeaderHidden(true);

  QScroller::grabGesture(ui->projectDirView, QScroller::TouchGesture);

  ui->tabWidget->setCurrentWidget(ui->tabWidget->findChild<QWidget *>(ui->dir_tab->objectName()));

  if (initialised) {
    QMessageBox::information(this,
                             "Project Loaded",
                             QString("Project %1 [%2] loaded successfully")
                                 .arg(currentProjectName)
                                 .arg(projectDirectory->absolutePath()));
  }

  exportDefaultJsonToFile(false);
  readBdmConfig();
}

void MainWindow::setProjectDirectory(const QString &newPath,
                                     const QString &oldPath) {
  if (oldPath == newPath) {
    return;
  }

  try {
    qDebug() << "setProjectDirectory" << newPath << oldPath;
    projectDirectory = new BiodynamoDirectory(
        BiodynamoDirectory::DirectoryType::Project, newPath);
    buildDirectory =
        new BiodynamoDirectory(BiodynamoDirectory::DirectoryType::Build,
                               projectDirectory->filePath("/build"));
  } catch (std::invalid_argument &e) {
    QMessageBox::warning(this, "Warning", e.what());

    setProjectDirectory(oldPath);

    return;
  }

  loadProjectName();
  openProjectDirectoryInTreeView();
}

void MainWindow::on_actionBuild_Project_triggered() {
  if (projectDirectory == nullptr || !projectDirectory->isValid()) {
    QMessageBox::warning(this, "title", "Create or Open project first");
    return;
  }

  buildDirectory = new BiodynamoDirectory(BiodynamoDirectory::DirectoryType::Build,
                                          QDir::cleanPath(
                                              projectDirectory->absolutePath().append("/build")));

  if (!buildDirectory->exists()) {
    qDebug() << buildDirectory->path() << "does not exist";
    qDebug() << buildDirectory->absolutePath() << "does not exist";
    QDir().mkdir(buildDirectory->absolutePath());
  } else {
    qDebug() << buildDirectory->path() << "exists";
  }

  buildProjectProcess = new BiodynamoProcess(*buildDirectory, *platformDirectory);

#ifdef Q_OS_WINDOWS
  buildProjectProcess->initialise({"New-Item -ItemType File -Path build.txt"});
#else
  buildProjectProcess->initialise({"cmake .. && make -j4"});
#endif

  connect(buildProjectProcess->getProcess(), &QProcess::readyReadStandardOutput,
          this, &MainWindow::onBuildOutput);
  connect(buildProjectProcess->getProcess(), &QProcess::readyReadStandardError,
          this, &MainWindow::onBuildOutput);
  connect(buildProjectProcess->getProcess(), &QProcess::finished, this,
          &MainWindow::onBuildFinished);
  connect(buildProjectProcess->getProcess(), &QProcess::errorOccurred, this,
          &MainWindow::onBuildErrorOccured);
  buildProjectProcess->start();
}

void MainWindow::onBuildOutput() {
  ui->consoleOutput->append(buildProjectProcess->readAll());
}

void MainWindow::onBuildFinished(int exitCode,
                                 QProcess::ExitStatus exitStatus) {
  buildProjectProcess->disconnectFromProcess(this);

  exportDefaultJsonToFile(true);
}

void MainWindow::onBuildErrorOccured(QProcess::ProcessError error) {
  qDebug() << "error occured";
  qDebug() << error;

  buildProjectProcess->disconnectFromProcess(this);
}

void MainWindow::exportDefaultJsonToFile(bool overwrite) {
  if (!projectDirectory->exists("build")) {
    return;
  }

  QString fileName = projectDirectory->absolutePath().append("/bdm.default.json");

  QFileInfo fileInfo(fileName);

  if (overwrite || !fileInfo.exists()) {
    exportJsonToFile(fileName);
  }
}

void MainWindow::exportJsonToFile(QString fileName) {
  if (projectDirectory == nullptr || !projectDirectory->isValid()) {
    QMessageBox::warning(this, "title", "Create or Open project first");
    return;
  }

  if (!projectDirectory->exists("build")) {
    QMessageBox::warning(this, "title", "Build first");
    return;
  }

  exportJsonProcess = new BiodynamoProcess(*buildDirectory, *platformDirectory);

#ifdef Q_OS_WINDOWS
  exportJsonProcess->initialise({"Write-Host \"{'test': 123}\""});
#else
  exportJsonProcess->initialise(
      {QString("./%1 --output-default-json").arg(currentProjectName)});

#endif

  connect(exportJsonProcess->getProcess(), &QProcess::readyReadStandardOutput,
          this, [this, fileName] { onExportJsonOutput(fileName); });
  connect(exportJsonProcess->getProcess(), &QProcess::finished, this,
          &MainWindow::onExportJsonFinished);
  connect(exportJsonProcess->getProcess(), &QProcess::errorOccurred, this, &MainWindow::onExportJsonErrorOccured);

  exportJsonProcess->start();
}

void MainWindow::onExportJsonOutput(QString fileName) {
  QString read = exportJsonProcess->readAllStandardOutput();
  if (read.first(1) == "{") {
    QJsonParseError error;
    QJsonDocument document = QJsonDocument::fromJson(read.toUtf8(), &error);
    if (error.error == QJsonParseError::NoError) {
      qDebug() << document.toJson(QJsonDocument::Compact);

      QFile file(fileName);
      file.open(QFile::ReadWrite);
      file.write(document.toJson(QJsonDocument::Indented));
      file.close();
    } else {
      qDebug() << error.errorString();
    }
  } else {
    qDebug() << read;
  }
}

void MainWindow::onExportJsonFinished(int exitCode,
                                      QProcess::ExitStatus exitStatus) {
  exportJsonProcess->disconnectFromProcess(this);
}

void MainWindow::onExportJsonErrorOccured(QProcess::ProcessError error) {
  qDebug() << error;
  exportJsonProcess->kill();
  exportJsonProcess->disconnectFromProcess(this);
}

void MainWindow::readBdmConfig() {
  QString filePath = projectDirectory->filePath("/bdm.default.json");

  qDebug() << filePath;

  QFile bdmConfigDefaultFile(filePath);
  if (bdmConfigDefaultFile.exists()) {
    bdmConfigDefaultFile.open(QFile::ReadOnly | QFile::Text);
    QJsonParseError error;
    bdmConfigDefault =
        QJsonDocument::fromJson(bdmConfigDefaultFile.readAll(), &error);
    if (error.error != QJsonParseError::NoError) {
      QMessageBox::warning(this, "title", error.errorString());
    }
    bdmConfigDefaultFile.close();
  } else if (!bdmConfigDefault.isEmpty()) {
    bdmConfigDefault = QJsonDocument::fromJson(QByteArray("{}"));
  }

  qDebug() << bdmConfigDefault.toJson();

  QFile bdmConfigFile(projectDirectory->filePath("/bdm.json"));
  if (bdmConfigFile.exists()) {
    bdmConfigFile.open(QFile::ReadOnly | QFile::Text);
    QJsonParseError error;
    bdmConfig = QJsonDocument::fromJson(bdmConfigFile.readAll());
    if (error.error != QJsonParseError::NoError) {
      QMessageBox::warning(this, "title", error.errorString());
    }
    bdmConfigFile.close();
  } else if (!bdmConfigDefault.isEmpty()) {
    bdmConfig = QJsonDocument::fromJson(bdmConfigDefault.toJson());
  } else {
    bdmConfig = QJsonDocument::fromJson(QByteArray("{}"));
  }

  qDebug() << bdmConfig.toJson();

  projectParamModel->clear();

  QJsonObject iterable = bdmConfig.object();
  QJsonObject::const_iterator it;
  for (it = iterable.constBegin(); it != iterable.constEnd(); it++) {
    QString key = it.key();
    QJsonObject value = it->toObject();
    SimulationParameter *simpam = new SimulationParameter(key, value.value("_typename").toString(), value.toVariantMap());
    projectParamModel->appendRow(simpam);
  }

  ui->projectDataView->setModel(projectParamModel);
}

void MainWindow::on_actionExportDefault_Project_triggered() {
  QString fileName = QFileDialog::getSaveFileName(
      this, "Caption", projectDirectory->filePath("/bdm.json"),
      "JSON (*.json)");
  exportJsonToFile(fileName);
}

void MainWindow::on_actionUser_Help_triggered() {
  QDesktopServices::openUrl(
      QUrl("https://www.biodynamo.org/user-guide/", QUrl::TolerantMode));
}

void MainWindow::on_actionDeveloper_Help_triggered() {
  QDesktopServices::openUrl(
      QUrl("https://www.biodynamo.org/developer-guide", QUrl::TolerantMode));
}

void MainWindow::on_actionOpen_in_FileManager_triggered() {
  if (projectDirectory != nullptr) {
    QDesktopServices::openUrl(
        QUrl(QString("file:///").append(projectDirectory->absolutePath())));
  }
}

void MainWindow::on_actionBiodynamo_GitHub_triggered() {
  QDesktopServices::openUrl(
      QUrl("https://github.com/BioDynaMo/biodynamo", QUrl::TolerantMode));
}

void MainWindow::on_actionChange_BioDynaMo_Path_triggered() {
  changeBiodynamoPath();
}

void MainWindow::changeBiodynamoPath() {
  QMessageBox::information(this, "BioDyNaMo", "Please provide correct path to BioDyNaMo bin directory, containing thisbdm.sh file");

  QString newBiodynamoPath = QFileDialog::getExistingDirectory(this, "Find BioDyNamo bin directory");

  qDebug() << newBiodynamoPath;

  if (newBiodynamoPath.isEmpty() || newBiodynamoPath.isNull() ||
      !QDir(newBiodynamoPath).exists("thisbdm.sh")) {
    QMessageBox::critical(
        this, "Error",
        "You must specify path to thisbdm.sh for this app to work");

    if (platformDirectory == nullptr || !platformDirectory->isValid()) {
      qDebug() << "quit1";
      qDebug() << QDir(newBiodynamoPath).path();
      qDebug() << QDir(newBiodynamoPath).exists("thisbdm.sh");
      qApp->quit();
    }
  } else {
    try {
      platformDirectory = new BiodynamoDirectory(
          BiodynamoDirectory::DirectoryType::Platform, newBiodynamoPath);
    } catch (std::invalid_argument &e) {
      qDebug() << "quit2";

      QMessageBox::critical(
          this, "Error",
          "You must specify path to thisbdm.sh for this app to work");

      qApp->quit();
    }
  }
}

void MainWindow::on_actionOpen_Project_triggered() {
  QString projectPath = QFileDialog::getExistingDirectory(
      this, tr("Choose directory containing a project"));

  if (projectPath.isEmpty() || projectPath.isNull()) {
    return;
  }

  setProjectDirectory(projectPath, projectDirectory != nullptr
                                       ? projectDirectory->absolutePath()
                                       : "");
}

void MainWindow::on_actionRun_Project_triggered()
{
  if (projectDirectory == nullptr || !projectDirectory->isValid()) {
    QMessageBox::warning(this, "title", "Create or Open project first");
    return;
  }

  if (buildDirectory == nullptr || !buildDirectory->absolutePath().contains(projectDirectory->absolutePath())) {
    buildDirectory = new BiodynamoDirectory(BiodynamoDirectory::DirectoryType::Build, projectDirectory->filePath("/build"));
  }

  if (!buildDirectory->exists()) {
    on_actionBuild_Project_triggered();
  }

  runProjectProcess = new BiodynamoProcess(*projectDirectory, *platformDirectory);

#ifdef Q_OS_WINDOWS
  runProjectProcess->initialise({"New-Item -ItemType File -Path run.txt"});
#else
  runProjectProcess->initialise({QString("biodynamo run %1").arg(currentProjectName)});
#endif

  connect(runProjectProcess->getProcess(), &QProcess::readyReadStandardOutput, this, &MainWindow::onRunOutput);
  connect(runProjectProcess->getProcess(), &QProcess::readyReadStandardError, this, &MainWindow::onRunOutput);
  connect(runProjectProcess->getProcess(), &QProcess::finished, this, &MainWindow::onRunFinished);
  connect(runProjectProcess->getProcess(), &QProcess::errorOccurred, this, &MainWindow::onRunErrorOccured);
  runProjectProcess->start();
}

void MainWindow::onRunOutput()
{
  ui->consoleOutput->append(runProjectProcess->readAll());
}

void MainWindow::onRunFinished() {}

void MainWindow::onRunErrorOccured() {}

void MainWindow::on_projectDirView_activated(const QModelIndex &index)
{
  QString filePath = projectDirModel->filePath(index);
  QFile file(filePath);
  QFileInfo fileInfo(filePath);

  qDebug() << file.fileName();

  file.open(QFile::ReadOnly | QFile::Text);
  ui->filePreview->setPlainText(file.readAll());
  ui->filePreviewFrame->show();
}

void MainWindow::on_projectDirView_doubleClicked(const QModelIndex &index)
{
  QString filePath = projectDirModel->filePath(index);
  QFile file(filePath);
  QFileInfo fileInfo(filePath);
  if (fileInfo.baseName().endsWith(".json") || fileInfo.baseName().endsWith(".toml")) {
  } else {
  }
}
