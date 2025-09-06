#include "MinMaxTree.hpp"

#include<cassert>





// MinMaxWalker //


MinMaxWalker::MinMaxWalker(const GameTree& tree) :
	GameWalker(tree)
{
}

float MinMaxWalker::minimax(EvalHeur heval, size_t recDepth)
{
	return curr->minimax(heval, recDepth);
}

float MinMaxWalker::alphabeta(EvalHeur heval, size_t recDepth)
{
	return curr->alphabeta(heval, recDepth);
}




// MinMaxAI //

MinMaxAI::MinMaxAI(size_t depth, EvalHeur heval) :
	TreeAI(&m_wlk, heval), depth(depth), m_wlk(m_tree)
{
	this->name = "minmax" + std::to_string(depth);
}

void MinMaxAI::reset()
{
	m_tree.reset();
	m_wlk = MinMaxWalker(m_tree);
}

void MinMaxAI::treeEvoluate()
{
	m_wlk.alphabeta(heval, depth);
}

