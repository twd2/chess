#include "engine.h"

Engine::Engine()
{

}

QString Engine::findWin(const QJsonArray &board)
{
    auto vec = toVector(board);
    // TODO
    return "";
}

bool Engine::isDangerous(const QJsonArray &board, int row, int col, const QString &enemy)
{
    if (enemy == "")
    {
        return false;
    }
    auto vec = toVector(board);
    vec[row][col] = enemy;
    return findDangerous1(vec, enemy) || findDangerous2(vec, enemy);
}

QVector<QVector<QString> > Engine::toVector(const QJsonArray &board)
{
    QVector<QVector<QString> > vec;
    for (int row = 0; row < board.count(); ++row)
    {
        const QJsonArray &colArray = board[row].toArray();
        QVector<QString> rowVec;
        for (int col = 0; col < colArray.count(); ++col)
        {
            rowVec.append(colArray[col].toString());
        }
        vec.append(rowVec);
    }
    return vec;
}

bool Engine::findDangerous1(const QVector<QVector<QString> > &board, const QString &enemy)
{
    // TODO
    return true;
}

bool Engine::findDangerous2(const QVector<QVector<QString> > &board, const QString &enemy)
{
    // TODO
    return true;
}
