#include "engine.h"

#include <QDebug>
#include <QObject>

std::default_random_engine Engine::gen;
std::uniform_int_distribution<int> Engine::dist(0, 1);

Engine::Engine()
{

}

board_t Engine::generate(int row, int col)
{
    return generate(row, col, [] (int, int) { return true; });
}

board_t Engine::generate(int row, int col, std::function<bool (int, int)> shapeFunc)
{
    board_t array;
    for (int i = 0; i < row; ++i)
    {
        board_row_t rowVec;
        for (int j = 0; j < col; ++j)
        {
            rowVec.append(shapeFunc(i, j) ? CH_SPACE : CH_FORBID);
        }
        array.append(rowVec);
    }
    return array;
}

chess_t Engine::findWin(const board_t &vec)
{
    int fullRowCount = 0;
    for (int row = 0; row < vec.count(); ++row)
    {
        auto &rowVec = vec[row];
        int isFull = 1;
        for (int col = 0; col < rowVec.count(); ++col)
        {
            chess_t ch = rowVec[col];
            if (ch == CH_SPACE)
            {
                isFull = 0;
                continue;
            }
            if (!isColor(ch))
            {
                continue;
            }
            // right
            if (col + 4 < rowVec.count())
            {
                if (rowVec[col + 1] == ch && rowVec[col + 2] == ch && rowVec[col + 3] == ch && rowVec[col + 4] == ch)
                {
                    return ch;
                }
            }
            // down
            if (row + 4 < vec.count())
            {
                if (vec[row + 1][col] == ch && vec[row + 2][col] == ch && vec[row + 3][col] == ch && vec[row + 4][col] == ch)
                {
                    return ch;
                }
            }
            // right down
            if (row + 4 < vec.count() && col + 4 < rowVec.count())
            {
                if (vec[row + 1][col + 1] == ch && vec[row + 2][col + 2] == ch && vec[row + 3][col + 3] == ch && vec[row + 4][col + 4] == ch)
                {
                    return ch;
                }
            }
            // left down
            if (row + 4 < vec.count() && col >= 4)
            {
                if (vec[row + 1][col - 1] == ch && vec[row + 2][col - 2] == ch && vec[row + 3][col - 3] == ch && vec[row + 4][col - 4] == ch)
                {
                    return ch;
                }
            }
        }
        fullRowCount += isFull;
    }
    if (fullRowCount == vec.count())
    {
        return CH_DRAW;
    }
    return CH_SPACE;
}

bool Engine::isDangerous(board_t vec, int row, int col, chess_t enemy)
{
    if (!isColor(enemy))
    {
        return false;
    }
    vec[row][col] = enemy;
    return findWin(vec) == enemy || findDangerous1(vec, enemy) || findDangerous2(vec, enemy) || findDangerous3(vec, enemy);
}

int Engine::countDangerous(const board_t &vec, chess_t enemy)
{
    int count = 0;
    for (int row = 0; row < vec.count(); ++row)
    {
        auto &rowVec = vec[row];
        for (int col = 0; col < rowVec.count(); ++col)
        {
            if (isBlock(vec, row, col))
            {
                continue;
            }
            count += isDangerous(vec, row, col, enemy);
        }
    }
    return count;
}

board_t Engine::fromJson(const QJsonArray &board)
{
    board_t vec;
    for (int row = 0; row < board.count(); ++row)
    {
        const QJsonArray &colArray = board[row].toArray();
        board_row_t rowVec;
        for (int col = 0; col < colArray.count(); ++col)
        {
            QString str = colArray[col].toString();
            if (str.count() > 0)
            {
                rowVec.append(str[0].toLatin1());
            }
            else
            {
                rowVec.append(CH_SPACE);
            }
        }
        vec.append(rowVec);
    }
    return vec;
}

