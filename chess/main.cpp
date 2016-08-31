#include "widget.h"

#include <QApplication>
#include <QtGlobal>
#include <QFont>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

#ifdef Q_OS_WIN
    QFont font;
    font.setFamily("Microsoft YaHei");
    a.setFont(font);
#endif

    Widget w;
    w.show();

    return a.exec();
}
