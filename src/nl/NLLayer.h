#ifndef NL_LAYER_HPP
#define NL_LAYER_HPP

#include<cstddef>

typedef float(*NLFoncActiv)(float);


// p : taille en entree
// n : taille en sortie
//
// matrix[i,j] = matrix[i*p + j]
// 
// Forme de la matrice :
//         p
//     <------->
//    ^
//    |
//  n | 			*  ( matrice colonne : taille p,1 )  -  ( biais : taille n,1 )
//    |
//    v
// 
// pour evaluer, les lignes [i*p : (i+1)*p[ donnent apres convolution un coefficient (i)
struct NLFoncComb {
	float* matrix;
	float* bias;
	size_t n, p; // n: sortie | p: entree
};


int _nl_comb_init(struct NLFoncComb* combin, size_t n, size_t p);
int _nl_comb_copy(struct NLFoncComb* dest, struct NLFoncComb* src);
void _nl_comb_rand(struct NLFoncComb* combin);
void _nl_comb_rand_lig(struct NLFoncComb* combin, size_t h_layer);
void _nl_comb_muta_lig(struct NLFoncComb* combin, size_t h_layer, float mut_factor);
void _nl_comb_mix(struct NLFoncComb* dest, struct NLFoncComb* src, float regen_proba, float mut_proba, float mut_factor);
void _nl_comb_destroy(struct NLFoncComb* combin);


struct NLLayer {
	NLFoncActiv activ;
	NLFoncActiv der_activ;
	struct NLFoncComb combin;
};

struct NLArgFoncActiv {
	NLFoncActiv activ;
	NLFoncActiv der_activ;
};


float nl_sigmoid(float x);
float nl_der_sigmoid(float x);
float nl_RELU(float x);
float nl_der_RELU(float x);
float nl_tanh(float x);
float nl_der_tanh(float x);
float nl_id(float x);
float nl_der_id(float x);

const struct NLArgFoncActiv Sigmoid = { nl_sigmoid,nl_der_sigmoid };
const struct NLArgFoncActiv RELU = { nl_RELU,nl_der_RELU };
const struct NLArgFoncActiv Tanh = { nl_tanh, nl_der_tanh };
const struct NLArgFoncActiv Id = { nl_id,nl_der_id };

#endif