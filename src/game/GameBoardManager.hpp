#ifndef GAME_BOARD_MANAGER_HPP
#define GAME_BOARD_MANAGER_HPP

#include "GameBoard.hpp"
#include "GamePlayer.hpp"


class GameBoardManager {
public:
	GameBoardManager(GameBoard* game, GamePlayer* p1, GamePlayer* p2);

	bool waiting() const;
	Pos playTurn();
	bool isFinish() const;
	void reset(GamePlayer* p1 = nullptr, GamePlayer* p2 = nullptr);
	void passParam(Pos pos, PlayerID p = PlayerID::Nobody);
	const GameBoard* currentState() const;

private:
	GameBoard* m_game_board;
	GamePlayer* m_game_players[2];

};


#endif