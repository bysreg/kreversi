/* Yo Emacs, this -*- C++ -*-
 *******************************************************************
 *******************************************************************
 *
 *
 * KREVERSI
 *
 *
 *******************************************************************
 *
 * A Reversi (or sometimes called Othello) game
 *
 *******************************************************************
 *
 * Created 1997 by Mario Weilguni <mweilguni@sime.com>. This file
 * is ported from Mats Luthman's <Mats.Luthman@sylog.se> JAVA applet.
 * Many thanks to Mr. Luthman who has allowed me to put this port
 * under the GNU GPL. Without his wonderful game engine kreversi
 * would be just another of those Reversi programs a five year old
 * child could beat easily. But with it it's a worthy opponent!
 *
 * If you are interested on the JAVA applet of Mr. Luthman take a
 * look at http://www.sylog.se/~mats/
 *
 *******************************************************************
 *
 * This file is part of the KDE project "KREVERSI"
 *
 * KREVERSI is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * KREVERSI is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with KREVERSI; see the file COPYING.  If not, write to
 * the Free Software Foundation, 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 *******************************************************************
 */

// The class Engine produces moves from a Game object through calls to the
// function ComputeMove().
//
// First of all: this is meant to be a simple example of a game playing
// program. Not everything is done in the most clever way, particularly not
// the way the moves are searched, but it is hopefully made in a way that makes
// it easy to understand. The function ComputeMove2() that does all the work
// is actually not much more than a hundred lines. Much could be done to
// make the search faster though, I'm perfectly aware of that. Feel free
// to experiment.
//
// The method used to generate the moves is called minimax tree search with
// alpha-beta pruning to a fixed depth. In short this means that all possible
// moves a predefined number of moves ahead are either searched or refuted
// with a method called alpha-beta pruning. A more thorough explanation of
// this method could be found at the world wide web at http:
// //yoda.cis.temple.edu:8080/UGAIWWW/lectures96/search/minimax/alpha-beta.html
// at the time this was written. Searching for "minimax" would also point
// you to information on this subject. It is probably possible to understand
// this method by reading the source code though, it is not that complicated.
//
// At every leaf node at the search tree, the resulting position is evaluated.
// Two things are considered when evaluating a position: the number of pieces
// of each color and at which squares the pieces are located. Pieces at the
// corners are valuable and give a high value, and having pieces at squares
// next to a corner is not very good and they give a lower value. In the
// beginning of a game it is more important to have pieces on "good" squares,
// but towards the end the total number of pieces of each color is given a
// higher weight. Other things, like how many legal moves that can be made in a
// position, and the number of pieces that can never be turned would probably
// make the program stronger if they were considered in evaluating a position,
// but that would make things more complicated (this was meant to be very
// simple example) and would also slow down computation (considerably?).
//
// The member m_board[10][10]) holds the current position during the
// computation. It is initiated at the start of ComputeMove() and
// every move that is made during the search is made on this board. It should
// be noted that 1 to 8 is used for the actual board, but 0 and 9 can be
// used too (they are always empty). This is practical when turning pieces
// when moves are made on the board. Every piece that is put on the board
// or turned is saved in the stack m_squarestack (see class SquareStack) so
// every move can easily be reversed after the search in a node is completed.
//
// The member m_bc_board[][] holds board control values for each square
// and is initiated by a call to the function private void SetupBcBoard()
// from Engines constructor. It is used in evaluation of positions except
// when the game tree is searched all the way to the end of the game.
//
// The two members m_coord_bit[9][9] and m_neighbor_bits[9][9] are used to
// speed up the tree search. This goes against the principle of keeping things
// simple, but to understand the program you do not need to understand them
// at all. They are there to make it possible to throw away moves where
// the piece that is played is not adjacent to a piece of opposite color
// at an early stage (because they could never be legal). It should be
// pointed out that not all moves that pass this test are legal, there will
// just be fewer moves that have to be tested in a more time consuming way.
//
// There are also two other members that should be mentioned: Score m_score
// and Score m_bc_score. They hold the number of pieces of each color and
// the sum of the board control values for each color during the search
// (this is faster than counting at every leaf node).
//

// The classes SquareStackEntry and SquareStack implement a
// stack that is used by Engine to store pieces that are turned during
// searching (see ComputeMove()).
//
// The class MoveAndValue is used by Engine to store all possible moves
// at the first level and the values that were calculated for them.
// This makes it possible to select a random move among those with equal
// or nearly equal value after the search is completed.


#include "Engine.h"
#include <qapplication.h>

const int Engine::LARGEINT = 99999;
const int Engine::ILLEGAL_VALUE = 8888888;
const int Engine::BC_WEIGHT = 3;

