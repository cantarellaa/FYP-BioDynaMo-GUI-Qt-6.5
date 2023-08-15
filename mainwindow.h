#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "biodynamoprocess.h"

#include <QFileIconProvider>
#include <QFileSystemModel>
#include <QJsonDocument>
#include <QList>
#include <QMainWindow>
#include <QPlainTextEdit>
#include <QProcess>
#include <QStandardItemModel>
#include <QTreeView>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_projectDirView_doubleClicked(const QModelIndex &index);

    void onNewProjectProcessStdOutput();
    void onNewProjectProcessStdError();
    void onNewProjectProcessErrorOccured(QProcess::ProcessError error);
    void onNewProjectProcessFinished(int exitCode, QProcess::ExitStatus exitStatus, QString newProjectPath);

    void onBuildOutput();
    void onBuildErrorOccured(QProcess::ProcessError error);
    void onBuildFinished(int exitCode, QProcess::ExitStatus exitStatus);

    void onExportJsonOutput(QString fileName);
    void onExportJsonErrorOccured(QProcess::ProcessError error);
    void onExportJsonFinished(int exitCode, QProcess::ExitStatus exitStatus);

    void on_actionNew_Project_triggered();
    void on_actionBuild_Project_triggered();
    void on_actionExportDefault_Project_triggered();
    void on_actionUser_Help_triggered();
    void on_actionDeveloper_Help_triggered();
    void on_actionOpen_in_FileManager_triggered();
    void on_actionBiodynamo_GitHub_triggered();
    void on_actionChange_BioDynaMo_Path_triggered();
    void on_actionOpen_Project_triggered();

    void on_actionRun_Project_triggered();

    void onRunOutput();
    void onRunFinished();
    void onRunErrorOccured();

    void on_projectDirView_activated(const QModelIndex &index);

private:
    Ui::MainWindow *ui;

    //
    bool initialised = false;

    // settings
    QString fontFamily = "Helvetica";
    int fontSize = 12;
    BiodynamoDirectory *platformDirectory = nullptr;
    BiodynamoDirectory *projectDirectory = nullptr;

    void writeSettings();
    void readSettings();
    void changeBiodynamoPath();
    void closeEvent(QCloseEvent *event);

    // processes
    BiodynamoProcess *newProjectProcess = nullptr;
    BiodynamoProcess *buildProjectProcess = nullptr;
    BiodynamoProcess *exportJsonProcess = nullptr;
    BiodynamoProcess *saveProjectProcess = nullptr;
    BiodynamoProcess *runProjectProcess = nullptr;

    // current project
    QString currentProjectName;
    BiodynamoDirectory *buildDirectory = nullptr;
    QJsonDocument bdmConfig;
    QJsonDocument bdmConfigDefault;

    void setProjectDirectory(const QString &newPath, const QString &oldPath = "");
    void loadProjectName();
    void openProjectDirectoryInTreeView();
    void readBdmConfig();
    void exportDefaultJsonToFile(bool overwrite = false);
    void exportJsonToFile(QString fileName);

    // views & models
    QFileIconProvider *iconProvider = new QFileIconProvider;
    QFileSystemModel *projectDirModel = new QFileSystemModel;
    QStandardItemModel *projectParamModel = new QStandardItemModel;
};
#endif // MAINWINDOW_H
