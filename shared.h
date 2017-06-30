#pragma once

#include <QMainWindow>
#include <QtNetwork/QNetworkDatagram>
#include <QtGui/QtGui>
#include <QtNetwork/QUdpSocket>
#include <QtNetwork/QTcpServer>
#include <QtNetwork/QTcpSocket>
#include <QDataStream>
#include <QMessageBox>
#include <QListWidget>
#include <QListWidgetItem>
#include <cassert>
#include <QUuid>
#include <chrono>

enum Ports : quint16
{
    server_tcp = 23334,
    server_udp = 23335,

};
constexpr int MAX_MSG_SIZE = 200;

struct opcd
{
    enum msgType : unsigned
    {

        RSP_LOGIN3_SUCC = 1 << 0,
        RSP_LISTADD = 1 << 2,
        RSP_IP_RECEIVED = 1 << 4,

        RQ_LOGIN = 1 << 6,
        RQ_LOGOUT = 1 << 8,
        RQ_FETCH = 1 << 10,
        RQ_SENDIP = 1 << 12,
        RQ_NEEDPUNC = 1 << 14,


        CMD_PUNCH = 1 << 16,
        CMD_CHANGED = 1 << 18,
        MSG_REALMSG = 1 << 20,
        MSG_PUNCH = 1 << 22,

        MSG_OK = 1 << 24, //收到了发来的消息

    };
    unsigned type, length;

    explicit operator int() { return length; }

    friend QDataStream &operator>>(QDataStream &op, opcd &s)
    {
        op >> (unsigned &) s.type >> s.length;
        return op;
    }

    friend QDataStream &operator<<(QDataStream &op, const opcd &s)
    {
        op << (unsigned &) s.type << s.length;
        return op;
    }

    void Ins(QByteArray &ar)
    {
         QDataStream op{&ar, QIODevice::ReadWrite};
        op.device()->seek(0);
        op << *this;
    }
};

template<class Header> //header：定长文件头，含有一个operator quint64()表示余下部分文件大小
struct object_with_header
{
    Header header;
};

struct read_first : public QObject,
                    public object_with_header<opcd>
{
Q_OBJECT
protected:
    QTcpSocket *conn;
    QByteArray br;
    quint64 received;
    bool is_done, is_error, proceeding;


    void handleerror()
    {
        emit fail();
        conn->close();
        deleteLater();
    }
public:
    explicit read_first(QTcpSocket *_conn, QObject *parent = nullptr)
            : QObject(parent), received(0), is_done(false), is_error(false), conn(_conn), br{}
    {
        connect(conn, SIGNAL(error()), this, SLOT(Error()));
        connect(conn, SIGNAL(disconnected()), this, SLOT(Disconnected()));
        connect(this, SIGNAL(success(QByteArray, QTcpSocket*)), parent, SLOT(dispatch(QByteArray, QTcpSocket*)));
        connect(conn, SIGNAL(readyRead()), this, SLOT(Read()));
        proceeding = true;
    }

public slots:

    void Read()
    {
        if (received < sizeof(header))
            received += conn->read((char *) &header + received, sizeof(header) - received);
        else
        {
            if (!br.size()) br.resize(int(header));
            quint64 rr = received - sizeof(header),
                    cn = int(header) - rr;
            if (cn > 0) received += conn->read(br.data() + rr, cn);
            else
            {
                is_done = true;
                emit success(br, *conn);
                conn->close();
                deleteLater();
            }
        }
    }

    void Error() { proceeding = false; emit fail(); conn->close();   }
    void Disconnected() { if (!is_done && proceeding) handleerror(); };
signals:
    void success(QByteArray, QTcpSocket&);
    void fail();
};

struct Userdata
{

    QUuid session;
    QString username, ipaddr;
    quint16 port;

    friend QDataStream &operator>>(QDataStream &op, Userdata &s)
    {
        op >> s.session >> s.username >> s.ipaddr >> s.port;
        return op;
    }

    friend QDataStream &operator<<(QDataStream &op, const Userdata &s)
    {
        op << s.session << s.username << s.ipaddr << s.port;
        return op;
    }
};

template<class T> inline QByteArray __make__(const T& t1)
{
    QByteArray ar;
    QDataStream st(&ar, QIODevice::ReadWrite);
    st << t1;
    return ar;
}
template<class T, class ...Ts> inline QByteArray __make__(const T& t1,  const Ts&... T2)
{
    QByteArray ar;
    QDataStream st(&ar, QIODevice::ReadWrite);
    st << t1 << __make__(T2...);
    return ar;
}

template<class ...Ts>
inline QByteArray compose_obj(opcd::msgType ty,  const Ts&... object)
{
    QByteArray ar = __make__(object...);
    (opcd{ty, (unsigned) ar.size()}).Ins(ar);
    return ar;
}

inline QByteArray compose_obj(opcd::msgType ty)
{
    QByteArray ar;
    (opcd{ty, (unsigned) ar.size()}).Ins(ar);
    return ar;
}


struct Operation
{
    enum
    {
        ADD = 1,
        DEL = 2,
        MOD = 4
    };
    unsigned type;
    Userdata data;

    friend QDataStream &operator>>(QDataStream &op, Operation &s)
    {
        op >> s.type >> s.data;
        return op;
    }

    friend QDataStream &operator<<(QDataStream &op, const Operation &s)
    {
        op << s.type << s.data;
        return op;
    }
};


constexpr std::chrono::milliseconds timeout(3000), refresh(2000);

#define errmsg(x) do {  QMessageBox::critical(this, tr("错误"), tr(x)); return; } while (false)
