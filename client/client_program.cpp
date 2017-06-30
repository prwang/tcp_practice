#include "client_program.h"
#include "../shared.h"

ClientProgram::ClientProgram(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::ClientProgram),
        with_client(new QUdpSocket(this)),
        with_server(new QTcpSocket(this)),
        timeout_guard(new QTimer(this)), //TIMER 所有需要回复的UDP包，和TCP连接的建立
        interval_sendip(new QTimer(this)),
        version(0)
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
    connect(with_server, SIGNAL(connected()), this, SIGNAL(login2()));
    connect(timeout_guard, SIGNAL(timeout()), this, SLOT(withserver_failed()));
    me = Userdata{QUuid::createUuid(), ui->leUser->text(),
                  with_client->localAddress().toString(), with_client->localPort()};
    timeout_guard->start(timeout);
    curServer = QHostAddress(ui->leHost->text());
    with_server->connectToHost(curServer, server_tcp); //
}

#define errmsg(x) do {  QMessageBox::critical(this, tr("错误"), tr(x)); return; } while (false)

void ClientProgram::on_leSendBuffer_returnPressed()
{
    QString &txt = curContact->pendingmsg = ui->leSendBuffer->text();
    curContact->try_time = 0;
    if (txt.size() > MAX_MSG_SIZE)
    {
        errmsg("消息过长！");
        txt.resize(0);
        return;
    }
    ui_display_local_msg(txt);
    curContact->sendmsg();
}

void Friend_::sendmsg()
{
    ++try_time;
    guard->stop();
    ClientProgram *cp = qobject_cast<ClientProgram *>(parent());
    cp->with_client->writeDatagram(
            compose_obj(opcd::MSG_REALMSG, cp->me.session, pendingmsg),
            QHostAddress(ipaddr), port);
    guard->start(timeout);
}

void Friend_::send_timeout()
{
    ClientProgram *cp = qobject_cast<ClientProgram *>(parent());
    if (try_time < 2)
    {


    } else cp->ui_send_error();
}

void Friend_::ok()
{
    guard->stop();
    ClientProgram *cp = qobject_cast<ClientProgram *>(parent());
    cp->ui_display_local_msg(pendingmsg);
    pendingmsg.resize(0);
}

void ClientProgram::send_ip()
{
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
                Friend_ &pt = *usertb.insert(x.data.session, Friend_(x.data));//insert会有replace行为
                if (x.type == x.ADD) ui_add(pt);
            } else if (x.type == x.DEL)
            {
                auto y = usertb.find(x.data.session);
                assert(y != usertb.end());
                ui_del(*y);
                if (y == curContact) curContact = usertb.end();
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
                                       QHostAddress(it->ipaddr), it->port);
        }
    }
    interval_sendip->start(refresh);
}

void ClientProgram::login2()
{
    timeout_guard->stop();
    disconnect(with_server, SIGNAL(connected()), nullptr, nullptr);
    with_server->write(compose_obj(opcd::RQ_LOGIN, me));
    disconnect(with_server, SIGNAL(connected()), this, SLOT(login2()));
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
        fetch();
        ui_setall(true);
    } else
    {
        QUuid senderID;
        input >> senderID;
        auto it = usertb.find(senderID);
        if (it == usertb.end())
            qDebug() << "user doesn't exist! maybe it has  already logged out!" << endl;
        if (head.type == opcd::MSG_REALMSG)
        {
            QString str;
            input >> str;
            ui_display_remote_msg(*it, str);
            with_client->writeDatagram(compose_obj(opcd::MSG_OK, me.session),
                                       dg.senderAddress(), dg.senderPort());
        } else if (head.type == opcd::MSG_PUNCH) it->sendmsg();
        else if (head.type == opcd::MSG_OK) it->ok();
        else assert(false);
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

void ClientProgram::ui_setupFriend()
{ //就是curContact里面的那个Friend_

    //首先若curContact是end()，则设置有右边为不可用
    if (curContact == usertb.end())
    {
        ui->enableCtrl->setDisabled(true);
        return;
    }

    /* TODO

    if (有没发出去的消息)
       禁止再发消息;
    填充那几个label;
    填充历史纪录;
     FIXME 会不会出现用户手速太快在禁用之前已经发出去了一个回车信号？
      */
}

void ClientProgram::ui_setall(bool enable)
{
    ui->splitter->setEnabled(enable);
}

void ClientProgram::ui_display_local_msg(const QString &msg)
{
    //TODO 在当前的textedit追加一段文字
}

void ClientProgram::ui_display_remote_msg(const Friend_ &peerid, const QString &msg)
{
    //TODO 设置窗口焦点( = setCurContact + up_setupFriend_)，并且把文字追加到后面
}

void ClientProgram::ui_add(Friend_ &fr)
{
    //TODO 维护列表框，特判不显示自己，把每一个的点击信号绑定给设置窗口焦点
    //还是应该在Friend_里面放一个ListItem*
}

void ClientProgram::ui_del(Friend_ &)
{

}
