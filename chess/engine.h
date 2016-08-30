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
    static QString findWin(const QJsonArray &board);
    static bool isDangerous(const QJsonArray &board, int row, int col, const QString &enemy);
private:
    static QVector<QVector<QString> > toVector(const QJsonArray &board);
    static bool findDangerous1(const QVector<QVector<QString> > &board, const QString &enemy);
    static bool findDangerous2(const QVector<QVector<QString> > &board, const QString &enemy);
};

#endif // ENGINE_H
