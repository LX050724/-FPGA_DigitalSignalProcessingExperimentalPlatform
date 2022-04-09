#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProcess>
#include <QUdpSocket>
#include <QTemporaryDir>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

    typedef enum {
        UDP_COMM_ACK,
        UDP_COMM_ERR,
        UDP_COMM_NO_MSG_ID,
    } UDP_COMM_CMD_CODE;

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:

    void on_toolButtonCoePath_clicked();
    void on_pushButtonCoeUpload_clicked();
    void on_toolButtonWavePath_clicked();
    void on_pushButtonWaveUpload_clicked();
    void on_actionAbout_triggered();
    void on_actionUpload_firmware_triggered();
    void on_pushButtonSetIP_clicked();
    void on_actionChick_firmware_Update_triggered();

    void Process_readyReadOutput();
    void Process_finished(int, QProcess::ExitStatus);
    void udp_readyRead();

private:
    Ui::MainWindow *ui;
    QString log_buffer;
    QProcess *process;
    QProcess *unzip_process;
    QUdpSocket *udpSocket;
    QHostAddress address;
    QTemporaryDir tmp_dir;
    void tftpUpload(const QString &filepath, const QString &targetDir, const QString &targetFilename = QString());
    void log_printf(const char *fmt, ...);
    void udp_sendMsg(uint8_t message_id, const QByteArray &data = {});
    void log_println(const char *fmt, ...);
    void unzip(const QString& file_path);
};
#endif // MAINWINDOW_H
