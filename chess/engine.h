#ifndef ENGINE_H
#define ENGINE_H

#include <QString>
#include <QJsonArray>
#include <QPoint>
#include <QVector>

class Engine
{
public:
    Engine();
    static char findWin(const QVector<QVector<char> > &vec);
    static bool isDangerous(QVector<QVector<char> > vec, int row, int col, char enemy);
    static QVector<QVector<char> > toVector(const QJsonArray &board);
private:
    static bool findDangerous1(const QVector<QVector<char> > &vec, char enemy);
    static bool findDangerous2(const QVector<QVector<char> > &vec, char enemy);
    static bool findDangerous3(const QVector<QVector<char> > &vec, char enemy);
    static bool isBlock(const QVector<QVector<char> > &vec, int row, int col);
};

#endif // ENGINE_H
