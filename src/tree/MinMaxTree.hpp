#ifndef MIN_MAX_TREE_HPP
#define MIN_MAX_TREE_HPP


#include "../game/GamePlayer.hpp"

#include "GameTree.hpp"



class MinMaxWalker : public GameWalker {
public:
	MinMaxWalker(const GameTree& tree);
	~MinMaxWalker() = default;

	float minimax(EvalHeur heval, size_t rec_depth);
	float alphabeta(EvalHeur heval, size_t rec_depth);

};

class MinMaxAI : public TreeAI {
public:
	MinMaxAI(size_t depth = 3, EvalHeur heval = scorediff);
	~MinMaxAI() = default;

	void reset() override;

protected:
	void treeEvoluate() override;

protected:
	MinMaxWalker m_wlk;
	size_t depth;

};


#endif // !MIN_MAX_TREE_HPP
