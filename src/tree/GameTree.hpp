#ifndef GAME_TREE_HPP
#define GAME_TREE_HPP

#include "GameTreeNode.hpp"
#include "../game/GamePlayer.hpp"

class GameTree {
public:
	GameTree();
	GameTree(const GameTree&) = delete;
	GameTree& operator=(const GameTree&) = delete;
	~GameTree();

	void reset();

private:
	TreeNode* top;

	friend class GameWalker;
};


class GameWalker {
public:
	GameWalker() = delete;
	GameWalker(const GameTree& tree);
	GameWalker(const GameWalker&) = default;
	GameWalker& operator=(const GameWalker&) = default;
	~GameWalker() = default;

	TreeNode* getRoot();

	Pos getMaxPlay() const;
	Pos getProbaPlay(float rd_src, float temp = 1.f) const;
	Pos getProbaPlay(float (*predicat)(float), float rd_src) const;

	void down(Pos pos);

	bool valid() const;


protected:
	TreeNode* curr;


};

class TreeAI : public AIPlayer {
public:
	TreeAI() = delete;
	~TreeAI() = default;

	void updateBoardFromPlay(Pos pos) override;
	Pos calcNextMove() override;

	GameTree& getTree();

protected:
	TreeAI(GameWalker* p_wlk, EvalHeur heval);

	virtual void treeEvoluate() = 0;

protected:
	GameTree m_tree;
	GameWalker* p_wlk;

	EvalHeur heval;
};

#endif // !GAME_TREE_HPP
