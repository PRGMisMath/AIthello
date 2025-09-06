#ifndef MINI_OTHELLO_HPP
#define MINI_OTHELLO_HPP

#include "../game/GameBoard.hpp"
#include<cstdint>
#include<vector>

struct _uint128_t {
	uint64_t low;
	uint64_t high;
};


// Classe compactee utile pour la generation rapide d'arbres de jeux
// Moins utile pour une vraie partie
class MiniOthello {
public:
	MiniOthello();
	MiniOthello(const MiniOthello& other) = default;
	MiniOthello& operator=(const MiniOthello& other) = default;

	bool play(Pos pos);

	// Vrai ==> plus de coup
	// La reciproque n'est vrai que si nextBoards a ete appelee
	bool isFinish() const;
	int getScore(PlayerID player) const;
	int getTurn() const;

	Pos lastPlay() const;
	PlayerID lastPlayer() const;

	const std::vector<MiniOthello> nextBoards() const;
	int countMoves(PlayerID player) const;

	void toRealBoard(PlayerID* dest) const;

private:
	// Retourne une piece
	// Suppose que c'est deja une case occupe
	inline void FlipPiecE(const _uint128_t& maskPos);
	// Place une piece
	// Suppose que la case est vide
	inline void PlacePiece(const _uint128_t& maskPos, const uint64_t& maskPlayer);
	inline bool IsPlayerHerE(const _uint128_t& maskPos, const uint64_t& maskPlayer) const;
	inline bool IsEmptyHerE(const _uint128_t& maskPos) const;

	const std::vector<MiniOthello> GenerateNextBoardS(PlayerID curr) const;

private:
	// Board format :
	//  - 2 bits par case 
	//  - 0x00 = Nobody
	//  - 0x01 = Black ; 0x10 : White
	_uint128_t m_board;
	unsigned char nb_piece[2];
	// Dernier joueur a avoir joue
	// Remarque : dans play, c'est le joueur que l'on essaie de faire de jouer (donc celui qui aura joue une fois le coup joue)
	PlayerID m_curr; 
	mutable unsigned char m_flag;
	Pos m_l_play = INVALID;

};

#endif // !MINI_OTHELLO_HPP
