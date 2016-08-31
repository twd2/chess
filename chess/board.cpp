#include "board.h"
#include "engine.h"

#include <QDebug>
#include <QPainter>

Board::Board(QWidget *parent) : QWidget(parent)
{

}

void Board::paintEvent(QPaintEvent *e)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    int boardSize = min(width(), height());
    if (chess.count() > 0 && chess[0].count() > 0)
    {
        int rowCount = chess.count(),
            colCount = chess[0].count();
        Q_ASSERT(rowCount == colCount);
        int doubleRadius = boardSize / rowCount;
        boardSize = doubleRadius * rowCount;
        int radius = doubleRadius / 2;
        const int boarder = radius * 0.2,
                  bigBoarder = radius * 0.6;
        p.fillRect(0, 0, boardSize, boardSize, Qt::darkYellow);
        QPixmap boardImg(":/image/board.jpg");
        p.drawPixmap(0, 0, boardSize, boardSize, boardImg.scaled(boardSize, boardSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
        for (int row = 0; row < rowCount; ++row)
        {
            p.drawLine(radius, row * doubleRadius + radius, boardSize - radius, row * doubleRadius + radius); // -
            p.drawLine(row * doubleRadius + radius, radius, row * doubleRadius + radius, boardSize - radius); // |
        }

        for (int row = 0; row < rowCount; ++row)
        {
            const auto &colArray = chess[row];
            for (int col = 0; col < colCount; ++col)
            {
                char ch = colArray[col];
                QRect rect(col * doubleRadius + boarder, row * doubleRadius + boarder, doubleRadius - boarder * 2, doubleRadius - boarder * 2);
                if (ch == 'W')
                {
                    p.setBrush(Qt::white);
                    p.drawEllipse(rect);
                }
                else if (ch == 'B')
                {
                    p.setBrush(Qt::black);
                    p.drawEllipse(rect);
                }
                else
                {
                    if (_hint)
                    {
                        if (Engine::isDangerous(chess, row, col, Engine::otherColor(color)))
                        {
                            QPixmap pix(":/image/boom.png");
                            p.drawPixmap(rect, pix.scaled(rect.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
                        }
                    }
                }
                if (row == lastRow && col == lastCol)
                {
                    p.fillRect(col * doubleRadius + bigBoarder, row * doubleRadius + bigBoarder, doubleRadius - bigBoarder * 2, doubleRadius - bigBoarder * 2,
                               Qt::red);
                }
            }
        }
    }
    if (lock)
    {
        QFont f = p.font();
        f.setBold(true);
        f.setPointSize(boardSize / 16);
        p.setFont(f);
        p.setPen(QColor::fromRgb(255, 0, 0, 128));
        p.drawText(QRect(0, 0, boardSize, boardSize), Qt::AlignCenter, lockText);
        p.fillRect(0, 0, boardSize, boardSize, QColor::fromRgb(0, 0, 0, 100));
    }
}

void Board::mousePressEvent(QMouseEvent *e)
{
    if (lock)
    {
        return;
    }
    if (chess.count() == 0 || chess[0].count() == 0)
    {
        return;
    }
    int boardSize = min(width(), height());
    int rowCount = chess.count(),
        colCount = chess[0].count();
    Q_ASSERT(rowCount == colCount);
    int doubleRadius = boardSize / rowCount;
    boardSize = doubleRadius * rowCount;
    int radius = doubleRadius / 2;
    const int boarder = radius * 0.2;

    for (int row = 0; row < rowCount; ++row)
    {
        auto &rowArray = chess[row];
        for (int col = 0; col < colCount; ++col)
        {
            char ch = rowArray[col];
            QRect rect(col * doubleRadius + boarder, row * doubleRadius + boarder, doubleRadius - boarder * 2, doubleRadius - boarder * 2);
            if (rect.contains(e->pos()) && ch == ' ')
            {
                // local render
                rowArray[col] = color;
                emit clicked(row, col);
                // setLast(row, col); // test
                qDebug () << "win=" << Engine::findWin(chess);
                update();
                return;
            }
        }
    }
}

QVector<QVector<char> > Board::getChess() const
{
    return chess;
}

void Board::setLock(bool lock, const QString &lockText)
{
    this->lock = lock;
    this->lockText = lockText;
    update();
}

void Board::setBoard(const QVector<QVector<char> > &chess)
{
    this->chess = chess;
    setLast(-1, -1);
    update();
}

bool Board::getLock() const
{
    return lock;
}

void Board::setLast(int row, int col)
{
    lastRow = row;
    lastCol = col;
    update();
}

bool Board::hint() const
{
    return _hint;
}

void Board::setHint(bool hint)
{
    _hint = hint;
    update();
}
