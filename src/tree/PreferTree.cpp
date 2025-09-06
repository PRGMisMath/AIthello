#include "PreferTree.hpp"

#include<queue>
#include<memory>

PreferWalker::PreferWalker(const GameTree& tree) :
	GameWalker(tree)
{
}



struct PreferParam {
	float priority;
	TreeNode* node;


	bool operator<(const PreferParam& pp) const { return priority < pp.priority; }
};

float shrimp(float x) {
	return 1.f / (1.f + expf(-x / 32.f));
}


float minimaxSearched(TreeNode* node) {
	if (node->isEnd())
		return node->evalHeur();

	float max_eval = -INFINITY;
	for (TreeNode::Rope& rope : node->ropes) {
		float eval = minimaxSearched(rope.child);

		if (eval > max_eval) {
			max_eval = eval;
		}

		rope.eval.minMax = eval;
	}

	return (node->isOpon) ? (-max_eval) : max_eval;
}

void PreferWalker::preferSearch(EvalHeur heval, size_t max_search, float temp)
{
	size_t count = 0;

	std::priority_queue<PreferParam> prefer_queue{};
	prefer_queue.push({ 1.f, curr });

	do {
		const PreferParam& params = prefer_queue.top();
		TreeNode* node = params.node;
		float nodePrio = params.priority;

		if (node->isLeaf()) { // Cas ou on doit generer
			node->generatePlays(); 
		}
		if (node->isEnd()) {
			prefer_queue.pop();
			continue;
		}

		// Si le joueur a avoir joue le coup est different de celui qui a joue le precedent
		node->isOpon = (node->savestate.lastPlayer() == !node->ropes[0].child->savestate.lastPlayer());

		std::vector<PreferParam> poses{};
		poses.reserve(node->ropes.size());
		float sum_exp = 0.f;
		for (TreeNode::Rope& rope : node->ropes) {
			if (isnan(rope.eval.minMax))
				rope.eval.minMax = rope.child->evalHeur(heval);
			float eval = rope.eval.minMax;
			eval = expf(eval / temp);

			sum_exp += eval;

			poses.push_back({
				eval,
				rope.child
			});
		}
		prefer_queue.pop();

		for (const PreferParam& param : poses) {
			prefer_queue.push({
				nodePrio * (param.priority / sum_exp),
				param.node
			});
		}
	} while (!prefer_queue.empty() && (count++) < max_search);

	minimaxSearched(curr);
}


// PreferAI //

PreferAI::PreferAI(size_t length, float temp, EvalHeur heval) :
	TreeAI(&m_wlk, heval), length(length), temp(temp), m_wlk(m_tree)
{
	name = "prefer" + std::to_string(length);
}

void PreferAI::reset()
{
	m_tree.reset();
	m_wlk = PreferWalker(m_tree);
}

void PreferAI::treeEvoluate()
{
	m_wlk.preferSearch(heval, length, temp);
}

