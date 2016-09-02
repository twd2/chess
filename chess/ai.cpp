#include "ai.h"

AI::AI(QObject *parent) : QObject(parent)
{

}

void AI::boardChanged(board_t board, chess_t self, int rev)
{
    QPoint p = Engine::findMostDangerous(board, self);
    emit suggest(p.y(), p.x(), rev);
}
