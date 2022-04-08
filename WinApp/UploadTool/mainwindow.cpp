#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QFileDialog>
#include <QHostAddress>
#include <QMessageBox>
#include <QNetworkAccessManager>

MainWindow::MainWindow(QWidget *parent)
        : QMainWindow(parent), ui(new Ui::MainWindow), process(new QProcess(this)) {
    ui->setupUi(this);
    process->setProgram(QFileInfo("./tftp.exe").absoluteFilePath());
    connect(process, SIGNAL(readyReadStandardOutput()),
            this, SLOT(Process_readyReadOutput()));
    ui->textEditLog->document()->setMaximumBlockCount(200);
}

MainWindow::~MainWindow() {
    delete ui;
    delete process;
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

void MainWindow::tftpUpload(const QString &filepath, const QString &targetDir) {
    QHostAddress address;
    if (!address.setAddress(ui->lineEditIpAddress->text())) {
        QMessageBox::warning(this, tr("Invalid IP address"),
                             tr("Invalid IP address") + ui->lineEditIpAddress->text());
        return;
    }
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
    args << dir.absoluteFilePath(sourceFile.fileName());
    ui->textEditLog->moveCursor(QTextCursor::End);
    ui->textEditLog->insertHtml("<font color=\"#00CC00\">tftp " + args.join(' ') + "</font>\n");
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
    tftpUpload(fw_path, "0:/");
}