bool Engine::findDangerous1(const board_t &vec, chess_t enemy)
{
    QVector<QVector<bool> > used;
    for (int row = 0; row < vec.count(); ++row)
    {
        auto &rowVec = vec[row];
        QVector<bool> v;
        for (int col = 0; col < rowVec.count(); ++col)
        {
            v.append(false);
        }
        used.append(v);
    }

    bool find;
    // two double-naked 3
    for (int i = 0; i < 2; ++i)
    {
        find = false;
        for (int row = 0; row < vec.count(); ++row)
        {
            auto &rowVec = vec[row];
            for (int col = 0; col < rowVec.count(); ++col)
            {
                chess_t ch = rowVec[col];
                if (ch != enemy)
                {
                    continue;
                }
                // right
                if (col + 2 < rowVec.count())
                {
                    if (rowVec[col + 1] == ch && rowVec[col + 2] == ch
                        && !isBlock(vec, row, col - 1) && !isBlock(vec, row, col + 3)
                        && (!used[row][col] || !used[row][col + 1] || !used[row][col + 2]))
                    {
                        used[row][col] = used[row][col + 1] = used[row][col + 2] = true;
                        find = true;
                        break;
                    }
                }
                // down
                if (row + 2 < vec.count())
                {
                    if (vec[row + 1][col] == ch && vec[row + 2][col] == ch
                        && !isBlock(vec, row - 1, col) && !isBlock(vec, row + 3, col)
                        && (!used[row][col] || !used[row + 1][col] || !used[row + 2][col]))
                    {
                        used[row][col] = used[row + 1][col] = used[row + 2][col] = true;
                        find = true;
                        break;
                    }
                }
                // right down
                if (row + 2 < vec.count() && col + 2 < rowVec.count())
                {
                    if (vec[row + 1][col + 1] == ch && vec[row + 2][col + 2] == ch
                        && !isBlock(vec, row - 1, col - 1) && !isBlock(vec, row + 3, col + 3)
                        && (!used[row][col] || !used[row + 1][col + 1] || !used[row + 2][col + 2]))
                    {
                        used[row][col] = used[row + 1][col + 1] = used[row + 2][col + 2] = true;
                        find = true;
                        break;
                    }
                }
                // left down
                if (row + 2 < vec.count() && col >= 2)
                {
                    if (vec[row + 1][col - 1] == ch && vec[row + 2][col - 2] == ch
                        && !isBlock(vec, row - 1, col + 1) && !isBlock(vec, row + 3, col - 3)
                        && (!used[row][col] || !used[row + 1][col - 1] || !used[row + 2][col - 2]))
                    {
                        used[row][col] = used[row + 1][col - 1] = used[row + 2][col - 2] = true;
                        find = true;
                        break;
                    }
                }
            }
            if (find)
            {
                break;
            }
        }
        if (!find)
        {
            return false;
        }
    }
    return find;
}

bool Engine::findDangerous2(const board_t &vec, chess_t enemy)
{
    bool find;
    // one double-naked 3
    find = false;
    for (int row = 0; row < vec.count(); ++row)
    {
        auto &rowVec = vec[row];
        for (int col = 0; col < rowVec.count(); ++col)
        {
            chess_t ch = rowVec[col];
            if (ch != enemy)
            {
                continue;
            }
            // right
            if (col + 2 < rowVec.count())
            {
                if (rowVec[col + 1] == ch && rowVec[col + 2] == ch
                    && !isBlock(vec, row, col - 1) && !isBlock(vec, row, col + 3))
                {
                    find = true;
                    break;
                }
            }
            // down
            if (row + 2 < vec.count())
            {
                if (vec[row + 1][col] == ch && vec[row + 2][col] == ch
                    && !isBlock(vec, row - 1, col) && !isBlock(vec, row + 3, col))
                {
                    find = true;
                    break;
                }
            }
            // right down
            if (row + 2 < vec.count() && col + 2 < rowVec.count())
            {
                if (vec[row + 1][col + 1] == ch && vec[row + 2][col + 2] == ch
                    && !isBlock(vec, row - 1, col - 1) && !isBlock(vec, row + 3, col + 3))
                {
                    find = true;
                    break;
                }
            }
            // left down
            if (row + 2 < vec.count() && col >= 2)
            {
                if (vec[row + 1][col - 1] == ch && vec[row + 2][col - 2] == ch
                    && !isBlock(vec, row - 1, col + 1) && !isBlock(vec, row + 3, col - 3))
                {
                    find = true;
                    break;
                }
            }
        }
        if (find)
        {
            break;
        }
    }
    if (!find)
    {
        return false;
    }
    find = false;
    // ...and...one single-naked 4
    for (int row = 0; row < vec.count(); ++row)
    {
        auto &rowVec = vec[row];
        for (int col = 0; col < rowVec.count(); ++col)
        {
            chess_t ch = rowVec[col];
            if (ch != enemy)
            {
                continue;
            }
            // right
            if (col + 3 < rowVec.count())
            {
                if (rowVec[col + 1] == ch && rowVec[col + 2] == ch && rowVec[col + 3] == ch
                    && (!isBlock(vec, row, col - 1) || !isBlock(vec, row, col + 4)))
                {
                    find = true;
                    break;
                }
            }
            // down
            if (row + 3 < vec.count())
            {
                if (vec[row + 1][col] == ch && vec[row + 2][col] == ch && vec[row + 3][col] == ch
                    && (!isBlock(vec, row - 1, col) || !isBlock(vec, row + 4, col)))
                {
                    find = true;
                    break;
                }
            }
            // right down
            if (row + 3 < vec.count() && col + 3 < rowVec.count())
            {
                if (vec[row + 1][col + 1] == ch && vec[row + 2][col + 2] == ch && vec[row + 3][col + 3] == ch
                    && (!isBlock(vec, row - 1, col - 1) || !isBlock(vec, row + 4, col + 4)))
                {
                    find = true;
                    break;
                }
            }
            // left down
            if (row + 3 < vec.count() && col >= 3)
            {
                if (vec[row + 1][col - 1] == ch && vec[row + 2][col - 2] == ch && vec[row + 3][col - 3] == ch
                    && (!isBlock(vec, row - 1, col + 1) || !isBlock(vec, row + 4, col - 4)))
                {
                    find = true;
                    break;
                }
            }
        }
        if (find)
        {
            break;
        }
    }
    return find;
}

