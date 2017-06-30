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
        curContact(nullptr)

{
    ui->setupUi(this);
    ui->lwContacts->setEnabled(false);
    ui_setall(false); //并发的意义时不要让界面失去响应但要防止用户持续按按钮
    connect(with_client, SIGNAL(readyRead()), this, SLOT(dispatch_udp()));
    connect(interval_sendip, SIGNAL(timeout()), this, SLOT(send_ip()));
    ui_setupFriend();
}


ClientProgram::~ClientProgram()
{
    delete ui;
}

void ClientProgram::on_pbLogin_clicked()
{
    connect(with_server, SIGNAL(connected()), this, SLOT(login2()));
    connect(timeout_guard, SIGNAL(timeout()), this, SLOT(withserver_failed()));
    me = Userdata{QUuid::createUuid(), ui->leUser->text(),
                  with_client->localAddress().toString(), with_client->localPort()};
    timeout_guard->start(timeout);
    curServer = QHostAddress(ui->leHost->text());
    with_server->connectToHost(curServer, server_tcp); //
}


void ClientProgram::on_leSendBuffer_returnPressed()
{
    if (curContact->pendingmsg.size()) return;
    QString &txt = curContact->pendingmsg = ui->leSendBuffer->text();
    curContact->try_time = 0;

    if (txt.size() > MAX_MSG_SIZE)
    {
        errmsg("message is too long!");
        txt.resize(0);
        return;
    }
    curContact->sendmsg();
}



void ClientProgram::send_ip()
{
    qDebug() << "send ip called!" << endl;
//注意： 现在keep-alive应该没问题？但是如果防火墙干掉了的话，可以再次打洞，keepalive只是为了保证映射关系不要再发生变化

    interval_sendip->stop();
    with_client->writeDatagram(compose_obj(opcd::RQ_SENDIP, me.session),
                               curServer, server_udp);
    timeout_guard->start(timeout);
}

void ClientProgram::dispatch(QByteArray inputdata, QTcpSocket &so)
{
    with_server->disconnectFromHost();

    QDataStream input(inputdata);
    opcd header;
    input >> header;
    if (header.type == opcd::RSP_LOGIN3_SUCC) return send_ip();

    if (header.type & opcd::CMD_CHANGED)
    {
        QVector<Operation> op;
        input >> op;
        for (auto x : op)
        {
            if (x.type == x.ADD || x.type == x.MOD)
            {
                Friend_ &pt = **usertb.insert(x.data.session,
                                             new Friend_(x.data, this));//insert会有replace行为
//现在有modify操作，那么如果发生了modify操作，则会清空聊天记录！

                if (x.type == x.ADD) ui_add(pt);
            } else if (x.type == x.DEL)
            {
                auto y = usertb.find(x.data.session);
                assert(y != usertb.end());
                ui_del(**y);
                if (*y == curContact) curContact = nullptr;
                usertb.remove(x.data.session);
            } else
                assert(false);

        }
        ui_setupFriend();
        version += op.size();
    }
    if (header.type & opcd::CMD_PUNCH)
    {
        QList<QUuid> pend;
        input >> pend;
        for (const auto &x : pend)
        {
            auto it = usertb.find(x);
            assert(it != usertb.end());
            with_client->writeDatagram(compose_obj(opcd::MSG_PUNCH, me.session),
                                       QHostAddress((*it)->ipaddr), (*it)->port);
        }
    }
    interval_sendip->start(refresh);
    ui_setall(true);
}

void ClientProgram::login2()
{
    qDebug() << "login2 called!" << endl;
    timeout_guard->stop();
    disconnect(with_server, SIGNAL(connected()), nullptr, nullptr);
    with_server->write(compose_obj(opcd::RQ_LOGIN, me));
    new read_first(with_server, this); // 下一步在dispatch-login3
}

//msg : 就是一个header+ uuid + msg

void ClientProgram::dispatch_udp()
{
    QNetworkDatagram dg = with_client->receiveDatagram();
    QDataStream input(dg.data());
    opcd head; input >> head;
    if (head.type == opcd::RSP_IP_RECEIVED)
    {
        timeout_guard->stop();
        qDebug() << "IP sent!" << endl;
        fetch();
    } else
    {
        QUuid senderID;
        input >> senderID;
        auto it = usertb.find(senderID);
        if (it == usertb.end())
            qDebug() << "user doesn't exist! maybe it has  already logged out!" << endl;
        if (head.type == opcd::MSG_REALMSG)
        {
            QString str; input >> str;
            curContact = *it;
            curContact->addmsg(str, "\n");
            ui_setupFriend();
            with_client->writeDatagram(compose_obj(opcd::MSG_OK, me.session),
                                       dg.senderAddress(), dg.senderPort());
        } else if (head.type == opcd::MSG_PUNCH) (*it)->sendmsg();
        else if (head.type == opcd::MSG_OK) (*it)->ok();
        else assert(false);
    }
}

void ClientProgram::withserver_failed()
{
    with_server->disconnectFromHost();
    errmsg("error when connecting with server!check your network and try again");
    timeout_guard->stop();
}

void ClientProgram::fetch()
{
    //先建立TCP连接，再处理接收到的数据，最后settimeout下一次fetch

    connect(with_server, SIGNAL(connected()), this, SLOT(fetch2()));
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

