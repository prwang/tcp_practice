#include "client_program.h"
#include "../shared.h"
ClientProgram::ClientProgram(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ClientProgram),
    with_client(new QUdpSocket(this)),
    with_server(new QTcpSocket(this)),
    timeout_guard(new QTimer(this)), //TIMER 所有需要回复的UDP包，和TCP连接的建立
    interval_sendip(new QTimer(this)),
    version(0),
    me{QUuid::createUuid(), ui->leUser->text(), with_client->localAddress().toString(), with_client->localPort() }
{
    ui->setupUi(this);
    ui->lwContacts->setEnabled(false);
    connect(with_client, SIGNAL(readyRead()), this, SLOT(dispatch_udp()));
}


ClientProgram::~ClientProgram()
{
    delete ui;
}
void ClientProgram::on_pbLogin_clicked()
{
    connect(with_server, SIGNAL(connected()), this, SIGNAL(login2()));
    connect(timeout_guard, SIGNAL(timeout()), this, SLOT(withserver_failed()));
    timeout_guard->start(timeout);
    curServer = QHostAddress(ui->leHost->text());
    with_server->connectToHost(curServer, server_tcp); //
    //并发的意义时不要让界面失去响应
    //但要防止用户持续按按钮
    ui_setall(false);
}

#define errmsg(x) do {  QMessageBox::critical(this, tr("错误"), tr(x)); return; } while (false)
void ClientProgram::on_leSendBuffer_returnPressed()
{
    //TODO 根据有没有洞决定怎么发包，见spec，先判断消息长度
}

void ClientProgram::send_ip()
{
    with_client->writeDatagram(compose_obj(opcd::RQ_SENDIP, me.session),
                               curServer, server_udp);
    timeout_guard->start(timeout);
}

void ClientProgram::dispatch(QByteArray inputdata, QTcpSocket& so)
{
    with_server->disconnectFromHost();

    QDataStream input(inputdata); opcd header; input >> header;
    if (header.type == opcd::RSP_LOGIN3_SUCC) return send_ip();

    if (header.type & opcd::CMD_CHANGED)
    {
        QVector<Operation> op; input >> op;
        for (auto x : op)
        {
            if (x.type == x.ADD)  usertb[x.data.session] = x.data;
            else if (x.type == x.DEL) usertb.remove(x.data.session);
            else assert(false);
        }
        version += op.size();
    }
    if (header.type & opcd::CMD_PUNCH)
    {
        QList<QUuid> pend; input >> pend;
        for (const auto& x : pend)
        {
            auto it = usertb.find(x);
            assert(it != usertb.end());
            with_client->writeDatagram(
                    compose_obj(opcd::MSG_PUNCH), QHostAddress(it->ipaddr), it->port);
        }
    }
}
void ClientProgram::login2()
{
    timeout_guard->stop();
    disconnect(with_server, SIGNAL(connected()), nullptr, nullptr);
    with_server->write(compose_obj(opcd::RQ_LOGIN,  me));
    disconnect(with_server, SIGNAL(connected()), this, SLOT(login2()));
    new read_first(with_server, this); // 下一步在dispatch-login3
}
void ClientProgram::dispatch_udp()
{
    QNetworkDatagram dg = with_client->receiveDatagram();
    const opcd& head = *(const opcd*)dg.data().data();

    switch (head.type)
    {
    case opcd::RSP_IP_RECEIVED:

        timeout_guard->stop();
        fetch();
        ui_setall(true);
        //TODO setinterval(下一次)
        break;
    case opcd::MSG_REALMSG:
        break; //TODO 新消息的相关行为
    case opcd::MSG_PUNCH:
        break; //TODO 要不要处理打洞包？要的，立刻发回真实数据
    case opcd::MSG_PUNCHPREPARE:
        qDebug() << "这不是port restricted型nat" << dg.senderAddress() << endl;
        break;
    default: assert(false);
    }
}

void ClientProgram::withserver_failed()
{
    with_server->disconnectFromHost();
    errmsg("与服务器连接失败，检查网络并重试！");
}
void ClientProgram::fetch()
{
    //先建立TCP连接，再处理接收到的数据，最后settimeout下一次fetch
    connect(with_server, SIGNAL(connected()), this, SIGNAL(fetch2()));
    connect(timeout_guard, SIGNAL(timeout()), this, SLOT(withserver_failed()));
    timeout_guard->start(timeout);
    curServer = QHostAddress(ui->leHost->text());
    with_server->connectToHost(curServer, server_tcp); //
}
void ClientProgram::fetch2()
{
    timeout_guard->stop();
    disconnect(with_server, SIGNAL(connected()), nullptr, nullptr);
    with_server->write(compose_obj(opcd::RQ_FETCH, me.session, version));
    new read_first(with_server, this); //下一次在tcp的dispatch
}


