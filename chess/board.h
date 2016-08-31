#ifndef BOARD_H
#define BOARD_H

#include "utils.h"

#include <memory>

#include <QWidget>
#include <QJsonArray>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QTcpServer>
#include <QTcpSocket>
#include <QJsonObject>

class Board : public QWidget
{
    Q_OBJECT
protected:
    QVector<QVector<char> > chess;
    bool lock = false;
    QString lockText = "";
    int lastRow = -1, lastCol = -1;
    bool _hint = true;
public:
    char color = 'W';
    explicit Board(QWidget *parent = 0);
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *) override;
    QVector<QVector<char> > getChess() const;
    bool getLock() const;
    bool hint() const;
signals:
    void clicked(int row, int col);
public slots:
    void setBoard(const QVector<QVector<char> > &);
    void setLock(bool, const QString &lockText = "");
    void setLast(int row, int col);
    void setHint(bool);


};

#endif // BOARD_H
