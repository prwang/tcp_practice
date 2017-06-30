#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "../shared.h"
#include "ui_client_program.h"
namespace Ui { class ClientProgram; }




struct Friend_ : Userdata, QObject
{
    Q_OBJECT
    QString history;
    QString pendingmsg; //特别地，长度为0，表示是没有正在等待的消息的
    QTimer *guard;
    int try_time;
    friend class ClientProgram;
public:
    explicit Friend_(const Userdata& bs, ClientProgram* parent) :Userdata(bs),
        history{}, pendingmsg{}, guard(new QTimer(this)), QObject(parent), try_time(0) { }
    void sendmsg();
    void ok();
private slots:
    void send_timeout();

};
class ClientProgram : public QWidget
{
    Q_OBJECT

public:
    explicit ClientProgram(QWidget *parent = 0);
    ~ClientProgram();
    friend class Friend_;

private:
    Ui::ClientProgram *ui;
    QTimer *timeout_guard,  *interval_sendip;
    QHostAddress curServer;
    QUdpSocket *with_client;
    QTcpSocket *with_server;
    Userdata me;
    QHash<QUuid, Friend_> usertb;
    QHash<QUuid, Friend_>::iterator curContact;
    //UI
    unsigned version;

    void send_ip();
    void ui_setupFriend();
    void ui_setall(bool enable);
    void ui_add(Friend_ &fr);
    void ui_del(Friend_ &);
    void ui_send_error();
    void ui_display_remote_msg(const Friend_ &peerid, const QString &msg);
    void ui_display_local_msg(const QString& msg);


private slots:
    void on_pbLogin_clicked();
    void on_leSendBuffer_returnPressed();
    void dispatch(QByteArray inputdata, QTcpSocket& so);
    void withserver_failed();
    void login2();
    void fetch2();
    void dispatch_udp();
    void fetch(); //在这里面settimeout，不要setinterval


};

#endif // MAINWINDOW_H
