#ifndef BOARD_H
#define BOARD_H

#include "utils.h"

#include <QWidget>
#include <QJsonArray>
#include <QPaintEvent>
#include <QMouseEvent>

class Board : public QWidget
{
    Q_OBJECT
protected:
    QJsonArray chess;
    bool lock = false;
public:
    explicit Board(QWidget *parent = 0);
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *) override;
    QJsonArray getChess() const;
    bool getLock() const;
signals:
    void clicked(int row, int col);
public slots:
    void setBoard(const QJsonArray &);
    void setLock(bool);
};

#endif // BOARD_H
