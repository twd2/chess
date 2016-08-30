#include "setendpoint.h"
#include "ui_setendpoint.h"

SetEndpoint::SetEndpoint(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SetEndpoint)
{
    ui->setupUi(this);
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
