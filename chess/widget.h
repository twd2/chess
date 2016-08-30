#ifndef WIDGET_H
#define WIDGET_H

#include <memory>

#include <QWidget>
#include <QTcpServer>
#include <QTcpSocket>

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();

private slots:
    void on_btnTestBoard_clicked();

    void on_btnTestLock_clicked();

private:
    Ui::Widget *ui;
    std::shared_ptr<QTcpServer> server = nullptr;
    std::shared_ptr<QTcpSocket> client = nullptr;
};

#endif // WIDGET_H
