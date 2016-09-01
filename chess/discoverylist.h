#ifndef DISCOVERYLIST_H
#define DISCOVERYLIST_H

#include "serverdiscovery.h"

#include <QDialog>
#include <QHostAddress>
#include <QPair>
#include <QMap>

namespace Ui {
class DiscoveryList;
}

class DiscoveryList : public QDialog
{
    Q_OBJECT

public:
    QPair<QHostAddress, quint16> selectedEndpoint;
    ServerDiscovery *discovery = nullptr;
    explicit DiscoveryList(ServerDiscovery *discovery, QWidget *parent = 0);
    ~DiscoveryList();
public slots:
    void serverFound(QString, quint16);
private slots:
    void on_btnRefresh_clicked();
    void refreshList();

    void on_lstServer_doubleClicked(const QModelIndex &index);

    void on_buttonBox_accepted();

private:
    QVector<QPair<QHostAddress, quint16> > servers;
    Ui::DiscoveryList *ui;

};

#endif // DISCOVERYLIST_H
