#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "../shared.h"
#include "ui_server_program.h"


namespace Ui
{
    class ServerProgram;
}


struct User : Userdata
{
    QListWidgetItem* disp;
    QList<QUuid> puncreq;
    explicit User(const Userdata& ud)
    : disp(nullptr), puncreq{}, Userdata(ud) {}

};

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

    QHash<QUuid, User> usertb;
    QVector<Operation> changes;
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
