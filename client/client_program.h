#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "../shared.h"
#include "ui_client_program.h"
namespace Ui {
class ClientProgram;
}




class ClientProgram : public QWidget
{
    Q_OBJECT

public:
    explicit ClientProgram(QWidget *parent = 0);
    ~ClientProgram();

private:
    Ui::ClientProgram *ui;
    QTimer *timeout_guard, *interval_fetch, *interval_sendip;
    QHostAddress curServer;
    //QHostAddress *curContact;
    QUdpSocket *with_client;
    QTcpSocket *with_server;
    Userdata* me;
    QHash<QUuid, Userdata> usertb;
    bool already_logged_in;
    //UI
    unsigned version;

    void ui_setall(bool enable);
    void ui_add(const Userdata &);
    void ui_del(const QUuid &);
    void ui_display_remote_msg(const QUuid& peerid, const QString& msg);
    void ui_display_local_msg(const QString& msg);


private slots:
    void on_pbLogin_clicked();
    void on_leSendBuffer_returnPressed();
    void dispatch(QByteArray inputdata, QTcpSocket& so);
    void withserver_failed();
    void login2();
    void fetch2()
    void dispatch_udp();
    void fetch(); //在这里面settimeout，不要setinterval


};

#endif // MAINWINDOW_H
