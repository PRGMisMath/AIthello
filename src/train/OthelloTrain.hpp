#ifndef OTHELLO_TRAIN_HPP
#define OTHELLO_TRAIN_HPP


#include "../tree/GameTree.hpp"
#include "../tree/MinMaxTree.hpp"
#include "../game/Othello.hpp"

#include "../nl/NLBackprop.h"

constexpr size_t nldatasize = 2 * BOARD_COUNT + 1;


typedef Pos (*SymetryFunc)(Pos);

inline Pos xyToPos(int x, int y) {
	return {(unsigned char)(x + y * BOARD_SIZE)};
}

static Pos sym_id(Pos p) {
	return p;
}

static Pos sym_rot90(Pos p)  {
	return xyToPos(y(p), BOARD_SIZE - 1 - x(p));
}

static Pos sym_rot180(Pos p) {
	return xyToPos(BOARD_SIZE - 1 - x(p), BOARD_SIZE - 1 - y(p));
}

static Pos sym_rot270(Pos p) {
	return xyToPos(BOARD_SIZE - 1 - y(p), x(p));
}

static Pos sym_flipV(Pos p) {
	return xyToPos(x(p), BOARD_SIZE - 1 - y(p));
}

static Pos sym_flipH(Pos p) {
	return xyToPos(BOARD_SIZE - 1 - x(p), y(p));
}

static Pos sym_diag(Pos p) {
	return xyToPos(y(p), x(p));
}

static Pos sym_antidiag(Pos p) {
	return xyToPos(BOARD_SIZE - 1 - y(p), BOARD_SIZE - 1 - x(p));
}

inline SymetryFunc symetries[8] = {
	sym_id,
	sym_rot90,
	sym_rot180,
	sym_rot270,
	sym_flipV,
	sym_flipH,
	sym_diag,
	sym_antidiag
};

// Remarque :
// Le bit pour indiquer le joueur est indispensable car 2 situations identiques ne donnent
// pas le meme resultat en fonction du joueur dont c'est le tour.

/**
 * Transform a game state into usuable data for ML purposes.
 * @details Generate a `2 * BOARD_COUNT` data array representing the presence of a said pound
 * of a said color in a given place (one `BOARD_COUNT` for each player).
 * Data concerning the evaluating player need to be added manually
 * as it is the last player to have play that is relevant.
 * @param game Game to convert
 * @param dest Destination
 * @param trans Transformation
 * @param inverse Inverse color to ensure symetry of evaluation
 */
void boardToData(const Othello& game, float* dest, SymetryFunc trans, bool inverse = false);

/**
 * Special case for faster conversion
 * @param game Game to convert
 * @param dest Destination
 */
void boardToData(const Othello& game, float* dest);


struct V1DataSet {
	NLTrainData* set;
	size_t sset; // nb de set
	size_t strain; // taille d'un set
	float* _block;
};

class NeuronalAI : public TreeAI {
public:
	explicit NeuronalAI(NLTrainAI* tai, size_t depth);

	void reset() override;

	float eval(const MiniOthello& game);

protected:
	void treeEvoluate() override;

private:
	NLTrainAI* m_tai;
	MinMaxWalker m_wlk;
	size_t m_depth;

};



#endif // !OTHELLO_TRAIN_HPP