bool Engine::findDangerous3(const board_t &vec, chess_t enemy)
{
    bool find = false;
    // one double-naked 4
    for (int row = 0; row < vec.count(); ++row)
    {
        auto &rowVec = vec[row];
        for (int col = 0; col < rowVec.count(); ++col)
        {
            chess_t ch = rowVec[col];
            if (ch != enemy)
            {
                continue;
            }
            // right
            if (col + 3 < rowVec.count())
            {
                if (rowVec[col + 1] == ch && rowVec[col + 2] == ch && rowVec[col + 3] == ch
                    && (!isBlock(vec, row, col - 1) && !isBlock(vec, row, col + 4)))
                {
                    find = true;
                    break;
                }
            }
            // down
            if (row + 3 < vec.count())
            {
                if (vec[row + 1][col] == ch && vec[row + 2][col] == ch && vec[row + 3][col] == ch
                    && (!isBlock(vec, row - 1, col) && !isBlock(vec, row + 4, col)))
                {
                    find = true;
                    break;
                }
            }
            // right down
            if (row + 3 < vec.count() && col + 3 < rowVec.count())
            {
                if (vec[row + 1][col + 1] == ch && vec[row + 2][col + 2] == ch && vec[row + 3][col + 3] == ch
                    && (!isBlock(vec, row - 1, col - 1) && !isBlock(vec, row + 4, col + 4)))
                {
                    find = true;
                    break;
                }
            }
            // left down
            if (row + 3 < vec.count() && col >= 3)
            {
                if (vec[row + 1][col - 1] == ch && vec[row + 2][col - 2] == ch && vec[row + 3][col - 3] == ch
                    && (!isBlock(vec, row - 1, col + 1) && !isBlock(vec, row + 4, col - 4)))
                {
                    find = true;
                    break;
                }
            }
        }
        if (find)
        {
            break;
        }
    }
    return find;
}

bool Engine::isBlock(const board_t &vec, int row, int col)
{
    if (row < 0 || row >= vec.count() || col < 0 || col >= vec[0].count())
    {
        return true;
    }
    return vec[row][col] != CH_SPACE;
}

QPoint Engine::findMostDangerous(const board_t &vec, chess_t self)
{
    chess_t enemy = nextColor(self);
    // TODO
    int maxScore = 0;
    QPoint p(0, 0);
    while (enemy != self)
    {
        for (int row = 0; row < vec.count(); ++row)
        {
            auto &rowVec = vec[row];
            for (int col = 0; col < rowVec.count(); ++col)
            {
                if (isBlock(vec, row, col))
                {
                    continue;
                }
                auto newVec = vec;
                newVec[row][col] = enemy;
                int score = (findWin(newVec) == enemy) * 5
                            + (findWin(newVec) == self) * 10
                            + findDangerous1(newVec, enemy)
                            + findDangerous2(newVec, enemy)
                            + findDangerous3(newVec, enemy)
                            + countDangerous(newVec, enemy);
                if (score > maxScore)
                {
                    maxScore = score;
                    p = QPoint(col, row);
                }
            }
        }
        enemy = nextColor(enemy);
    }
    if (maxScore == 0)
    {
        for (int row = 0; row < vec.count(); ++row)
        {
            const auto &rowVec = vec[row];
            for (int col = 0; col < rowVec.count(); ++col)
            {
                if (!isBlock(vec, row, col))
                {
                    p = QPoint(col, row);
                }
            }
        }
    }
    return p;
}

QString Engine::name(chess_t ch)
{
    if (ch == CH_BLACK)
    {
        return QObject::tr("Black");
    }
    else if (ch == CH_WHITE)
    {
        return QObject::tr("White");
    }
    else if (ch == CH_VIEWER)
    {
        return QObject::tr("Viewer");
    }
    else
    {
        return QObject::tr("Control");
    }
}

QJsonArray Engine::toJson(const board_t &vec)
{
    QJsonArray array;
    for (int row = 0; row < vec.count(); ++row)
    {
        auto &rowVec = vec[row];
        QJsonArray rowArray;
        for (int col = 0; col < rowVec.count(); ++col)
        {
            rowArray.append(toJson(rowVec[col]));
        }
        array.append(rowArray);
    }
    return array;
}
