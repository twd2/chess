#ifndef ENGINE_H
#define ENGINE_H

#include <random>
#include <functional>

#include <QString>
#include <QJsonArray>
#include <QPoint>
#include <QVector>

typedef char chess_t;
typedef QVector<chess_t> board_row_t;
typedef QVector<board_row_t> board_t;

constexpr chess_t CH_BLACK = 'B';
constexpr chess_t CH_WHITE = 'W';
constexpr chess_t CH_FORBID = 'X';
constexpr chess_t CH_SPACE = ' ';
constexpr chess_t CH_DRAW = '-';
constexpr chess_t CH_VIEWER = 'V';

class Engine
{
public:
    Engine();
    static std::default_random_engine gen;
    static std::uniform_int_distribution<int> dist;
    static chess_t findWin(const board_t &vec);
    static bool isDangerous(board_t vec, int row, int col, chess_t enemy);
    static int countDangerous(const board_t &vec, chess_t enemy);
    static board_t fromJson(const QJsonArray &board);

    static inline chess_t fromJson(const QJsonValue &ch)
    {
        return ch.toString()[0].toLatin1();
    }

    static QJsonArray toJson(const board_t &vec);

    static inline QString toJson(chess_t ch)
    {
        return QString(ch);
    }

    static constexpr inline bool isColor(chess_t ch)
    {
        return ch == CH_BLACK || ch == CH_WHITE;
    }

    static board_t generate(int row = 15, int col = 15);
    static board_t generate(int row, int col, std::function<bool (int, int)> shapeFunc);

    static constexpr inline chess_t nextColor(chess_t c)
    {
        return c == CH_WHITE ? CH_BLACK : CH_WHITE;
    }

    static constexpr inline chess_t previousColor(chess_t c)
    {
        return c == CH_WHITE ? CH_BLACK : CH_WHITE;
    }

    static inline chess_t randomColor()
    {
        int r = dist(gen);
        return r ? CH_BLACK : CH_WHITE;
    }

    static bool findDangerous1(const board_t &vec, chess_t enemy);
    static bool findDangerous2(const board_t &vec, chess_t enemy);
    static bool findDangerous3(const board_t &vec, chess_t enemy);
    static bool isBlock(const board_t &vec, int row, int col);
    static QPoint findMostDangerous(const board_t &vec, chess_t self);
    static QString name(chess_t);
};

#endif // ENGINE_H
