#ifndef GAME_TREE_NODE_HPP
#define GAME_TREE_NODE_HPP

#include "../game/Othello.hpp"

#include<vector>
#include<functional>


typedef std::function<float(const MiniOthello&)> EvalHeur;


float noheur(const MiniOthello&); 
float scorediff(const MiniOthello& game);
float sigscorediff(const MiniOthello& game);
float lnscorediff(const MiniOthello& game); // deprecated
float nbposheur(const MiniOthello& game);

//template <EvalHeur heval> // Maybe later ??
class TreeNode {
public:

	struct StateEval {
		float rec = NAN;
		float minMax = NAN;
	};

	struct Rope {
		StateEval eval;

		Pos pos;
		TreeNode* child;
	};

public:
	TreeNode() = default;
	~TreeNode();

	float minimax(EvalHeur heval, size_t recDepth);
	float alphabeta(EvalHeur heval, size_t recDepth, float alpha = -INFINITY, float beta = INFINITY);

	bool isLeaf() const; // enfant non genere ?
	bool isEnd() const;  // n'a pas de suite ? (non gen / extremite)

	float evalHeur();
	float evalHeur(EvalHeur heval);

	bool generatePlays();


public:
	std::vector<Rope> ropes;
	MiniOthello savestate;
	bool isOpon;


private:
	bool p_genChild = false;
	float heur = NAN;


};


#endif // !GAME_TREE_NODE_HPP