inline void SquareStackEntry::setXY(int x, int y) {
  m_x = x;
  m_y = y;
}

#if !defined(__GNUC__)

ULONG64::ULONG64() : QBitArray(64) {
  fill(0);
}

ULONG64::ULONG64( unsigned int value ) : QBitArray(64) {
  fill(0);
  for(int i = 0; i < 32; i++) {
    setBit(i, (bool)(value & 1));
    value >>= 1;
  }
}

void ULONG64::shl() {
  for(int i = 63; i > 0; i--)
    setBit(i, testBit(i-1));
  setBit(0, 0);
}

#endif


SquareStackEntry::SquareStackEntry() {
  setXY(0,0);
}


SquareStack::SquareStack() {
  init(0);
}


SquareStack::SquareStack(int size) {
  init(size);
}


void SquareStack::resize(int size) {
  m_squarestack.resize(size);
}


void SquareStack::init(int size) {
  resize(size);
  m_top = 0;
  for (int i=0; i<size; i++)
    m_squarestack[i].setXY(0,0);
}


inline SquareStackEntry SquareStack::Pop() {
  return m_squarestack[--m_top];
}


inline void SquareStack::Push(int x, int y) {
  m_squarestack[m_top].m_x = x;
  m_squarestack[m_top++].m_y = y;
}


inline void MoveAndValue::setXYV(int x, int y, int value) {
  m_x = x;
  m_y = y;
  m_value = value;
}


MoveAndValue::MoveAndValue() {
  setXYV(0,0,0);
}


MoveAndValue::MoveAndValue(int x, int y, int value) {
  setXYV(x, y, value);
}


Engine::Engine(int st, int sd) : SuperEngine(st, sd) {
  SetupBcBoard();
  SetupBits();
}


Engine::Engine(int st) : SuperEngine(st) {
  SetupBcBoard();
  SetupBits();
}


Engine::Engine() : SuperEngine(1) {
  SetupBcBoard();
  SetupBits();
}


// keep GUI alive
void Engine::yield() {
  qApp->processEvents();
}

Move Engine::computeMove(Game g) {
  m_exhaustive = false;

  Color color = g.toMove();

  if (color == Nobody)
    return Move(-1, -1, Nobody);

  // Figure out the current score
  m_score.set(White, g.score(White));
  m_score.set(Black, g.score(Black));
  // If the game just started...
  if (m_score.score(White) + m_score.score(Black) == 4)
    return ComputeFirstMove(g);

  // JAVA m_board = new int[10][10];
  //m_squarestack = new SquareStack(3000); // More than enough...
  m_squarestack.init(3000);
  m_depth = m_strength;

  if (m_score.score(White) + m_score.score(Black) +
      m_depth + 3 >= 64)
    m_depth =
      64 - m_score.score(White) - m_score.score(Black);
  else if (m_score.score(White) + m_score.score(Black) +
	   m_depth + 4 >= 64)
    m_depth += 2;
  else if (m_score.score(White) + m_score.score(Black) +
	   m_depth + 5 >= 64)
    m_depth++;

  if (m_score.score(White) + m_score.score(Black) +
      m_depth >= 64) m_exhaustive = true;

  m_coeff =
    100 - (100*
	   (m_score.score(White) + m_score.score(Black) +
	    m_depth - 4))/60;

  m_nodes_searched = 0;

  for (uint x=0; x<10; x++)
    for (uint y=0; y<10; y++)
      m_board[x][y] = Nobody;

  for (uint x=1; x<9; x++)
    for (uint y=1; y<9; y++)
      m_board[x][y] = g.color(x, y);

  m_bc_score.set(White, CalcBcScore(White));
  m_bc_score.set(Black, CalcBcScore(Black));

  ULONG64 colorbits = ComputeOccupiedBits(color);
  ULONG64 opponentbits = ComputeOccupiedBits(opponent(color));

  int maxval = -LARGEINT;
  int max_x = 0;
  int max_y = 0;

  MoveAndValue moves[60];
  int number_of_moves = 0;
  int number_of_maxval = 0;

  setInterrupt(false);

  ULONG64 null_bits;
  null_bits = 0;

  //struct tms tmsdummy;
  //long starttime = times(&tmsdummy);
  // Compute this once at the start of the loops.
//  int high = 20 - m_strength;

  for (int x=1; x<9; x++)
    for (int y=1; y<9; y++)
      if (m_board[x][y] == Nobody &&
	  (m_neighbor_bits[x][y] & opponentbits) != null_bits)
	{

	  int val = ComputeMove2(x, y, color, 1, maxval,
				 colorbits, opponentbits);

	  if (val != ILLEGAL_VALUE)
	    {
	      moves[number_of_moves++].setXYV(x, y, val);

	      if (val > maxval)
		{
		  // Make it so it is easy for us to "miss" something.
		  // i.e. more relistic.  Also makes m_strength mean more.
	          int randi = m_random.getLong(7);
		  if(maxval == -LARGEINT ||
                    randi < (int)m_strength ){
	            maxval = val;
		    max_x = x;
		    max_y = y;
		    number_of_maxval = 1;
		  }
		}
	      else if (val == maxval) number_of_maxval++;
	    }

	  if (interrupt()) break;
	}

  // long endtime = times(&tmsdummy);

  if (number_of_maxval > 1)
    {
      int r = m_random.getLong(number_of_maxval) + 1;

      int i;

      for (i=0; i < number_of_moves; i++)
	if (moves[i].m_value == maxval && --r <= 0) break;

      max_x = moves[i].m_x;
      max_y = moves[i].m_y;
    }

  if (interrupt()) {
    return Move(-1, -1, Nobody);
  } else if (maxval != -LARGEINT) {
    return Move(max_x, max_y, color);
  } else {
    return Move(-1, -1, Nobody);
  }
}


