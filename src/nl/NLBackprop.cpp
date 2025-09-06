#include "NLBackprop.h"

#include<stdlib.h>
#include<string.h>
#include<math.h>
#include "NLDebug.h"


void createTrainAI(struct NLTrainAI* tai, struct NLNetwork* netw)
{
	tai->netw = netw;

	tai->param.learnVol = NL_LEARN_VOL;

	// Creation d'un reseau de neurone dedie au stockage des d�rivees partielles obtenues a partir de l'ancien reseau
	struct NLNetwork* der_netw = &(tai->calc.der_netw);
	der_netw->max_dim = 0; // Indique : faux reseau la uniquement comme amas de donnee
	der_netw->num_layers = netw->num_layers;
	der_netw->layers = (struct NLLayer*)malloc(netw->num_layers * sizeof(struct NLLayer));
	alloc_test(der_netw->layers, "Bad Alloc (CTAI-I)");
	for (size_t s = 0; s < netw->num_layers; ++s) {
		if (_nl_comb_init(&(der_netw->layers[s].combin), netw->layers[s].combin.n, netw->layers[s].combin.p) == -1)
			alloc_test(NULL, "Bad alloc (CTAI-I)");
		memset(der_netw->layers[s].combin.bias, 0,
			netw->layers[s].combin.n * sizeof(float));
		memset(der_netw->layers[s].combin.matrix, 0,
			netw->layers[s].combin.n * netw->layers[s].combin.p * sizeof(float));
	}

	// Creation d'array imbrique pour conserver toutes les resultats intermediaires de l'evaluation
	tai->calc.calc_mem = (float**)malloc((2 * netw->num_layers + 1) * sizeof(float*)); // <- decalage pour mettre l'entree
	float** calc_mem = tai->calc.calc_mem;
	alloc_test(calc_mem, "Initial alloc failed (CTAI-I)");
	for (size_t s = 0; s < netw->num_layers; ++s) {
		calc_mem[2 * s + 1] = (float*)malloc(netw->layers[s].combin.n * sizeof(float));
		calc_mem[2 * s + 2] = (float*)malloc(netw->layers[s].combin.n * sizeof(float));
		alloc_test(calc_mem[2 * s + 1], "Initial alloc failed (CTAI-I)");
		alloc_test(calc_mem[2 * s + 2], "Initial alloc failed (CTAI-I)");
	}

	// Pour la recopie afin d'eviter les effets de bord
	tai->calc.process = (float*)malloc(netw->max_dim * sizeof(float));
	alloc_test(tai->calc.process, "Initial alloc failed (CTAI-I)");
	// Pour transmettre les erreurs vers l'arriere (e_k^(u))
	tai->calc.error = (float*)malloc(netw->max_dim * sizeof(float));
	alloc_test(tai->calc.error, "Initial alloc failed (CTAI-I)");
}

void destroyTrainAI(struct NLTrainAI* tai, bool destroyNetw)
{
	destroy_network(&(tai->calc.der_netw));
	for (size_t s = 1; s < 2 * tai->netw->num_layers + 1; ++s) {
		free(tai->calc.calc_mem[s]);
	}
	free(tai->calc.calc_mem);
	free(tai->calc.process);
	free(tai->calc.error);

	if (destroyNetw)
		destroy_network(tai->netw);
}

float squareCost(const float* theoric, const float* result, NLFoncActiv der, size_t length, float* error)
{
	float cost = 0.f;
	for (size_t k = 0; k < length; ++k) {
		float sq_diff = (result[k] - theoric[k]);

		error[k] = sq_diff * der(theoric[k]); // _ * 2 ?

		sq_diff *= sq_diff;
		cost += sq_diff;
	}
	return cost;
}

float logCost(const float* theoric, const float* result, NLFoncActiv der, size_t length, float* error)
{
	float cost = 0.f;
	for (size_t k = 0; k < length; ++k) {
		float th = theoric[k],
			exp = result[k];

		float ln_diff = -th * logf(exp) - (1 - th) * logf(1 - exp);
		if (isnan(ln_diff))
			ln_diff = 0.f;
		cost += ln_diff;

		error[k] = (exp - th);
	}
	return cost;
}


void evalNetwork(struct NLTrainAI* tai, struct NLTrainData data)
{
	float** calc_mem = tai->calc.calc_mem;

	eval_network_mem(tai->netw, data.entry, calc_mem + 1); // <- tient compte du decalage
	calc_mem[0] = data.entry;
}

float computeCost(struct NLTrainAI* tai, struct NLTrainData data, CostFunction calcCost)
{
	size_t L = tai->netw->num_layers - 1;
	struct NLLayer* last_layer = tai->netw->layers + L;
	float** calc_mem = tai->calc.calc_mem;
	float* error = tai->calc.error;

	return calcCost(data.result, calc_mem[2 * L + 2], last_layer->der_activ, last_layer->combin.n, error);
}

