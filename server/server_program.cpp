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
        new server_msg(server->nextPendingConnection(), this);
}


void ServerProgram::ui_add(const Userdata& ud)
{
    usertb_ui[ud.session] = new QListWidgetItem(ud.username, ui->listWidget);
}
void ServerProgram::ui_del(const QUuid& user)
{
    auto x = usertb_ui.find(user);
    assert(x != usertb_ui.end()); //每个用户理论上只能下线一次
    delete *x;
    usertb_ui.remove(user);
}

void ServerProgram::dispatch_udp()
{
    while (punching->bytesAvailable())
    {
        QNetworkDatagram nd = punching->receiveDatagram();
        QDataStream stm(nd.data());
        opcd c; stm >> c;
        qDebug() << "udp: new pack arrived" << endl;
        if (c.type & opcd::RQ_POSTLOGIN)
        {
            QUuid us; stm >> us;
            qDebug() << "postlogin: user = " << us << endl;
            auto it = usertb.find(us);
            assert(it != usertb.end());
            it->ipaddr = nd.senderAddress().toString();
            it->port = (uint16_t)nd.senderPort();
            changes.append(Operation{Operation::ADD, *it});
            punching->write(compose_obj(opcd::RSP_LOGINFIN));
        }
        if (c.type & opcd::RQ_NEEDPUNC)
        {
            QUuid us, ur; stm >> us >> ur; //自己的身份和他人的身份
            auto  y = puncreq_tb.find(ur);
            qDebug() << "needpunc: req from" << us << " to " << ur << endl;
            if (y == puncreq_tb.end())
            {
                qDebug() << "needpunching: target doesn't exist" << endl;
                return;
            }
            //附加信息
            y->append(us);
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
        Userdata ud;
        input >> ud;
        usertb[ud.session] = ud;
        ui_add(ud);
        so.write(compose_obj(opcd::RSP_OK));
    }
    if (header.type & opcd::RQ_LOGOUT)
    {
        QUuid id;
        input >> id;
/*
void ServerProgram::logout(QUuid id, QTcpSocket & sk)
{
    //TODO 这里重写完删掉
    assert(usertb.find(id) != usertb.end());
    sk.write(compose_obj(opcd::RSP_OK));
    auto y = usertb_ui[id];
    delete y;
    usertb_ui.remove(id);
    usertb.remove(id);
}
 */
    }
    if (header.type & opcd::RQ_FETCH)
    {
        QByteArray outputdata;
        QDataStream output(outputdata);
        QUuid id;
        quint64 version;
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
        auto y = puncreq_tb.find(id);
        if (y != puncreq_tb.end() && y->size() != 0)
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

