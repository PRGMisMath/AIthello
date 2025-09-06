#ifndef GAME_VIEW_HPP
#define GAME_VIEW_HPP

#include<SFML/Graphics.hpp>
#include "../game/GameBoard.hpp"

struct GameAppear {
	sf::Vector2i plateau_size;
	int nb_player;
	const sf::Texture* tex_pion;
	sf::Vector2i tex_size;
};

class GameView : public sf::Drawable, public sf::Transformable {
public:
	static sf::IntRect poundTextureRect(PlayerID p);

public:
	GameView(const GameAppear& gapp, const PlayerID* board, int case_size = 30);

	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;

	sf::Vector2f getSize() const;
	int getCaseSize() const;
	void setCaseSize(int case_size);

	void update();
	void update(int x, int y);

	const PlayerID* getBoard() const;
	void setBoard(const PlayerID* board);

private:
	void InitCasesLineS();

private:
	sf::VertexArray m_cases, m_lines;
	GameAppear m_gapp;
	const PlayerID* m_board;
	int m_case_size;

};

#endif // !GAME_VIEW_HPP
