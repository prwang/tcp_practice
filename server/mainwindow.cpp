#include "mainwindow.h"
#include "../shared.h"

MainWindow::MainWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainWindow),
    server(new QTcpServer(this))
{
    ui->setupUi(this);
    server->listen(QHostAddress::Any, Ports::server_tcp);
    connect(server, SIGNAL(server->newConnection()), this, SLOT(acceptConnection()));
}


MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::acceptConnection() { new server_msg(server->nextPendingConnection(), this); }


void MainWindow::login(Userdata ud, QTcpSocket& sk)
{

    usertb[ud.session] = ud;
    usertb_ui[ud.session] = new QListWidgetItem(ud.username, ui->listWidget);
    write_close(opcd::RSP_OK, sk, usertb);
}

void MainWindow::getlist(QUuid id, QTcpSocket & sk)
{
    auto x = usertb.find(id);
    if (x == usertb.end()) write_close(opcd::RSP_NOTFOUND, sk, emptyobj()); //这种情况是严重错误
    else write_close(opcd::RSP_OK, sk, usertb);
}

void MainWindow::logout(QUuid id, QTcpSocket & sk)
{
    if (usertb.find(id) == usertb.end()) write_close(opcd::RSP_NOTFOUND, sk, emptyobj()); //这种情况是严重错误
    else
    {
        write_close(opcd::RSP_OK, sk, emptyobj());
        auto y = usertb_ui[id];
        delete y;
        usertb_ui.remove(id);
        usertb.remove(id);
    }
}
