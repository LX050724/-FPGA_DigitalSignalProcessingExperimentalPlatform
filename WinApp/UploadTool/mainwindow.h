#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProcess>
#include <QUdpSocket>
#include <QTemporaryDir>
#include <QJsonArray>
#include <QNetworkAccessManager>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
Q_OBJECT

    typedef struct {
        QString filepath;
        QString targetDir;
        QString targetFilename;
    } tftp_uploadItem;

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

    void Process_readyReadOutput();
    void tftp_process_finished(int exitCode, QProcess::ExitStatus exitStatus);
    void unzip_process_finished(int exitCode, QProcess::ExitStatus exitStatus);
    void udp_readyRead();
    void http_request_finished(QNetworkReply *reply);

    void on_actionCheck_firmware_Update_triggered();

private:
    Ui::MainWindow *ui;
    QString log_buffer;
    QProcess *tftp_process;
    QProcess *unzip_process;
    QUdpSocket *udpSocket;
    QNetworkAccessManager *networkAccessManager;

    QHostAddress address;
    QTemporaryDir temporaryDir;
    QStringList log_buf;
    QString fw_version;
    QJsonArray fw_list;
    QJsonArray res_list;
    QJsonArray app_list;
    QList<tftp_uploadItem> tftpUpload_fifo;
protected:
    void closeEvent(QCloseEvent *event) override;
private:

    void update_fw();
    void udp_sendMsg(uint8_t message_id, const QByteArray &data = {});
    void tftpUpload(const QString &filepath, const QString &targetDir, const QString &targetFilename = QString());
    void unzip(const QString &file_path);
    void log_printf(const char *fmt, ...);
    void log_println(const char *fmt, ...);
    QList<MainWindow::tftp_uploadItem> searchDir(const QString &path, const QString &targetDir);
};

#endif // MAINWINDOW_H
