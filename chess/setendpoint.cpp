#include "setendpoint.h"
#include "ui_setendpoint.h"

#include <QSignalMapper>

SetEndpoint::SetEndpoint(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SetEndpoint)
{
    ui->setupUi(this);
    QSignalMapper *mapper = new QSignalMapper(this);
    mapper->setMapping(ui->num0, 0);
    connect(ui->num0, SIGNAL(clicked(bool)), mapper, SLOT(map()));
    mapper->setMapping(ui->num1, 1);
    connect(ui->num1, SIGNAL(clicked(bool)), mapper, SLOT(map()));
    mapper->setMapping(ui->num2, 2);
    connect(ui->num2, SIGNAL(clicked(bool)), mapper, SLOT(map()));
    mapper->setMapping(ui->num3, 3);
    connect(ui->num3, SIGNAL(clicked(bool)), mapper, SLOT(map()));
    mapper->setMapping(ui->num4, 4);
    connect(ui->num4, SIGNAL(clicked(bool)), mapper, SLOT(map()));
    mapper->setMapping(ui->num5, 5);
    connect(ui->num5, SIGNAL(clicked(bool)), mapper, SLOT(map()));
    mapper->setMapping(ui->num6, 6);
    connect(ui->num6, SIGNAL(clicked(bool)), mapper, SLOT(map()));
    mapper->setMapping(ui->num7, 7);
    connect(ui->num7, SIGNAL(clicked(bool)), mapper, SLOT(map()));
    mapper->setMapping(ui->num8, 8);
    connect(ui->num8, SIGNAL(clicked(bool)), mapper, SLOT(map()));
    mapper->setMapping(ui->num9, 9);
    connect(ui->num9, SIGNAL(clicked(bool)), mapper, SLOT(map()));
    mapper->setMapping(ui->btnDot, -1);
    connect(ui->btnDot, SIGNAL(clicked(bool)), mapper, SLOT(map()));
    connect(mapper, SIGNAL(mapped(int)), this, SLOT(numClicked(int)));
}

SetEndpoint::~SetEndpoint()
{
    delete ui;
}

void SetEndpoint::on_buttonBox_rejected()
{
    reject();
}

void SetEndpoint::on_buttonBox_accepted()
{
    address = QHostAddress(ui->lineEdit->text());
    port = ui->lineEdit_2->text().toInt();
    accept();
}

void SetEndpoint::numClicked(int n)
{
    if (n >= 0)
    {
        ui->lineEdit->setText(ui->lineEdit->text() + QString::number(n));
    }
    else if (n == -1)
    {
        ui->lineEdit->setText(ui->lineEdit->text() + ".");
    }
}

void SetEndpoint::setAddress(QHostAddress addr)
{
    ui->lineEdit->setText(addr.toString());
}

void SetEndpoint::setPort(quint16 port)
{
    ui->lineEdit_2->setText(QString::number(port));
}

void SetEndpoint::on_btnDel_clicked()
{
    if (ui->lineEdit->text().length() > 0)
    {
        ui->lineEdit->setText(ui->lineEdit->text().mid(0, ui->lineEdit->text().length() - 1));
    }
}
