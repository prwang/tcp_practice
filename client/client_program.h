#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "../shared.h"
#include "ui_client_program.h"
namespace Ui {
class ClientProgram;
}


struct login_recv : read_first
{
Q_OBJECT
public:
    explicit login_recv(QTcpSocket* _conn, QObject *parent = nullptr)
        : read_first(_conn, parent)
    {
        connect(this, SIGNAL(success(QTcpSocket*)), parent, SLOT(login_permitted(QTcpSocket*)));
        connect(this, SIGNAL(fail()), parent, SLOT(fail()));
    }
private:
    void dispatch() override
    {
        if (header.type & opcd::RSP_OK)
            emit success(conn);
        else emit fail();
        deleteLater();
    }
    void handleerror() override
    {
        emit fail();
        deleteLater();
    }
signals:
    void success(QTcpSocket*);
    void fail();

};



class ClientProgram : public QWidget
{
    Q_OBJECT

public:
    explicit ClientProgram(QWidget *parent = 0);
    ~ClientProgram();

private:
    Ui::ClientProgram *ui;
    QTimer *tm;
    QHostAddress curServer;
    QHostAddress *curContact;
    QUdpSocket *with_client;
    QTcpSocket *with_server;
    Userdata* me;
    //UI


    void ui_add(const Userdata &);
    void ui_del(const QUuid &);
    void ui_display_remote_msg(const QUuid& peerid, const QString& msg);
    void ui_display_local_msg(const QString& msg);


private slots:
    void on_pbLogin_clicked();
    void on_leSendBuffer_returnPressed();
    void login_failed();
    void login_send();
    void login_permitted();
    void dispatch_udp();
    void fetch(); //在这里面settimeout，不要setinterval


};

#endif // MAINWINDOW_H
