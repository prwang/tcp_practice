#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "../shared.h"
#include "ui_server_program.h"


namespace Ui
{
    class ServerProgram;
}


class server_msg : public read_first
{
Q_OBJECT
public:
    explicit server_msg(QTcpSocket *_conn, QObject *parent = nullptr)
            : read_first(_conn, parent)
    {
        connect(this, SIGNAL(success(QTcpSocket*)), parent, SLOT(dispatch(QTcpSocket*)));
        connect(this, SIGNAL(fail()), parent, SLOT(fail()));
    }

private:
    void handleerror() override { deleteLater(); }

    void dispatch() override
    {
        emit success(std::move(br), *conn);
        deleteLater();
    }

signals:
    void success(QByteArray, QTcpSocket&);
    void fail();
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
    QHash<QUuid, Userdata> usertb;
    QHash<QUuid, QListWidgetItem *> usertb_ui;
    QHash<QUuid, QList<QUuid> > puncreq_tb;
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
