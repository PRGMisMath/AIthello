#include "GameTreeNode.hpp"

#include<cassert>
#include<algorithm>

float noheur(const MiniOthello&) { return 0.f; }
float scorediff(const MiniOthello& game) {
	return game.getScore(game.lastPlayer()) - game.getScore(!game.lastPlayer());
}
float sigscorediff(const MiniOthello& game)
{
	return 2.f / (1.f + expf(-scorediff(game))) - 1.f;
}
float lnscorediff(const MiniOthello& game)
{
	float diffln = std::log(game.getScore(game.lastPlayer())) - std::log(game.getScore(!game.lastPlayer()));
	return 2.f / (1.f + std::expf(-diffln)) - 1.f;
}

float nbposheur(const MiniOthello& game) {
	return game.countMoves(game.lastPlayer()) - game.countMoves(!game.lastPlayer());
}

bool TreeNode::generatePlays()
{
	if (!isLeaf())
		return !isEnd();

	std::vector<MiniOthello> boards = savestate.nextBoards();
	for (const MiniOthello& board : boards) {
		Rope rope = Rope();

		rope.child = new TreeNode();
		rope.child->savestate = board;
		rope.pos = board.lastPlay();

		ropes.push_back(rope);
	}

	p_genChild = true;

	return !isEnd();
}

bool TreeNode::isLeaf() const
{
	return !p_genChild;
}

bool TreeNode::isEnd() const
{
	return ropes.size() == 0;
}

float TreeNode::evalHeur()
{
	assert(!isnan(s_heur));
	return s_heur;
}

float TreeNode::evalHeur(EvalHeur heval)
{
	if (isnan(s_heur))
		s_heur = heval(savestate);
	return s_heur;
}

float TreeNode::minimax(EvalHeur heval, size_t recDepth, bool noGen)
{
	assert(heval != nullptr);

	if (
		recDepth == 0
		// ++ Cas d'arret critique
		|| (!isLeaf() && isEnd())
		|| (isEnd() && noGen) // Dans ce cas, on ne genere pas
		)
	{
		return evalHeur(heval);
	}

	if (isLeaf() && !generatePlays()) // Cas ou on doit generer
	{
		return evalHeur(heval);
	}

	// Si le joueur a avoir joue le coup est different de celui qui a joue le precedent
	isOpon = (savestate.lastPlayer() == !ropes[0].child->savestate.lastPlayer());

	float max_eval = -INFINITY;
	for (Rope& rope : ropes) {

		float eval = rope.child->minimax(heval, recDepth - 1, noGen);

		if (eval > max_eval) {
			max_eval = eval;
		}

		rope.eval.minMax = eval;
	}

	return (isOpon) ? (-max_eval) : max_eval;
}

float TreeNode::alphabeta(EvalHeur heval, size_t recDepth, bool noGen, float alpha, float beta)
{
	assert(heval != nullptr);

	if (
		recDepth == 0
		// ++ Cas d'arret critique
		|| (!isLeaf() && isEnd())
		|| (isEnd() && noGen) // Dans ce cas, on ne genere pas
		)
	{
		return evalHeur(heval);
	}

	if (s_depth == recDepth) // On a en memoire ce noeud
	{
		if (s_lowerbound >= beta)
			return s_lowerbound;
		if (s_upperbound <= alpha)
			return s_upperbound;
		alpha = std::max(alpha, s_lowerbound);
		beta = std::min(beta, s_upperbound);
	}
	else { // New depth ==> reset
		s_depth = recDepth;
		s_lowerbound = -INFINITY;
		s_upperbound = +INFINITY;
	}

	if (isLeaf()) { // Cas ou on doit generer
		if (!generatePlays())
			return evalHeur(heval);
	}
	else // Cas ou les donnees existaient deja
		std::sort(std::begin(ropes), std::end(ropes), [](const Rope& rope1, const Rope& rope2) -> float { return rope1.eval.minMax > rope2.eval.minMax; });

	// Si le joueur a avoir joue le coup est different de celui qui a joue le precedent
	// C'est le meme pour tous
	isOpon = (savestate.lastPlayer() == !ropes[0].child->savestate.lastPlayer());

	float max_eval = -INFINITY;
	for (Rope& rope : ropes) {
		if (alpha > beta) {
			rope.eval.minMax = -INFINITY; // indique un cut
			if (isOpon) {
				// alpha cut ==> upperbound
				s_upperbound = beta;
			}
			else {
				// beta cut ==> lowerbound
				s_lowerbound = alpha;
			}
			break;
		}

		float eval;
		if (isOpon) {
			eval = rope.child->alphabeta(heval, recDepth - 1, noGen, -beta, -alpha);
			beta = std::min(beta, -eval);
		}
		else {
			eval = rope.child->alphabeta(heval, recDepth - 1, noGen, alpha, beta);
			alpha = std::max(alpha, eval);
		}

		if (eval > max_eval)
			max_eval = eval;

		rope.eval.minMax = eval;
	}

	// if (alpha < beta) { s_lowerbound = alpha; s_upperbound = beta; } // useless for mtdf

	return (isOpon) ? (-max_eval) : max_eval;
}

float TreeNode::mtdf(EvalHeur heval, size_t recDepth, float startHeur, bool noGen, float precision) {
	assert(heval != nullptr);

	float lowerbound = -INFINITY, upperbound = +INFINITY;

	do {
		float wndw = startHeur;
		if (startHeur == lowerbound) wndw += precision;
		startHeur = alphabeta(heval, recDepth, noGen, wndw - precision, wndw);
		if (startHeur <= wndw - precision) {
			upperbound = startHeur;
		}
		else if (startHeur >= wndw) {
			lowerbound = wndw;
		}
		else {
			return startHeur;
		}
	} while (lowerbound < upperbound);
	return startHeur;
}

TreeNode::~TreeNode()
{
	for (const Rope& rope : ropes) {
		delete rope.child;
	}
}

