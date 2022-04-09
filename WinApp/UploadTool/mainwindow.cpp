#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <stdarg.h>
#include <QFileDialog>
#include <QHostAddress>
#include <QMessageBox>
#include <QNetworkAccessManager>

MainWindow::MainWindow(QWidget *parent)
        : QMainWindow(parent), ui(new Ui::MainWindow),
          process(new QProcess(this)), udpSocket(new QUdpSocket(this)) {
    ui->setupUi(this);
    process->setProgram(QFileInfo("./tftp.exe").absoluteFilePath());
    connect(process, SIGNAL(readyReadStandardOutput()),
            this, SLOT(Process_readyReadOutput()));
    connect(process, SIGNAL(finished(int, QProcess::ExitStatus)),
            this, SLOT(Process_finished(int, QProcess::ExitStatus)));
    ui->textEditLog->document()->setMaximumBlockCount(200);
    on_pushButtonSetIP_clicked();
    udpSocket->bind(70);
    connect(udpSocket, SIGNAL(readyRead()),
            this, SLOT(udp_readyRead()));
}

MainWindow::~MainWindow() {
    delete ui;
    delete process;
    delete udpSocket;
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
    while (process->bytesAvailable()) {
        ui->textEditLog->moveCursor(QTextCursor::End);
        ui->textEditLog->insertPlainText(QString::fromLocal8Bit(process->readLine()));
    }
}

void MainWindow::tftpUpload(const QString &filepath, const QString &targetDir, const QString &targetFilename) {
    QFileInfo sourceFile(filepath);
    if (!sourceFile.exists()) {
        QMessageBox::warning(this, tr("Error"), tr("File does not exist"));
        return;
    }
    QDir dir(targetDir);
    QStringList args;
    args << "-i" << "-v" << "-t1";
    args << address.toString();
    args << "PUT";
    args << sourceFile.filePath();
    args << dir.absoluteFilePath(targetFilename.isEmpty() ? sourceFile.fileName() : targetFilename);
    ui->textEditLog->moveCursor(QTextCursor::End);
    ui->textEditLog->insertHtml("<font color=\"#1D8348\">tftp " + args.join(' ') + "</font><br>");
    process->setArguments(args);
    if (!process->open()) {
        QMessageBox::warning(this, tr("Error"), tr("Can not open process"));
    }
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

void MainWindow::Process_finished(int exitCode, QProcess::ExitStatus exitStatus) {
    if (exitStatus == QProcess::NormalExit) {
        log_println("<font color=\"#1D8348\">exit code: %d, NormalExit</font>", exitCode);
    } else {
        log_println("<font color=\"#CB4335\">exit code: %d, CrashExit</font>", exitCode);
    }
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
            log_println("<font color=\"#D4AC0D\">Firmware version: %s</font>", datagram.constData() + 1);
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

void MainWindow::on_actionChick_firmware_Update_triggered() {
    udp_sendMsg(0);
}

void MainWindow::log_printf(const char *fmt, ...) {
    va_list ap;
            va_start(ap, fmt);
    ui->textEditLog->moveCursor(QTextCursor::End);
    ui->textEditLog->insertHtml(QString::vasprintf(fmt, ap));
            va_end(ap);
}

void MainWindow::log_println(const char *fmt, ...) {
    va_list ap;
            va_start(ap, fmt);
    ui->textEditLog->moveCursor(QTextCursor::End);
    ui->textEditLog->insertHtml(QString::vasprintf(fmt, ap) + "<br>");
            va_end(ap);
}

