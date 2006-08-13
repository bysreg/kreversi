#ifndef KREVERSI_GAME_H
#define KREVERSI_GAME_H

#include <QObject>

#include "commondefs.h"

class KReversiBoard;
class Engine;

/**
 *  KReversiGame incapsulates all of the game logic.
 *  It creates KReversiBoard and manages a chips on it.
 *  Whenever the board state changes it emits corresponding signals.
 *  FIXME dimsuz: enumerate them briefly
 *  The idea is also to abstract from any graphic representation of the game process
 */
class KReversiGame : public QObject
{
    Q_OBJECT
public:
    KReversiGame();
    ~KReversiGame();
    /**
     *  @returns the board so the callers can examine its current state
     */
    const KReversiBoard& board() const;
    ChipColor currentPlayer() const { return m_curPlayer; }
    // NOTE: this is just a wrapper around KReversiBoard::playerScore
    // Maybe consider merging KReversiBoard into this class?
    // same applies to chipColorAt
    int playerScore( ChipColor player ) const;
    // NOTE: this is just a wrapper around KReversiBoard::playerScore
    ChipColor chipColorAt( int row, int col ) const;
    /**
     *  This will put the player chip at row, col.
     *  If that is possible of course
     */
    void makePlayerMove(int row, int col);
    /**
     *  This function will make computer decide where he 
     *  wants to put his chip... and he'll put it there!
     */
    void makeComputerMove();
    /**
     *  Returns true, if it's computer's turn now
     */
    bool computersTurn() const { return m_curPlayer == m_computerColor; }
signals:
    void boardChanged();
    void currentPlayerChanged();
private:
    enum Direction { Up, Down, Right, Left, UpLeft, UpRight, DownLeft, DownRight };
    /**
     * This function will tell you if the move is possible.
     * That's why it was given such a name ;)
     */
    bool isMovePossible( const KReversiMove& move ) const; 
    /**
     *  Searches for "chunk" in direction dir for move.
     *  As my English-skills are somewhat limited, let me introduce 
     *  new terminology ;).
     *  I'll define a "chunk" of chips for color "C" as follows:
     *  (let "O" be the color of the opponent for "C")
     *  CO[O]C <-- this is a chunk
     *  where [O] is one or more opponent's pieces
     */
    bool hasChunk( Direction dir, const KReversiMove& move) const;
    /**
     *  Performs move, i.e. marks all the chips that player wins with
     *  this move with current player color
     */
    void makeMove( const KReversiMove& move );
    /**
     *  Board itself
     */
    KReversiBoard *m_board;
    /**
     *  Color of the current player
     */
    ChipColor m_curPlayer;
    /**
     *  The color of the computer played chips
     */
    ChipColor m_computerColor;
    /**
     *  Our AI
     */
    Engine *m_engine;
};
#endif
