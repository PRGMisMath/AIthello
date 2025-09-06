#include "OthelloTrain.hpp"

// Conversion (row, col) -> index





void boardToData(const Othello& game, float* dest,  SymetryFunc trans, bool inverse)
{
	const PlayerID* src = game.getBoard();
	if (!inverse) {
		for (Pos p = START; valid(p); ++p) {
			dest[2 * *p] = (src[*trans(p)] == PlayerID::Noir);
			dest[2 * (int)*p + 1] = (src[*trans(p)] == PlayerID::Blanc);
		}
	}
	else {
		for (Pos p = START; valid(p); ++p) {
			dest[2 * *p] = (src[*trans(p)] == PlayerID::Blanc);
			dest[2 * (int)*p + 1] = (src[*trans(p)] == PlayerID::Noir);
		}
	}
}

void boardToData(const Othello &game, float *dest) {
	const PlayerID* src = game.getBoard();
	for (Pos p = START; valid(p); ++p) {
		dest[2 * *p] = (src[*p] == PlayerID::Noir);
		dest[2 * (int)*p + 1] = (src[*p] == PlayerID::Blanc);
	}
}

NeuronalAI::NeuronalAI(NLTrainAI *tai, size_t depth) :
	TreeAI(&m_wlk, [this](const MiniOthello& game) mutable { return eval(game);}),
	m_tai(tai), m_wlk(m_tree), m_depth(depth)
{
}

void NeuronalAI::reset() {
}

float NeuronalAI::eval(const MiniOthello &game) {
	auto netw = m_tai->netw;
	boardToData(game, netw->p_process1);
	netw->p_process1[nldatasize - 1] = (game.lastPlayer() == PlayerID::Noir);
	float res;
	eval_network(netw, netw->p_process1, &res);
	return res * 64;
}

void NeuronalAI::treeEvoluate() {
	if (m_wlk.getRoot()->savestate.getTurn() <= 5) {
		m_wlk.alphabeta(noheur, 1);
	}
	else if (m_wlk.getRoot()->savestate.getTurn() >= 49) {
		m_wlk.alphabeta(scorediff, 11);
	}
	else {
		m_wlk.alphabeta(heval, m_depth);
	}
}
