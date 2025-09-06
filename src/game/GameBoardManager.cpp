#include "GameBoardManager.hpp"

GameBoardManager::GameBoardManager(GameBoard* game, GamePlayer* p1, GamePlayer* p2) :
	m_game_board(game), m_game_players()
{
	m_game_players[0] = p1;
	m_game_players[1] = p2;
}

bool GameBoardManager::waiting() const
{
	return !m_game_players[*m_game_board->currentPlayer()]->hasNext();
}

bool GameBoardManager::isFinish() const
{
	return m_game_board->isFinish();
}

void GameBoardManager::reset(GamePlayer* p1, GamePlayer* p2) {
	if (p1 != nullptr)
		m_game_players[0] = p1;
	if (p2 != nullptr)
		m_game_players[1] = p2;

	m_game_board->reset();
	m_game_players[0]->reset();
	m_game_players[1]->reset();
}

Pos GameBoardManager::playTurn()
{
	GamePlayer* current = m_game_players[*m_game_board->currentPlayer()];

	Pos play = current->next();
	if (!m_game_board->canPlay(play)) {
		current->invalidPlay();
		return INVALID;
	}

	m_game_board->play(play);
	m_game_players[0]->updateBoardFromPlay(play);
	m_game_players[1]->updateBoardFromPlay(play);

	return play;
}

void GameBoardManager::passParam(Pos pos, PlayerID p)
{
	if (p == PlayerID::Nobody)
		p = m_game_board->currentPlayer();
	m_game_players[*p]->passParam(pos);
}

const GameBoard* GameBoardManager::currentState() const
{
	return m_game_board;
}
