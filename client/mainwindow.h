#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "../shared.h"
#include "ui_mainwindow.h"
namespace Ui {
class MainWindow;
}

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QHostAddress* curContact;
    QUdpSocket *sender;
private slots:
    void on_pbLogin_clicked();
    void on_leSendBuffer_returnPressed();

};

#endif // MAINWINDOW_H
