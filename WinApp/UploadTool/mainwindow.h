#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProcess>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:

    void on_toolButtonCoePath_clicked();

    void on_pushButtonCoeUpload_clicked();

    void on_toolButtonWavePath_clicked();

    void Process_readyReadOutput();

    void on_pushButtonWaveUpload_clicked();

    void on_actionAbout_triggered();

    void on_actionUpload_firmware_triggered();

private:
    Ui::MainWindow *ui;
    QString log_buffer;
    QProcess *process;

    void tftpUpload(const QString &filepath, const QString &targetDir);
};
#endif // MAINWINDOW_H
