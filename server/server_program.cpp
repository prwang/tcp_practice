#include <QtWidgets/QListWidgetItem>
#include "server_program.h"
ServerProgram::ServerProgram(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ServerProgram),
    server(new QTcpServer(this)),
    punching(new QUdpSocket(this))
{
    ui->setupUi(this);
    server->listen(QHostAddress::Any, Ports::server_tcp);
    punching->bind(QHostAddress::Any, Ports::server_udp);
    connect(server, SIGNAL(newConnection()), this, SLOT(acceptConnection()));
    connect(punching, SIGNAL(bytesAvailable()), this, SLOT(dispatch_udp()));
}


ServerProgram::~ServerProgram()
{
    delete ui;
}

void ServerProgram::acceptConnection()
{
    while (server->hasPendingConnections())
        new read_first(server->nextPendingConnection(), this);
}


void ServerProgram::ui_add(const Userdata& ud)
{
    usertb[ud.session].disp
            = new QListWidgetItem(ud.username, ui->listWidget);
}
void ServerProgram::ui_del(const QUuid& user)
{
    auto x = usertb.find(user);
    assert(x != usertb.end()); //每个用户理论上只能下线一次
    delete x->disp;
}

void ServerProgram::dispatch_udp()
{
    while (punching->bytesAvailable())
    {
        QNetworkDatagram nd = punching->receiveDatagram();
        QDataStream stm(nd.data());
        opcd c; stm >> c;
        qDebug() << "udp: new pack arrived" << endl;
        if (c.type & opcd::RQ_SENDIP)
        {
            QUuid us; stm >> us;
            qDebug() << "postlogin: user = " << us << endl;
            auto it = usertb.find(us);
            assert(it != usertb.end());
            if (it->port != nd.senderPort() || it->ipaddr != nd.senderAddress().toString())
            {
                qDebug() << tr("ip changed: user=%1, from %2:%3 to %4:%5").arg(us.toString())
                        .arg(it->ipaddr).arg(it->port)
                        .arg(nd.senderAddress().toString()).arg(nd.senderPort()) << endl;

                it->ipaddr = nd.senderAddress().toString();
                it->port = (quint16) nd.senderPort();
                changes.append(Operation{Operation::MOD, *it});
            }
            punching->writeDatagram(compose_obj(opcd::RSP_IP_RECEIVED),
                                    nd.senderAddress(), (quint16)nd.senderPort());
        }
        if (c.type & opcd::RQ_NEEDPUNC)
        {
            QUuid us, ur; stm >> us >> ur; //自己的身份和他人的身份
            auto  y = usertb.find(ur);
            qDebug() << "needpunc: req from" << us << " to " << ur << endl;
            if (y == usertb.end())
            {
                qDebug() << "needpunching: target doesn't exist" << endl;
                return;
            }
            //附加信息
            y->puncreq.append(us);
            punching->write(compose_obj(opcd::RSP_LISTADD));
        }
    }
}


void ServerProgram::dispatch(QByteArray inputdata, QTcpSocket& so)
{
    connect(&so, SIGNAL(disconnected()), this, SLOT(cleanup()));
    QDataStream input(inputdata);
    opcd header;
    input >> header;
    if (header.type & opcd::RQ_LOGIN)
    {
        Userdata ud; input >> ud;
        changes.append(Operation{Operation::ADD, usertb[ud.session] = ud });
        ui_add(ud);
        so.write(compose_obj(opcd::RSP_LOGIN3_SUCC));
    }
    if (header.type & opcd::RQ_LOGOUT)
    {
        QUuid id;
        input >> id;
        assert(usertb.find(id) != usertb.end());
        ui_del(id);
        usertb.remove(id);
    }
    if (header.type & opcd::RQ_FETCH)
    {//回传：更改列表 + 等待的打洞请求　+　别人完成的打洞请求
        QByteArray outputdata;
        QDataStream output(outputdata);
        QUuid id;
        unsigned version;
        input >> id >> version;
        auto x = usertb.find(id);
        if (x == usertb.end())
        {
            qDebug() << "user doesn't exist now" << endl;

        }
        unsigned flag = 0;
        inputdata.resize(0);
        if (version < changes.size())
        {
            flag |= opcd::CMD_CHANGED;
            QVector<Operation> wr(changes.size() - version);
            std::copy(changes.begin() + version, changes.end(), wr.begin());
            output << wr;
        }
        auto y = usertb.find(id);
        if (y != usertb.end() && y->puncreq.size() != 0)
        {
            flag |= opcd::CMD_PUNCH;
            QList<QUuid> pend;
            pend.swap(*y);
            output << pend;
        }
    } //打洞请求用UDP发吧
    end: ;
}

void ServerProgram::cleanup()
{
    QTcpSocket* sk = qobject_cast<QTcpSocket*>(QObject::sender());
    if (sk) sk->deleteLater();
}

