#include "discoverylist.h"
#include "ui_discoverylist.h"

DiscoveryList::DiscoveryList(ServerDiscovery *discovery, QWidget *parent)
    : QDialog(parent), discovery(discovery),
    ui(new Ui::DiscoveryList)
{
    ui->setupUi(this);
    connect(ui->btnRefresh, SIGNAL(clicked(bool)), discovery, SLOT(findServer()));
    connect(discovery, SIGNAL(serverFound(QString, quint16)), this, SLOT(serverFound(QString, quint16)));
}

DiscoveryList::~DiscoveryList()
{
    delete ui;
}

void DiscoveryList::serverFound(QString addressStr, quint16 port)
{
    QHostAddress address(addressStr);
    for (const QPair<QHostAddress, quint16> &p : servers)
    {
        if (p.first == address && p.second == port)
        {
            // already found
            return;
        }
    }
    servers.append(qMakePair(address, port));
    refreshList();
}

void DiscoveryList::on_btnRefresh_clicked()
{
    servers.clear();
    refreshList();
}

void DiscoveryList::refreshList()
{
    ui->lstServer->clear();
    for (int i = 0; i < servers.count(); ++i)
    {
        ui->lstServer->addItem(QString("%1:%2").arg(servers[i].first.toString()).arg(servers[i].second));
    }
}

void DiscoveryList::on_lstServer_doubleClicked(const QModelIndex &index)
{
    qDebug() << (selectedEndpoint = servers[index.row()]);
    accept();
}

void DiscoveryList::on_buttonBox_accepted()
{
    if (ui->lstServer->selectedItems().count() == 0)
    {
        reject();
        return;
    }
    qDebug() << (selectedEndpoint = servers[ui->lstServer->currentRow()]);
    accept();
}
