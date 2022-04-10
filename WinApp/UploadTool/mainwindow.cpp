#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <cstdarg>
#include <QFileDialog>
#include <QHostAddress>
#include <QMessageBox>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QCloseEvent>

#define UPDATE_SERVER "http://127.0.0.1:5000"

MainWindow::MainWindow(QWidget *parent)
        : QMainWindow(parent), ui(new Ui::MainWindow),
          tftp_process(new QProcess(this)), unzip_process(new QProcess(this)), udpSocket(new QUdpSocket(this)),
          networkAccessManager(new QNetworkAccessManager(this)) {
    ui->setupUi(this);

    tftp_process->setProgram(QFileInfo("./tftp.exe").absoluteFilePath());
    connect(tftp_process, SIGNAL(readyReadStandardOutput()),
            this, SLOT(Process_readyReadOutput()));
    connect(tftp_process, SIGNAL(finished(int, QProcess::ExitStatus)),
            this, SLOT(tftp_process_finished(int, QProcess::ExitStatus)));

    unzip_process->setProgram(QFileInfo("./7zr.exe").absoluteFilePath());
    connect(unzip_process, SIGNAL(readyReadStandardOutput()),
            this, SLOT(Process_readyReadOutput()));
    connect(unzip_process, SIGNAL(finished(int, QProcess::ExitStatus)),
            this, SLOT(unzip_process_finished(int, QProcess::ExitStatus)));

    connect(networkAccessManager, SIGNAL(finished(QNetworkReply * )),
            this, SLOT(http_request_finished(QNetworkReply * )));

    ui->textEditLog->document()->setMaximumBlockCount(200);
    on_pushButtonSetIP_clicked();
    udpSocket->bind(70);
    connect(udpSocket, SIGNAL(readyRead()),
            this, SLOT(udp_readyRead()));
    qDebug() << "tmp dir:" << temporaryDir.path();
}

MainWindow::~MainWindow() {
    delete ui;
    delete tftp_process;
    delete unzip_process;
    delete udpSocket;
    delete networkAccessManager;
}

void MainWindow::on_toolButtonCoePath_clicked() {
    QString CoePath = QFileDialog::getOpenFileName(this, tr("Select coe file"),
                                                   QDir::homePath(), "Xilinx COE files (*.coe)");
    ui->lineEditCoePath->setText(CoePath);
}

void MainWindow::on_toolButtonWavePath_clicked() {
    QString WavePath = QFileDialog::getOpenFileName(this, tr("Select wave file"),
                                                    QDir::homePath(), "Binary files (*.bin);;JSON files (*.json)");
    ui->lineEditCoePath->setText(WavePath);
}

void MainWindow::on_pushButtonCoeUpload_clicked() {
    tftpUpload(ui->lineEditCoePath->text(), "0:/数字滤波器");
}

void MainWindow::on_pushButtonWaveUpload_clicked() {
    tftpUpload(ui->lineEditWavePath->text(), "0:/信号发生器");
}

void MainWindow::Process_readyReadOutput() {
    QString str;
    while (tftp_process->bytesAvailable())
        str += QString::fromLocal8Bit(tftp_process->readLine());
    int new_line_index = str.lastIndexOf('\n');
    if (new_line_index >= 0 || log_buf.length() > 32) {
        log_buf << str.left(new_line_index);
        ui->textEditLog->moveCursor(QTextCursor::End);
        ui->textEditLog->insertPlainText(log_buf.join(""));
        log_buf.clear();
        log_buf << str.mid(new_line_index + 1);
    }
}


QList<MainWindow::tftp_uploadItem> MainWindow::searchDir(const QString &path, const QString &targetDir) {
    QFileInfo fileInfo(path);
    if (fileInfo.isFile()) return {{fileInfo.absoluteFilePath(), targetDir, ""}};
    QList<MainWindow::tftp_uploadItem> list;
    QDir dir(path);
    for (const QFileInfo &info: dir.entryInfoList(QDir::NoDotAndDotDot | QDir::AllEntries)) {
        if (info.isFile()) {
            list.append({info.absoluteFilePath(), targetDir, ""});
        } else {
            QDir target_dir(targetDir);
            list.append(searchDir(info.absoluteFilePath(), target_dir.absoluteFilePath(info.fileName())));
        }
    }
    return list;
}

