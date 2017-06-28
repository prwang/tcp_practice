#include "mainwindow.h"

using namespace std;
mt19937_64 RAND((random_device())());

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