Move Engine::ComputeFirstMove(Game g) {
  int r;
  Color color = g.toMove();

  r = m_random.getLong(4) + 1;

  if (color == White)
    {
      if (r == 1) return Move(3, 5, color);
      else if (r == 2) return  Move(4, 6, color);
      else if (r == 3) return  Move(5, 3, color);
      else return  Move(6, 4, color);
    }
  else
    {
      if (r == 1) return  Move(3, 4, color);
      else if (r == 2) return  Move(5, 6, color);
      else if (r == 3) return  Move(4, 3, color);
      else return  Move(6, 5, color);
    }
}


int Engine::ComputeMove2(int xplay, int yplay, Color color, int level,
			 int cutoffval, ULONG64 colorbits,
			 ULONG64 opponentbits)
{
  int number_of_turned = 0;
  SquareStackEntry mse;
  Color opponent = ::opponent(color);

  m_nodes_searched++;

  m_board[xplay][yplay] = color;
  colorbits |= m_coord_bit[xplay][yplay];
  m_score.inc(color);
  m_bc_score.add(color, m_bc_board[xplay][yplay]);

  ///////////////////
  // Turn all pieces:
  ///////////////////

  for (int xinc=-1; xinc<=1; xinc++)
    for (int yinc=-1; yinc<=1; yinc++)
      if (xinc != 0 || yinc != 0)
	{
	  int x, y;

	  for (x = xplay+xinc, y = yplay+yinc; m_board[x][y] == opponent;
	       x += xinc, y += yinc)
	    ;

	  if (m_board[x][y] == color)
	    for (x -= xinc, y -= yinc; x != xplay || y != yplay;
		 x -= xinc, y -= yinc)
	      {
		m_board[x][y] = color;
		colorbits |= m_coord_bit[x][y];
		opponentbits &= ~m_coord_bit[x][y];
		m_squarestack.Push(x, y);
		m_bc_score.add(color, m_bc_board[x][y]);
		m_bc_score.sub(opponent, m_bc_board[x][y]);
		number_of_turned++;
	      }
	}

  int retval = -LARGEINT;

  if (number_of_turned > 0)
    {
      //////////////
      // Legal move:
      //////////////

      m_score.add(color, number_of_turned);
      m_score.sub(opponent, number_of_turned);

      if (level >= m_depth) retval = EvaluatePosition(color); // Terminal node
      else
	{
	  int maxval = TryAllMoves(opponent, level, cutoffval, opponentbits,
				   colorbits);

	  if (maxval != -LARGEINT) retval = -maxval;
	  else
	    {
	      ///////////////////////////////////////////////////////////////
	      // No possible move for the opponent, it is colors turn again:
	      ///////////////////////////////////////////////////////////////
	      retval= TryAllMoves(color, level, -LARGEINT, colorbits,
				  opponentbits);

	      if (retval == -LARGEINT)
		{
		  ///////////////////////////////////////////////
		  // No possible move for anybody => end of game:
		  ///////////////////////////////////////////////

		  int finalscore =
		    m_score.score(color) - m_score.score(opponent);

		  if (m_exhaustive) retval = finalscore;
		  else
		    {
		      // Take a sure win and avoid a sure loss (may not be optimal):

		      if (finalscore > 0) retval = LARGEINT - 65 + finalscore;
		      else if (finalscore < 0) retval = -(LARGEINT - 65 + finalscore);
		      else retval = 0;
		    }
		}
	    }
	}

      m_score.add(opponent, number_of_turned);
      m_score.sub(color, number_of_turned);
    }

  /////////////////
  // Restore board:
  /////////////////

  for (int i = number_of_turned; i > 0; i--)
    {
      mse = m_squarestack.Pop();
      m_bc_score.add(opponent, m_bc_board[mse.m_x][mse.m_y]);
      m_bc_score.sub(color, m_bc_board[mse.m_x][mse.m_y]);
      m_board[mse.m_x][mse.m_y] = opponent;
    }

  m_board[xplay][yplay] = Nobody;
  m_score.sub(color, 1);
  m_bc_score.sub(color, m_bc_board[xplay][yplay]);

  if (number_of_turned < 1 || interrupt()) return ILLEGAL_VALUE;
  else return retval;
}