void MainWindow::tftpUpload(const QString &filepath, const QString &targetDir, const QString &targetFilename) {
    if (tftp_process->isOpen()) {
        tftpUpload_fifo.append({filepath, targetDir, targetFilename});
        return;
    }
    QFileInfo sourceFile(filepath);
    if (sourceFile.isDir()) {
        auto list = searchDir(filepath, targetDir);
        auto item = list.first();
        list.removeFirst();
        tftpUpload_fifo.append(list);
        tftpUpload(item.filepath, item.targetDir, item.targetFilename);
        return;
    }
    if (!sourceFile.exists()) {
        QMessageBox::warning(this, tr("Error"), tr("File does not exist"));
        return;
    }
    QDir dir(targetDir);
    QStringList args;
    args << "-i" << "-t1";
    args << address.toString();
    args << "PUT";
    args << sourceFile.filePath();
    args << dir.absoluteFilePath(targetFilename.isEmpty() ? sourceFile.fileName() : targetFilename);
    ui->textEditLog->moveCursor(QTextCursor::End);
    ui->textEditLog->insertHtml("<font color=\"#1D8348\">tftp " + args.join(' ') + "</font><br>");
    tftp_process->setArguments(args);
    if (!tftp_process->open()) QMessageBox::warning(this, tr("Error"), tr("Can not open process"));
}

void MainWindow::on_actionAbout_triggered() {
    QMessageBox::information(this, tr("About"),
                             tr("FPGA digital signal processing experimental platform upload tool V1.0"));
}

void MainWindow::on_actionUpload_firmware_triggered() {
    QString fw_path = QFileDialog::getOpenFileName(this, tr("Select firmware file"),
                                                   QDir::homePath(), "Firmware (BOOT.BIN)");
    if (fw_path.isEmpty()) return;
    tftpUpload(fw_path, "0:/", "BOOT.BIN");
}

void MainWindow::on_pushButtonSetIP_clicked() {
    if (!address.setAddress(ui->lineEditIpAddress->text())) {
        QMessageBox::warning(this, tr("Invalid IP address"), tr("Invalid IP address") + ui->lineEditIpAddress->text());
    }
}

void MainWindow::tftp_process_finished(int exitCode, QProcess::ExitStatus exitStatus) {
    if (exitStatus == QProcess::NormalExit) {
        log_println("<font color=\"#1D8348\">exit code: %d, NormalExit</font>", exitCode);
    } else {
        log_println("<font color=\"#CB4335\">exit code: %d, CrashExit</font>", exitCode);
    }
    tftp_process->close();
    if (!tftpUpload_fifo.isEmpty()) {
        tftp_uploadItem item = tftpUpload_fifo.first();
        tftpUpload(item.filepath, item.targetDir, item.targetFilename);
        tftpUpload_fifo.pop_front();
    }
}

void MainWindow::unzip_process_finished(int exitCode, QProcess::ExitStatus exitStatus) {
    if (exitStatus == QProcess::NormalExit) {
        log_println("<font color=\"#1D8348\">exit code: %d, NormalExit</font>", exitCode);
        tftpUpload(temporaryDir.filePath("unzip"), "0:/");
    } else {
        log_println("<font color=\"#CB4335\">exit code: %d, CrashExit</font>", exitCode);
    }
    unzip_process->close();
}

void MainWindow::udp_readyRead() {
    QHostAddress sender;
    quint16 senderPort;
    QByteArray datagram;
    datagram.resize((int) udpSocket->pendingDatagramSize());
    udpSocket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);
    uint8_t message_id = datagram[0];
    log_printf("<font color=\"#2E86C1\">Device[%s:%d]--></font>", sender.toString().data(), senderPort);
    switch (message_id) {
        case 0: {
            fw_version = QString(datagram.constData() + 1);
            log_println("<font color=\"#D4AC0D\">Firmware version: %s</font>", qUtf8Printable(fw_version));
            update_fw();
            break;
        }
        case 0xff: {
            uint8_t ret_code = datagram[1];
            switch ((UDP_COMM_CMD_CODE) ret_code) {
                case UDP_COMM_ACK:
                    break;
                case UDP_COMM_ERR:
                    log_println("<font color=\"#E53935\">Error: Unknown error</font>");
                    break;
                case UDP_COMM_NO_MSG_ID:
                    log_println("<font color=\"#E53935\">Error: No such message id</font>");
                    break;
            }
        }
    }
}

void MainWindow::udp_sendMsg(uint8_t message_id, const QByteArray &data) {
    QByteArray _data = data;
    _data.push_front((char) message_id);
    udpSocket->writeDatagram(_data, address, 70);
}

void MainWindow::log_printf(const char *fmt, ...) {
    va_list ap;
            va_start(ap, fmt);
    ui->textEditLog->moveCursor(QTextCursor::End);
    ui->textEditLog->insertPlainText(log_buf.join(""));
    log_buf.clear();

    ui->textEditLog->moveCursor(QTextCursor::End);
    ui->textEditLog->insertHtml(QString::vasprintf(fmt, ap));
            va_end(ap);
}

