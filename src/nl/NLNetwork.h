#ifndef NL_NETWORK_HPP
#define NL_NETWORK_HPP

#include "NLLayer.h"

#include<stdio.h>
#include<fstream>


// Etant donnee la notation matricielle,
// p correspond � la taille en ENTREE
// et n a celle en SORTIE.
// ==> Penser matriciellement !
struct NLFormat {
	size_t n, p;
	struct NLArgFoncActiv arg_act;
};

struct NLNetwork {
	struct NLLayer* layers;
	size_t num_layers;
	size_t max_dim;
	float* p_process1; // pour l'evaluation
	float* p_process2; // pour l'evaluation
};

void create_rand_network(struct NLNetwork* network, struct NLFormat* format, size_t l_format);
void create_copy_network(struct NLNetwork* dest, struct NLNetwork* src);
void destroy_network(struct NLNetwork* network);
void eval_network(struct NLNetwork* network, const float* input, float* output);
void eval_network_mem(struct NLNetwork* network, const float* input, float** out_layer);
void mix_network(struct NLNetwork* dest, struct NLNetwork* src, float regen_proba, float mut_proba, float mut_factor);
void nn_serialize(std::ofstream& stream, struct NLNetwork* nn);
void nn_reader(std::ifstream& stream, struct NLNetwork* res);

struct TrainAI {
	struct NLNetwork network;
	int comp_score;
	int ref_score;
};


#endif // !NL_NETWORK_HPP
