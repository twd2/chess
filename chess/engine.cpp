#include "engine.h"

#include <QDebug>

Engine::Engine()
{

}

char Engine::findWin(const QVector<QVector<char> > &vec)
{
    for (int row = 0; row < vec.count(); ++row)
    {
        auto &rowVec = vec[row];
        for (int col = 0; col < rowVec.count(); ++col)
        {
            char ch = rowVec[col];
            if (ch == ' ')
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
    }
    return ' ';
}

bool Engine::isDangerous(QVector<QVector<char> > vec, int row, int col, char enemy)
{
    if (enemy == ' ')
    {
        return false;
    }
    vec[row][col] = enemy;
    return findWin(vec) == enemy || findDangerous1(vec, enemy) || findDangerous2(vec, enemy) || findDangerous3(vec, enemy);
}

QVector<QVector<char> > Engine::toVector(const QJsonArray &board)
{
    QVector<QVector<char> > vec;
    for (int row = 0; row < board.count(); ++row)
    {
        const QJsonArray &colArray = board[row].toArray();
        QVector<char> rowVec;
        for (int col = 0; col < colArray.count(); ++col)
        {
            QString str = colArray[col].toString();
            if (str.count() > 0)
            {
                rowVec.append(str[0].toLatin1());
            }
            else
            {
                rowVec.append(' ');
            }
        }
        vec.append(rowVec);
    }
    return vec;
}

bool Engine::findDangerous1(const QVector<QVector<char> > &vec, char enemy)
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
                char ch = rowVec[col];
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

bool Engine::findDangerous2(const QVector<QVector<char> > &vec, char enemy)
{
    bool find;
    // one double-naked 3
    find = false;
    for (int row = 0; row < vec.count(); ++row)
    {
        auto &rowVec = vec[row];
        for (int col = 0; col < rowVec.count(); ++col)
        {
            char ch = rowVec[col];
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
    // one single-naked 4
    for (int row = 0; row < vec.count(); ++row)
    {
        auto &rowVec = vec[row];
        for (int col = 0; col < rowVec.count(); ++col)
        {
            char ch = rowVec[col];
            if (ch != enemy)
            {
                continue;
            }
            // right
            if (col + 2 < rowVec.count())
            {
                if (rowVec[col + 1] == ch && rowVec[col + 2] == ch && rowVec[col + 3] == ch
                    && (!isBlock(vec, row, col - 1) || !isBlock(vec, row, col + 4)))
                {
                    find = true;
                    break;
                }
            }
            // down
            if (row + 2 < vec.count())
            {
                if (vec[row + 1][col] == ch && vec[row + 2][col] == ch && vec[row + 3][col] == ch
                    && (!isBlock(vec, row - 1, col) || !isBlock(vec, row + 4, col)))
                {
                    find = true;
                    break;
                }
            }
            // right down
            if (row + 2 < vec.count() && col + 2 < rowVec.count())
            {
                if (vec[row + 1][col + 1] == ch && vec[row + 2][col + 2] == ch && vec[row + 3][col + 3] == ch
                    && (!isBlock(vec, row - 1, col - 1) || !isBlock(vec, row + 4, col + 4)))
                {
                    find = true;
                    break;
                }
            }
            // left down
            if (row + 2 < vec.count() && col >= 2)
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

bool Engine::findDangerous3(const QVector<QVector<char> > &vec, char enemy)
{
    bool find = false;
    // one double-naked 4
    for (int row = 0; row < vec.count(); ++row)
    {
        auto &rowVec = vec[row];
        for (int col = 0; col < rowVec.count(); ++col)
        {
            char ch = rowVec[col];
            if (ch != enemy)
            {
                continue;
            }
            // right
            if (col + 2 < rowVec.count())
            {
                if (rowVec[col + 1] == ch && rowVec[col + 2] == ch && rowVec[col + 3] == ch
                    && (!isBlock(vec, row, col - 1) && !isBlock(vec, row, col + 4)))
                {
                    find = true;
                    break;
                }
            }
            // down
            if (row + 2 < vec.count())
            {
                if (vec[row + 1][col] == ch && vec[row + 2][col] == ch && vec[row + 3][col] == ch
                    && (!isBlock(vec, row - 1, col) && !isBlock(vec, row + 4, col)))
                {
                    find = true;
                    break;
                }
            }
            // right down
            if (row + 2 < vec.count() && col + 2 < rowVec.count())
            {
                if (vec[row + 1][col + 1] == ch && vec[row + 2][col + 2] == ch && vec[row + 3][col + 3] == ch
                    && (!isBlock(vec, row - 1, col - 1) && !isBlock(vec, row + 4, col + 4)))
                {
                    find = true;
                    break;
                }
            }
            // left down
            if (row + 2 < vec.count() && col >= 2)
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

bool Engine::isBlock(const QVector<QVector<char> > &vec, int row, int col)
{
    if (row < 0 || row >= vec.count() || col < 0 || col >= vec[0].count())
    {
        return true;
    }
    return vec[row][col] != ' ';
}