void MainWindow::log_println(const char *fmt, ...) {
    va_list ap;
            va_start(ap, fmt);
    ui->textEditLog->moveCursor(QTextCursor::End);
    ui->textEditLog->insertPlainText(log_buf.join(""));
    log_buf.clear();

    ui->textEditLog->moveCursor(QTextCursor::End);
    ui->textEditLog->insertHtml(QString::vasprintf(fmt, ap) + "<br>");
            va_end(ap);
}

void MainWindow::unzip(const QString &filename) {
    QDir dir(temporaryDir.filePath("unzip"));
    if (dir.exists()) dir.removeRecursively();
    QStringList args = {"x", "-r", "-y", filename, "-o" + temporaryDir.filePath("unzip")};
    unzip_process->setArguments(args);
    unzip_process->setWorkingDirectory(temporaryDir.path());
    ui->textEditLog->insertHtml("<font color=\"#1D8348\">7zr " + args.join(' ') + "</font><br>");
    if (!unzip_process->open()) {
        QMessageBox::warning(this, tr("Error"), tr("Can not open process"));
    }
}

void MainWindow::on_actionCheck_firmware_Update_triggered() {
    QNetworkRequest fw_request, res_request;
    res_list = fw_list = QJsonArray();
    fw_version.clear();
    fw_request.setUrl(QUrl(UPDATE_SERVER"/checkUpdate/firmware"));
    res_request.setUrl(QUrl(UPDATE_SERVER"/checkUpdate/resources"));
    networkAccessManager->get(fw_request);
    networkAccessManager->get(res_request);
    udp_sendMsg(0);
}

void MainWindow::http_request_finished(QNetworkReply *reply) {
    QString url = reply->url().toString();
    QByteArray data;
    if (reply->isReadable())
        data = reply->readAll();
    QVariant ContentDisposition = reply->header(QNetworkRequest::ContentDispositionHeader);
    if (ContentDisposition.isValid()) {
        QString filename = ContentDisposition.toString().mid(21);
        QFileInfo fileInfo(temporaryDir.filePath(filename));
        QFile file(fileInfo.absoluteFilePath());
        file.open(QFile::WriteOnly);
        file.write(data);
        file.close();
        if (filename.endsWith(".7z")) {
            unzip(fileInfo.absoluteFilePath());
        } else {
            tftpUpload(fileInfo.absoluteFilePath(), "0:/", "BOOT.BIN");
        }
    } else {
        if (url.endsWith("/checkUpdate/firmware")) {
            fw_list = QJsonDocument::fromJson(data).array();
            update_fw();
        } else if (url.endsWith("/checkUpdate/resources")) {
            res_list = QJsonDocument::fromJson(data).array();
            update_fw();
        }
    }
}

void MainWindow::update_fw() {
    if (res_list.isEmpty() || fw_list.isEmpty() || fw_version.isEmpty())
        return;
    uint64_t latest_fw_version;
    QJsonObject latest_fw, res, now_fw;
    for (const auto item: fw_list) {
        QJsonObject obj = item.toObject();
        if (latest_fw.isEmpty() ||
            latest_fw.find("version")->toString().toULongLong() <
            obj.find("version")->toString().toULongLong()) {
            latest_fw_version = obj.find("version")->toString().toULongLong();
            latest_fw = obj;
        }
        if (obj.find("version")->toString() == fw_version)
            now_fw = obj;
    }

    for (const auto item: res_list) {
        QJsonObject obj = item.toObject();
        if (obj.find("version")->toInt() == latest_fw.find("res_version")->toInt()) {
            res = obj;
            break;
        }
    }

    if (latest_fw_version <= fw_version.toULongLong()) {
        QMessageBox::information(this, tr("upgrade"), tr("The current version is the latest version"));
    } else {
        QString text = tr("Discover a new version\nfirmware upgrade log:\n");
        text += latest_fw.find("upgrade log")->toString();
        QMessageBox::StandardButton key = QMessageBox::question(this, tr("upgrade"), text);
        if (key == QMessageBox::Yes) {
            QUrl fw_url(UPDATE_SERVER"/downloads/firmware/" + latest_fw.find("filename")->toString());
            QNetworkRequest fw_request(fw_url);
            networkAccessManager->get(fw_request);
            if (latest_fw.find("res_version")->toInt() != now_fw.find("res_version")->toInt()) {
                QUrl res_url(UPDATE_SERVER"/downloads/resources/" + res.find("filename")->toString());
                QNetworkRequest res_request(res_url);
                networkAccessManager->get(res_request);
            }
        }
    }
}

void MainWindow::closeEvent(QCloseEvent *event) {
    if (unzip_process->isOpen() || tftp_process->isOpen()) {
        QMessageBox::warning(this, tr("is running"), tr("A task is currently running"));
        event->ignore();
    } else QWidget::closeEvent(event);
}