int Engine::TryAllMoves(Color opponent, int level, int cutoffval,
			ULONG64 opponentbits, ULONG64 colorbits)
{
  int maxval = -LARGEINT;

  // keep GUI alive
  yield();

  ULONG64 null_bits;
  null_bits = 0;

  for (int x=1; x<9; x++)
    {
      for (int y=1; y<9; y++)
	if (m_board[x][y] == Nobody &&
	    (m_neighbor_bits[x][y] & colorbits) != null_bits)
	  {
	    int val = ComputeMove2(x, y, opponent, level+1, maxval, opponentbits,
				   colorbits);

	    if (val != ILLEGAL_VALUE && val > maxval)
	      {
		maxval = val;
		if (maxval > -cutoffval || interrupt()) break;
	      }
	  }

      if (maxval > -cutoffval || interrupt()) break;
    }

  if (interrupt()) return -LARGEINT;
  return maxval;
}


int Engine::EvaluatePosition(Color color)
{
  int retval;

  Color opponent = ::opponent(color);
  int score_color = m_score.score(color);
  int score_opponent = m_score.score(opponent);

  if (m_exhaustive) retval = score_color - score_opponent;
  else
    {
      retval = (100-m_coeff) *
	(m_score.score(color) - m_score.score(opponent)) +
	m_coeff * BC_WEIGHT *
	(m_bc_score.score(color)-m_bc_score.score(opponent));
    }

  return retval;
}


void Engine::SetupBits()
{
  //m_coord_bit = new long[9][9];
  //m_neighbor_bits = new long[9][9];

  ULONG64 bits = 1;

  for (int i=1; i < 9; i++)
    for (int j=1; j < 9; j++)
      {
	m_coord_bit[i][j] = bits;
#if !defined(__GNUC__)
	bits.shl();
#else
	bits *= 2;
#endif
      }

  for (int i=1; i < 9; i++)
    for (int j=1; j < 9; j++)
      {
	m_neighbor_bits[i][j] = 0;

	for (int xinc=-1; xinc<=1; xinc++)
	  for (int yinc=-1; yinc<=1; yinc++)
	    if (xinc != 0 || yinc != 0)
	      if (i + xinc > 0 && i + xinc < 9 && j + yinc > 0 && j + yinc < 9)
		m_neighbor_bits[i][j] |= m_coord_bit[i + xinc][j + yinc];
      }
}


void Engine::SetupBcBoard()
{
  // JAVA m_bc_board = new int[9][9];

  for (int i=1; i < 9; i++)
    for (int j=1; j < 9; j++)
      {
	if (i == 2 || i == 7) m_bc_board[i][j] = -1; else m_bc_board[i][j] = 0;
	if (j == 2 || j == 7) m_bc_board[i][j] -= 1;
      }

  m_bc_board[1][1] = 2;
  m_bc_board[8][1] = 2;
  m_bc_board[1][8] = 2;
  m_bc_board[8][8] = 2;

  m_bc_board[1][2] = -1;
  m_bc_board[2][1] = -1;
  m_bc_board[1][7] = -1;
  m_bc_board[7][1] = -1;
  m_bc_board[8][2] = -1;
  m_bc_board[2][8] = -1;
  m_bc_board[8][7] = -1;
  m_bc_board[7][8] = -1;
}


int Engine::CalcBcScore(Color color)
{
  int sum = 0;

  for (int i=1; i < 9; i++)
    for (int j=1; j < 9; j++)
      if (m_board[i][j] == color)
	sum += m_bc_board[i][j];

  return sum;
}


ULONG64 Engine::ComputeOccupiedBits(Color color)
{
  ULONG64 retval = 0;

  for (int i=1; i < 9; i++)
    for (int j=1; j < 9; j++)
      if (m_board[i][j] == color) retval |= m_coord_bit[i][j];

  return retval;
}

