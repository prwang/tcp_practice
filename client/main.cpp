#include "client_program.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ClientProgram w;
    w.show();
    return a.exec();
}
