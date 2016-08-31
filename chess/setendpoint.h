#ifndef SETENDPOINT_H
#define SETENDPOINT_H

#include <QDialog>
#include <QHostAddress>

namespace Ui {
class SetEndpoint;
}

class SetEndpoint : public QDialog
{
    Q_OBJECT

public:
    explicit SetEndpoint(QWidget *parent = 0);
    ~SetEndpoint();

    QHostAddress address;
    quint16 port;

    void setAddress(QHostAddress);
    void setPort(quint16);
private slots:
    void on_buttonBox_rejected();
    void on_buttonBox_accepted();
    void numClicked(int);

    void on_btnDel_clicked();

private:
    Ui::SetEndpoint *ui;
};

#endif // SETENDPOINT_H
