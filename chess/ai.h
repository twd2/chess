#ifndef AI_H
#define AI_H

#include "engine.h"

#include <QObject>

class AI : public QObject
{
    Q_OBJECT
public:
    explicit AI(QObject *parent = 0);

signals:
    void suggest(int row, int col, int rev);
public slots:
    void boardChanged(board_t, chess_t, int rev);
};

#endif // AI_H
