#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "../shared.h"
#include "ui_server_program.h"


namespace Ui
{
    class ServerProgram;
}




class ServerProgram : public QWidget
{
Q_OBJECT


public:
    explicit ServerProgram(QWidget *parent = 0);

    ~ServerProgram();

private:
    Ui::ServerProgram *ui;
    QTcpServer *server;
    QUdpSocket *punching;
    QHash<QUuid, Userdata> usertb;
    QHash<QUuid, QListWidgetItem *> usertb_ui;
    QHash<QUuid, QList<QUuid> > puncreq_tb;
    //TODO 把三个东西组合起来 struct Userdata_server : userdata
    //QLWI别放基类里面，到时候要写没有UI的服务器版本
    QVector<Operation> changes; //回传：更改列表 + 等待的打洞请求　+　别人完成的打洞请求

public slots:
    void ui_add(const Userdata&);
    void ui_del(const QUuid&);

    void dispatch(QByteArray, QTcpSocket&);
    void cleanup();
private slots:


    void acceptConnection();

    void dispatch_udp();

};

#endif // MAINWINDOW_H
