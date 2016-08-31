#ifndef BOARD_H
#define BOARD_H

#include "utils.h"
#include "engine.h"

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
    board_t _board;
    bool _lock = false;
    QString lockText = "";
    int lastRow = -1, lastCol = -1;
    bool _hint = true;
public:
    chess_t myColor = CH_WHITE;
    explicit Board(QWidget *parent = 0);
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *) override;
    board_t board() const;
    bool lock() const;
    bool hint() const;
signals:
    void clicked(int row, int col);
public slots:
    void setBoard(const board_t &);
    void setLock(bool, const QString &lockText = "");
    void setLast(int row, int col);
    void setHint(bool);


};

#endif // BOARD_H
