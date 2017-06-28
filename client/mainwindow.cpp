#include "mainwindow.h"
#include "../shared.h"
MainWindow::MainWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainWindow),
    sender(new QUdpSocket(this)),
    curContact(nullptr)
{
    ui->setupUi(this);
}


MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::on_pbLogin_clicked()
{
    //udp必须用udp打洞！
    //port restrict 可以从**发**过的端口**收信息
    //上线时**广播所有**地址
    QString host = ui->leHost->text(), user = ui->leUser->text();


}

#define errmsg(x) do {  QMessageBox::critical(this, tr("错误"), tr(x)); return; } while (false)
void MainWindow::on_leSendBuffer_returnPressed()
{
    QByteArray msg = ui->leSendBuffer->text().toUtf8();
    if (msg.size() > MAX_UDP_SIZE) errmsg("消息过长");
    else if (!curContact) errmsg("请选择好友");

    //TODO 通知对面开始bind? 对面已经bind

    sender->writeDatagram(msg.data(), msg.size(),
                          *curContact, 45454);

}



