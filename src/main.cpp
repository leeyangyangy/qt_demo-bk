#include "widget.h"

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);
    Widget w;
    w.resize(1024,768);
    w.show();
    return a.exec();
}