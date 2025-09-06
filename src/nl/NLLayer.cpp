#include "NLLayer.h"

#include "../utils/Random.hpp"
#include "NLDebug.h"
#include<stdlib.h>
#include<string.h>
#include<math.h>





//////////////////////////////////////
// --- Fonctions de combinaison --- //
//////////////////////////////////////


int _nl_comb_init(struct NLFoncComb* combin, size_t n, size_t p)
{
	combin->n = n;
	combin->p = p;
	combin->matrix = (float*)malloc(n * p * sizeof(float));
	if (combin->matrix == NULL)
		return -1;
	combin->bias = (float*)malloc(n * sizeof(float));
	if (combin->bias == NULL)
		return -1;
	
	return 0;
}

// Cree une copie ==> Allocation de l'espace associe
int _nl_comb_copy(struct NLFoncComb* dest, struct NLFoncComb* src)
{
	dest->n = src->n;
	dest->p = src->p;
	dest->matrix = (float*)malloc(src->n * src->p * sizeof(float));
	if (dest->matrix == NULL)
		return -1;
	memcpy(dest->matrix, src->matrix, src->n * src->p * sizeof(float));
	dest->bias = (float*)malloc(src->n * sizeof(float));
	if (dest->bias == NULL)
		return -1;
	memcpy(dest->bias, src->bias, src->n * sizeof(float));
	return 0;
}

void _nl_comb_rand(struct NLFoncComb* combin)
{
	for (int l = 0; l < combin->n; ++l)
		_nl_comb_rand_lig(combin, l);
}

void _nl_comb_rand_lig(struct NLFoncComb* combin, size_t lig)
{
	// On devrait multiplie par '~= p' pour que le biais puisse prendre toutes les valeurs possibles
	combin->bias[lig] = rd::src.normal(0, 1); 

	auto norm_distr = rd::src.normal_distributor(0, 1);

	float* mat_layer = combin->matrix + lig * combin->p;
	for (size_t c = 0; c < combin->p; ++c) {
		mat_layer[c] = norm_distr.next();
	}
}

void _nl_comb_muta_lig(struct NLFoncComb* combin, size_t lig, float mut_factor)
{
	auto mut_distr = rd::src.bernoulli_distributor(mut_factor);

	if (mut_distr.next()) {
		combin->bias[lig] = rd::src.normal(0, combin->p);
	}

	auto norm_distr = rd::src.normal_distributor(0, 1);

	float* mat_layer = combin->matrix + (lig * combin->p);
	for (size_t c = 0; c < combin->p; ++c) {
		if (mut_distr.next()) {
			mat_layer[c] = norm_distr.next();
		}
	}
}

void _nl_comb_mix(struct NLFoncComb* dest, struct NLFoncComb* src, float regen_proba, float mut_proba, float mut_factor)
{
#ifdef NL_DEBUG
	if (dest->p != src->p || dest->n != src->n)
		debug_break(BadSizeArgument, "Impossible mixing !");
#endif // NL_DEBUG

	auto mut_distr = rd::src.bernoulli_distributor(mut_proba);
	auto regen_distr = rd::src.bernoulli_distributor(regen_proba);
	auto bread_distr = rd::src.bernoulli_distributor(.5f);

	for (size_t l = 0; l < src->n; ++l) {
		if (regen_distr.next())
			_nl_comb_rand_lig(dest, l);
		else {
			if (bread_distr.next()) {
				memcpy(dest->matrix + l * dest->p, src->matrix + l * src->p, dest->p * sizeof(float));
				dest->bias[l] = src->bias[l];
			}
			if (mut_distr.next())
				_nl_comb_muta_lig(dest, l, mut_factor);
		}
	}
}

void _nl_comb_destroy(struct NLFoncComb* combin)
{
	free(combin->matrix);
	free(combin->bias);
}



/////////////////////////////////////
// --- Fonctions d'activations --- //
/////////////////////////////////////

float nl_sigmoid(float x)
{
	return 1 / (1 + expf(-x));
}
float nl_der_sigmoid(float x)
{
	float sig = nl_sigmoid(x);
	return sig * (1 - sig);
}


float nl_RELU(float x) 
{
	return ((x < 0) ? 0 : x);
}
float nl_der_RELU(float x)
{
	return ((x < 0) ? 0 : 1);
}

float nl_tanh(float x) {
	return tanhf(x);
}

float nl_der_tanh(float x) {
	float t = tanhf(x);
	return 1.0f - t * t;
}

float nl_id(float x)
{
	return x;
}
float nl_der_id(float x)
{
	return 1.f;
}
