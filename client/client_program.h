#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "../shared.h"
#include "ui_client_program.h"
namespace Ui { class ClientProgram; }
#include "friend.h"



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
    QHash<QUuid, Friend_* > usertb;
    QHash<QListWidgetItem*, Friend_*> lst_rev;
    Friend_* curContact;
    //UI
    unsigned version;

    void hole_request();
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
    void send_ip();
    void dispatch(QByteArray inputdata, QTcpSocket& so);
    void on_lwContacts_itemClicked(QListWidgetItem* );
    void withserver_failed();
    void login2();
    void fetch2();
    void dispatch_udp();
    void fetch(); //在这里面settimeout，不要setinterval


};

#endif // MAINWINDOW_H
