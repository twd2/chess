#include "widget.h"
#include "ui_widget.h"

#include <QJsonObject>

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
}

Widget::~Widget()
{
    delete ui;
}

void Widget::on_pushButton_5_clicked()
{
    QVector<QVector<char> > array;
    for (int i = 0; i < 15; ++i)
    {
        QVector<char> row;
        for (int j = 0; j < 15; ++j)
        {
            row.append(' ');
        }
        array.append(row);
    }
    ui->widget->setBoard(array);
}

void Widget::on_pushButton_6_clicked()
{
    ui->widget->setLock(!ui->widget->getLock());
}
