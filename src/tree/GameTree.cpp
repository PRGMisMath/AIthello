#include "GameTree.hpp"

#include "../utils/Random.hpp"
#include<cassert>


// GameTree //

GameTree::GameTree() :
	top(new TreeNode())
{
}

GameTree::~GameTree()
{
	delete top;
}

void GameTree::reset() {
	delete top;
	top = new TreeNode();
}



// GameWalker //

GameWalker::GameWalker(const GameTree& tree) :
	curr(tree.top)
{
}


TreeNode * GameWalker::getRoot() {
	return curr;
}

Pos GameWalker::getMaxPlay() const
{
	assert(valid());

	float max_eval = -INFINITY;
	Pos res;
	for (const TreeNode::Rope& rope : curr->ropes) {
		if (max_eval < rope.eval.minMax) {
			max_eval = rope.eval.minMax;
			res = rope.pos;
		}
	}
	return res;
}

Pos GameWalker::getProbaPlay(float rd_src, float temp) const
{
	assert(valid());

	float sum_eval = 0.f;
	for (const TreeNode::Rope& rope : curr->ropes) {
		sum_eval += exp(rope.eval.minMax / temp);
	}
	float lim_rd = rd_src * sum_eval;
	sum_eval = 0.f;
	for (const TreeNode::Rope& rope : curr->ropes) {
		sum_eval += exp(rope.eval.minMax / temp);
		if (sum_eval >= lim_rd)
			return rope.pos;
	}
	return INVALID;
}

Pos GameWalker::getProbaPlay(float(*predicat)(float), float rd_src) const
{
	assert(valid());

	float sum_eval = 0.f;
	for (const TreeNode::Rope& rope : curr->ropes) {
		sum_eval += predicat(rope.eval.minMax);
	}
	float lim_rd = rd_src * sum_eval;
	sum_eval = 0.f;
	for (const TreeNode::Rope& rope : curr->ropes) {
		sum_eval += predicat(rope.eval.minMax);
		if (sum_eval >= lim_rd)
			return rope.pos;
	}
	return INVALID;
}

void GameWalker::down(Pos pos)
{
	assert(valid());

	if (curr->isLeaf())
		curr->generatePlays();

	TreeNode* target = nullptr;
	for (TreeNode::Rope& rope : curr->ropes) {
		if (rope.pos == pos) {
			target = rope.child;
			break;
		}
	}

	curr = target;
}

bool GameWalker::valid() const
{
	return curr != nullptr;
}



// TreeAI //

TreeAI::TreeAI(GameWalker* p_wlk, EvalHeur heval) :
	p_wlk(p_wlk), m_tree(), heval(heval)
{
}

void TreeAI::updateBoardFromPlay(Pos pos)
{
	p_wlk->down(pos);
}

Pos TreeAI::calcNextMove()
{
	this->treeEvoluate();
	return p_wlk->getProbaPlay(rd::src.uniform_real(0,1), 0.2f);
}

GameTree& TreeAI::getTree()
{
	return m_tree;
}