void propagateError(struct NLTrainAI* tai)
{
	struct NLNetwork* netw = tai->netw;
	size_t L = netw->num_layers - 1;
	struct NLNetwork* der_netw = &(tai->calc.der_netw);
	float** calc_mem = tai->calc.calc_mem;
	float* process = tai->calc.process;
	float* error = tai->calc.error;

	for (size_t rl = L; 1; --rl) { // /!\ C'est un entier non signe
		struct NLLayer* w_layer = netw->layers + rl;
		size_t n = w_layer->combin.n, p = w_layer->combin.p;

		// Derivee par rapport aux w et b
		for (size_t i = 0; i < n; ++i)
			der_netw->layers[rl].combin.bias[i] -= error[i];
		for (size_t i = 0; i < n; ++i) {
			float* mat_layer = der_netw->layers[rl].combin.matrix + i * p;
			for (size_t j = 0; j < p; ++j)
				mat_layer[j] += error[i] * calc_mem[2 * rl][j]; // <- correspond a couche precedente
		}

		// Derivee par rapport aux z_k
		if (rl == 0) break; // <- le dernier calcul est inutile
		for (size_t j = 0; j < p; ++j) {
			process[j] = 0;
			for (size_t i = 0; i < n; ++i) {
				process[j] += w_layer->combin.matrix[i * p + j] * error[i];
			}
			process[j] *= w_layer->der_activ(calc_mem[2 * rl - 1][j]); // <- correspond a couche precedente
		}
		for (size_t k = 0; k < p; ++k)
			error[k] = process[k];
	}
}




float evalCostAndDeriv(NLTrainAI* tai, NLTrainData* data, size_t lenData, CostFunction calcCost)
{
	// Cout moyen
	float moy_cout = 0.f;

	struct NLNetwork* netw = tai->netw;
	size_t L = netw->num_layers - 1;
	struct NLNetwork* der_netw = &(tai->calc.der_netw);
	float** calc_mem = tai->calc.calc_mem;
	float* process = tai->calc.process;
	float* error = tai->calc.error;

	// Calcul de toutes les derivees partielles pour la fonction de cout moyenne
	for (size_t d = 0; d < lenData; ++d) {
		/// Passe avant
		evalNetwork(tai, data[d]);

		/// Calcul du cout et de la derivee
		moy_cout += computeCost(tai, data[d], calcCost);

		/// Passe arri�re
		propagateError(tai);
	}

	tai->calc.ratio = lenData;
	tai->calc.moy_cost = moy_cout / lenData;

	return tai->calc.moy_cost;
}

void applyDeriv(NLTrainAI* tai)
{
	struct NLNetwork* netw = tai->netw;
	struct NLNetwork* der_netw = &(tai->calc.der_netw);
	size_t ratio = tai->calc.ratio;
	float learnVol = tai->param.learnVol;

	// Mise a jour du reseau en suivant la pente ( - learnVol * [der_part] )
	for (size_t s = 0; s < netw->num_layers; ++s) {
		struct NLLayer* w_layer = netw->layers + s;
		struct NLLayer* w_der_layer = der_netw->layers + s;
		size_t n = w_layer->combin.n, p = w_layer->combin.p;
		for (size_t l = 0; l < n; ++l) {
			w_layer->combin.bias[l] += -learnVol * w_der_layer->combin.bias[l] / ratio;
			float* mat_layer = w_layer->combin.matrix + l * p;
			float* mat_der_layer = w_der_layer->combin.matrix + l * p;
			for (size_t k = 0; k < p; ++k)
				mat_layer[k] += -learnVol * mat_der_layer[k] / ratio;
		}
	}
	
	reinitDeriv(tai);
}

void reinitDeriv(NLTrainAI* tai, float factor)
{
	struct NLNetwork* netw = tai->netw;
	struct NLNetwork* der_netw = &(tai->calc.der_netw);

	// Reinitialise les derivees a 0
	for (size_t s = 0; s < netw->num_layers; ++s) {
		struct NLLayer* w_der_layer = der_netw->layers + s;
		size_t n = w_der_layer->combin.n, p = w_der_layer->combin.p;
		for (size_t l = 0; l < n; ++l) {
			w_der_layer->combin.bias[l] *= factor;
			float* mat_der_layer = w_der_layer->combin.matrix + l * p;
			for (size_t k = 0; k < p; ++k)
				mat_der_layer[k] *= factor;
		}
	}
}

float backpropagation(struct NLTrainAI* tai, struct NLTrainData* data, size_t lenData)
{
	evalCostAndDeriv(tai, data, lenData);

	applyDeriv(tai);

	return tai->calc.moy_cost;
}
