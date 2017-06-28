#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "../shared.h"
#include "ui_mainwindow.h"


namespace Ui
{
    class MainWindow;
}


class server_msg : public read_first
{
Q_OBJECT
public:
    explicit server_msg(QTcpSocket *_conn, QObject *parent = nullptr)
            : read_first(_conn, parent)
    {
        connect(this, SIGNAL(sg_login(Userdata)), parent, SLOT(login(Userdata)));
        connect(this, SIGNAL(sg_logout(Userdata)), parent, SLOT(logout(Userdata)));
        connect(this, SIGNAL(sg_getlist(Userdata)), parent, SLOT(getlist(Userdata)));
    }
private:
    void after_read_done() override
    {
        QDataStream ds(br);

//或者用一个uint64表示session
        switch (header.type)
        {
        case opcd::RQ_LOGIN:
        {

            Userdata u;
            ds >> u;
            emit sg_login(u, *conn);
            break;
        }
        case opcd::RQ_LOGOUT:
        {
            QUuid id; ds >> id;

            emit sg_logout(id, *conn);
            break;
        }
        case opcd::RQ_GETLIST:
        {
            QUuid id; ds >> id;
            emit sg_getlist(id, *conn);
            break;
        }
        default:;
        }
        deleteLater();
    }
signals:
    void sg_login(Userdata, QTcpSocket&);
    void sg_logout(QUuid, QTcpSocket&);
    void sg_getlist(QUuid, QTcpSocket&);
};


class MainWindow : public QWidget
{
Q_OBJECT


public:
    explicit MainWindow(QWidget *parent = 0);

    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QTcpServer *server;
    QHash<QUuid, Userdata> usertb;
    QHash<QUuid, QListWidgetItem*> usertb_ui;
    QVector<Operation> changes;
public slots:
    void login(Userdata, QTcpSocket&);
    void logout(QUuid, QTcpSocket&);
    void getlist(QUuid, QTcpSocket&);

private slots:
    void acceptConnection();

};

#endif // MAINWINDOW_H
