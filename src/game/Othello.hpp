#ifndef OTHELLO_HPP
#define OTHELLO_HPP

#include<vector>
#include<sstream>

#include "GameBoard.hpp"
#include "OthelloSave.hpp"
#include "../tree/MiniOthello.hpp"


#define NB_PLAYER 2

class MiniOthello;


class Othello : public GameBoard {
public:
	Othello();
	Othello(const Othello& other) = default;
	Othello& operator=(const Othello& other) = default;

	Othello(const MiniOthello& mini); // dangereux (que pour la generation)

	virtual bool play(Pos pos) override;
	virtual bool canPlay(Pos pos) const override;

	virtual bool isFinish() const override;
	virtual PlayerID currentPlayer() const override;
	virtual PlayerID winner() const override;

	virtual void reset() override;

	int getTurn() const;
	int getScore(PlayerID p) const;
	const PlayerID* getBoard() const;

	const std::vector<Pos>& getPlayableSpots() const;

	void loadFromSavePtr(const GamePointer& gp);

private:

	void CountPossibilitY(PlayerID p);
	bool IsPlayable(PlayerID p, Pos pos) const;


private:

	PlayerID m_board[BOARD_COUNT + 1]; // Plateau[x:Horizontal * BOARD_SIZE + y:Vertical] + (case dehors)
	PlayerID m_curr;

	/// Compiled board info ([0] = Noir & [1] = Blanc)
	bool m_canPlayOn[BOARD_COUNT];
	int nb_possibility;
	int nb_piece[NB_PLAYER];
	std::vector<Pos> m_pos_vec;

#ifdef _DEBUG
public:
	Othello(float* src);
	std::string view;
#endif // _DEBUG
};

class OthelloSave : public Othello {
public:
	OthelloSave() = default;
	OthelloSave(const OthelloSave& other) = default;
	OthelloSave& operator=(const OthelloSave& other) = default;

	void reset() override;
	virtual bool play(Pos pos) override;
	bool unplay();

	const GameSave& getGameSave() const;
	GamePointer getGamePointer() const;

private:
	GameSave m_game_save;

};

#endif