#ifndef NL_BACKPROP_HPP
#define NL_BACKPROP_HPP

#include "NLNetwork.h"

#define NL_LEARN_VOL 0.5f


struct NLTrainData {
	float* entry;
	float* result;
};

struct _NLCalcul {
	// Pour stocker les d�rivees partielles obtenues
	struct NLNetwork der_netw;
	size_t ratio; // = lenData
	// Pour conserver toutes les resultats intermediaires de l'evaluation
	float** calc_mem;
	// Pour la recopie afin d'eviter les effets de bord
	float* process;
	// Pour transmettre les erreurs vers l'arriere (e_k^(u))
	float* error;
	// Pour stocker le dernier cout moyen calcule
	float moy_cost;
};

struct _NLParam {
	float learnVol;
};

struct NLTrainAI {
	struct NLNetwork* netw;
	struct _NLCalcul calc;
	struct _NLParam param;
};

void createTrainAI(struct NLTrainAI* tai, struct NLNetwork* netw);
void destroyTrainAI(struct NLTrainAI* tai, bool destroyNetw = false);

typedef float (*CostFunction)(const float* theoric, const float* result, NLFoncActiv der, size_t length, float* error);
float squareCost(const float* theoric, const float* result, NLFoncActiv der, size_t length, float* error);
float logCost(const float* theoric, const float* result, NLFoncActiv der, size_t length, float* error);

void evalNetwork(struct NLTrainAI* tai, struct NLTrainData data);
float computeCost(struct NLTrainAI* tai, struct NLTrainData data, CostFunction calcCost);
void propagateError(struct NLTrainAI* tai);

float evalCostAndDeriv(struct NLTrainAI* tai, struct NLTrainData* data, size_t lenData, CostFunction calcCost = squareCost);
void applyDeriv(struct NLTrainAI* tai);
void reinitDeriv(NLTrainAI* tai, float factor = 0.f);
float backpropagation(struct NLTrainAI* tai, struct NLTrainData* data, size_t lenData);

#endif // !NL_BACKPROP_HPP
