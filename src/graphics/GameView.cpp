#include "GameView.hpp"

#include "Othello.hpp"

sf::IntRect GameView::poundTextureRect(PlayerID p)
{
	return { 
		{20 * (*p), 0}, {20, 20}
	};
}

GameView::GameView(const GameAppear& gapp, const PlayerID* board, int case_size) :
	m_gapp(gapp), m_board(board), m_case_size(case_size),
	m_cases(sf::PrimitiveType::Triangles, 6 * gapp.plateau_size.x * gapp.plateau_size.y),
	m_lines(sf::PrimitiveType::Lines, 2 * (gapp.plateau_size.x + gapp.plateau_size.y + 2))
{
	this->InitCasesLineS();
}

void GameView::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	states.transform *= this->getTransform();
	states.texture = m_gapp.tex_pion;

	target.draw(m_cases, states);
	states.texture = NULL;
	target.draw(m_lines, states);
}

sf::Vector2f GameView::getSize() const
{
	return { (float)m_case_size * BOARD_SIZE, (float)m_case_size * BOARD_SIZE };
}

int GameView::getCaseSize() const
{
	return m_case_size;
}

void GameView::setCaseSize(int case_size)
{
	m_case_size = case_size;
	this->InitCasesLineS();
}

void GameView::update()
{
	for (int x = 0; x < m_gapp.plateau_size.x; ++x)
		for (int y = 0; y < m_gapp.plateau_size.y; ++y)
			update(x, y);
}

void GameView::update(int x, int y)
{
	for (int c = 0; c < 6; ++c) {
		if (m_board[x * m_gapp.plateau_size.y + y] == PlayerID::Nobody)
			m_cases[6 * (x * m_gapp.plateau_size.y + y) + c].texCoords = { 0,0 };
		else
			m_cases[6 * (x * m_gapp.plateau_size.y + y) + c].texCoords = sf::Vector2f(
				m_gapp.tex_size.x * ((c == 1 || c == 2 || c == 5) + *m_board[x * m_gapp.plateau_size.y + y]),
				m_gapp.tex_size.y * (c == 2 || c == 4 || c == 5)
			);
	}
}


const PlayerID* GameView::getBoard() const
{
	return m_board;
}

void GameView::setBoard(const PlayerID* board)
{
	m_board = board;
}

void GameView::InitCasesLineS()
{
	const sf::Vector2i& Splateau = m_gapp.plateau_size;

	// Lines
	for (int x = 0; x <= Splateau.x; ++x) {
		m_lines[2 * x] = { sf::Vector2f(m_case_size * x, 0), sf::Color::Black };
		m_lines[2 * x + 1] = { sf::Vector2f(m_case_size * x, m_case_size * Splateau.y), sf::Color::Black };
	}
	for (int y = 0; y <= Splateau.y; ++y) {
		m_lines[2 * (Splateau.x + 1 + y)] = { sf::Vector2f(0, m_case_size * y), sf::Color::Black };
		m_lines[2 * (Splateau.x + 1 + y) + 1] = { sf::Vector2f(m_case_size * Splateau.x, m_case_size * y), sf::Color::Black };
	}

	// Case
	for (int x = 0; x < Splateau.x; ++x)
		for (int y = 0; y < Splateau.y; ++y)
			for (int c = 0; c < 6; ++c)
				m_cases[6 * (x * Splateau.y + y) + c] = { sf::Vector2f(
					m_case_size * (y + (c == 1 || c == 2 || c == 5)),
					m_case_size * (x + (c == 2 || c == 4 || c == 5))
				) }; // On inverse pour etre en accord avec la FFOthello
	this->update();
}
