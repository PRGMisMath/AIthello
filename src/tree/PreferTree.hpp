#ifndef PREFER_TREE_HPP
#define PREFER_TREE_HPP

#include "../game/GamePlayer.hpp"
#include "GameTree.hpp"

class PreferWalker : public GameWalker {
public:
	PreferWalker(const GameTree& tree);

	void preferSearch(EvalHeur heval, size_t max_search, float temp = 1.f);
};

class PreferAI : public TreeAI {
public:
	PreferAI(size_t length = 50, float temp = 100.f, EvalHeur heval = scorediff);
	~PreferAI() = default;

	void reset() override;

protected:
	void treeEvoluate() override;

protected:
	PreferWalker m_wlk;
	size_t length;
	float temp;
};

#endif // !PREFER_TREE_HPP
