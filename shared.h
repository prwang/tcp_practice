#pragma once

#include <QMainWindow>

#include <QtGui/QtGui>
#include <QtNetwork/QUdpSocket>
#include <QtNetwork/QTcpServer>
#include <QtNetwork/QTcpSocket>
#include <QDataStream>
#include <QMessageBox>
#include <QUuid>

enum Ports : quint16
{
    server_tcp = 23334,
    server_udp = 23335,

};
constexpr int MAX_UDP_SIZE = 1200;
struct opcd
{
    enum msgType : unsigned
    {
        RSP_OK =                   0b1,
        RSP_NOTFOUND =            0b10,

        RQ_LOGIN =               0b100,
        RQ_LOGOUT =             0b1000,
        RQ_GETLIST =           0b10000,

        TR_NOTIF =           0b1000000,
        TR_KEEP =           0b10000000

    } type;
    unsigned length;
    explicit operator int() { return length; }

    friend QDataStream &operator>>(QDataStream &op, opcd& s)
    {
        op >> (unsigned&)s.type >> s.length;
        return op;
    }
    friend QDataStream &operator<<(QDataStream &op, const opcd& s)
    {
        op << (unsigned&)s.type << s.length;
        return op;
    }
    void Ins(QDataStream& op)
    {
        op.device()->seek(0);
        op << *this;
    }
};

template<class Header> //header：定长文件头，含有一个operator quint64()表示余下部分文件大小
struct object_with_header { Header header; };
struct read_first : public QObject,
                    public object_with_header<opcd>
{
Q_OBJECT
protected:
    QTcpSocket *conn;
    QByteArray br;
    quint64 received;
    bool is_done, is_error;
    explicit read_first(QTcpSocket *_conn, QObject *parent = nullptr)
            :  QObject(parent), received(0), is_done(false), is_error(false), conn(_conn), br{}
    {
        connect(conn, SIGNAL(readyRead()), this, SLOT(Read()));
        connect(conn, SIGNAL(error()), this, SLOT(Error()));
        connect(conn, SIGNAL(disconnected()), this, SLOT(Disconnected()));
    }
    virtual void after_read_done() = 0;
public slots:
    void Read()
    {
        if (conn->bytesAvailable() <= 0) return; // ?
        if (received < sizeof(header))
            received += conn->read((char*)&header + received, sizeof(header) - received);
        else
        {
            if (!br.size()) br.resize(int(header));
            quint64 rr = received - sizeof(header),
                    cn = int(header) - rr;
            if (cn > 0) received += conn->read(br.data() + rr, cn);
            else { is_done = true;  after_read_done(); }
        }
    }
    void Error() { is_error = true; }
    void Disconnected() { if (!is_done) is_error = true; };
};

struct Userdata
{

    QUuid session;
    QString username, ipaddr;
    quint16 port;
    //TODO: diff+传patch，dtl比较好用 quint64 dataVersion;
    friend QDataStream &operator>>(QDataStream &op, Userdata &s)
    {
        op >> s.session >>  s.username >> s.ipaddr >> s.port;
        return op;
    }

    friend QDataStream &operator<<(QDataStream &op, Userdata &s)
    {
        op << s.session << s.username << s.ipaddr << s.port;
        return op;
    }
};



template<class T> inline void write_close(opcd::msgType ty, QTcpSocket &sk, const T &object)
{
    QByteArray ar; QDataStream st(&ar, QIODevice::ReadWrite);
    st << object;
    (opcd{ty, (unsigned)ar.size()}).Ins(st);
    sk.write(ar); sk.close();
}

struct emptyobj
{
    friend QDataStream &operator>>(QDataStream &op, emptyobj& s) { return op; }
    friend QDataStream &operator<<(QDataStream &op, const emptyobj& s) { return op; }
};



struct changes
{

};
class userdataall : QHash<QUuid, Userdata>
{
};
