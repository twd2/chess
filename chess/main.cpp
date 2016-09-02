#include "engine.h"
#include "widget.h"

#include <QApplication>
#include <QLocale>
#include <QtGlobal>
#include <QTranslator>
#include <QFont>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    qRegisterMetaType<chess_t>("chess_t");
    qRegisterMetaType<chess_t>("chess_t&");
    qRegisterMetaType<board_t>("board_t");
    qRegisterMetaType<board_t>("board_t&");

    QLocale locale;
    qDebug() << locale.name().toLower();
    QTranslator trans, qtTrans;
    if (locale.name().toLower() == "zh_cn")
    {
        qDebug() << "loading zh_CN";
        trans.load(":/locale/zh_CN.qm");
        a.installTranslator(&trans);
    }

#ifdef Q_OS_WIN
    QFont font;
    font.setFamily("Microsoft YaHei");
    a.setFont(font);
#endif

    Widget w;
    w.show();

    return a.exec();
}
