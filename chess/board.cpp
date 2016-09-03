#include "board.h"

#include <QDebug>
#include <QPainter>

Board::Board(QWidget *parent) : QWidget(parent)
{

}

void Board::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    int boardSize = min(width(), height());
    if (_board.count() > 0 && _board[0].count() > 0)
    {
        int rowCount = _board.count(),
            colCount = _board[0].count();
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

        QSize rectSize(doubleRadius - boarder * 2, doubleRadius - boarder * 2);
        QPixmap pixForbid = QPixmap(":/image/forbid.png").scaled(rectSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        QPixmap pixBoom = QPixmap(":/image/boom.png").scaled(rectSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        for (int row = 0; row < rowCount; ++row)
        {
            const auto &colArray = _board[row];
            for (int col = 0; col < colCount; ++col)
            {
                chess_t ch = colArray[col];
                QRect rect(col * doubleRadius + boarder, row * doubleRadius + boarder, rectSize.width(), rectSize.height());
                if (ch == CH_WHITE)
                {
                    p.setBrush(Qt::white);
                    p.drawEllipse(rect);
                }
                else if (ch == CH_BLACK)
                {
                    p.setBrush(Qt::black);
                    p.drawEllipse(rect);
                }
                else if (ch == CH_FORBID)
                {
                    p.drawPixmap(rect, pixForbid);
                }
                else
                {
                    if (Engine::isColor(myColor) && _hint)
                    {
                        bool dangerous = false;
                        chess_t color = Engine::nextColor(myColor);
                        while (color != myColor)
                        {
                            if (Engine::isDangerous(_board, row, col, color))
                            {
                                dangerous = true;
                                break;
                            }
                            color = Engine::nextColor(color);
                        }
                        if (dangerous)
                        {
                            p.drawPixmap(rect, pixBoom);
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
    if (_lock)
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
    if (_lock)
    {
        return;
    }
    if (_board.count() == 0 || _board[0].count() == 0)
    {
        return;
    }
    int boardSize = min(width(), height());
    int rowCount = _board.count(),
        colCount = _board[0].count();
    Q_ASSERT(rowCount == colCount);
    int doubleRadius = boardSize / rowCount;
    boardSize = doubleRadius * rowCount;
    int radius = doubleRadius / 2;
    const int boarder = radius * 0.2;

    // TODO: improve
    for (int row = 0; row < rowCount; ++row)
    {
        for (int col = 0; col < colCount; ++col)
        {
            QRect rect(col * doubleRadius + boarder, row * doubleRadius + boarder, doubleRadius - boarder * 2, doubleRadius - boarder * 2);
            if (rect.contains(e->pos()))
            {
                click(row, col);
                return;
            }
        }
    }
}

board_t Board::board() const
{
    return _board;
}

void Board::setLock(bool lock, const QString &lockText)
{
    this->_lock = lock;
    this->lockText = lockText;
    update();
}

void Board::setBoard(const board_t &chess)
{
    this->_board = chess;
    setLast(-1, -1);
    update();
}

bool Board::lock() const
{
    return _lock;
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

int Board::rev() const
{
    return _rev;
}

void Board::setHint(bool hint)
{
    _hint = hint;
    update();
}

void Board::click(int row, int col)
{
    if (_lock)
    {
        return;
    }
    if (_board.count() == 0 || _board[0].count() == 0)
    {
        return;
    }
    if (!Engine::isBlock(_board, row, col))
    {
        // local render
        _board[row][col] = myColor;
        ++_rev;
        update();
        emit clicked(row, col);
    }
}
