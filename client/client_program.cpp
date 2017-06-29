#include "client_program.h"
#include "../shared.h"
ClientProgram::ClientProgram(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ClientProgram),
    with_client(new QUdpSocket(this)),
    curContact(nullptr),
    with_server(new QTcpSocket(this)),
    tm(new QTimer(this))
{
    ui->setupUi(this);
    ui->lwContacts->setEnabled(false);
    connect(with_client, SIGNAL(readyRead()), this, SLOT(dispatch_udp()));
    me = new Userdata{ QUuid::createUuid(), ui->leUser->text(), "0.0.0.0", with_client->localPort() };
}


ClientProgram::~ClientProgram()
{
    delete ui;
}
void ClientProgram::on_pbLogin_clicked()
{
    connect(with_server, SIGNAL(connected()), this, SIGNAL(login_send()));
    connect(tm, SIGNAL(timeout()), this, SLOT(login_failed()));
    curServer = QHostAddress(ui->leHost->text());
    with_server->connectToHost(curServer, server_tcp);
}

#define errmsg(x) do {  QMessageBox::critical(this, tr("错误"), tr(x)); return; } while (false)
void ClientProgram::on_leSendBuffer_returnPressed()
{
    //TODO 根据有没有洞决定怎么发包
}

void ClientProgram::login_send()
{
    tm->stop();
    with_server->write(compose_obj(opcd::RQ_LOGIN,  me));
    disconnect(with_server, SIGNAL(connected()), this, SLOT(login_send()));
    new login_recv(with_server, this);
}

void ClientProgram::login_permitted()
{
    QByteArray dt = compose_obj(opcd::RQ_POSTLOGIN, me->session);
    with_client->writeDatagram(dt, curServer, server_udp);
    tm->start(timeout);
}
void ClientProgram::dispatch_udp()
{
    QNetworkDatagram dg = with_client->receiveDatagram();
    const opcd& head = *(const opcd*)dg.data().data();
    switch (head.type)
    {
    case opcd::RSP_LOGINFIN:
        tm->stop(); fetch();
        break;
    case opcd::MSG_REALMSG:
        break; //TODO 新消息的相关行为
    case opcd::MSG_PUNCH:
        break; //TODO 要不要处理打洞包？
    default: assert(false);
    }
}

void ClientProgram::login_failed()
{
    with_server->disconnectFromHost();
    errmsg("连接失败，检查网络并重试！");
}
void ClientProgram::fetch()
{

}


