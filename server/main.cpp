#include "server_program.h"

using namespace std;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ServerProgram w;
    w.show();

    struct S
    {
        int a, b;
    };

    return a.exec();

};


