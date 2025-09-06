#include "NLNetwork.h"

#include "NLDebug.h"
#include<stdlib.h>
#include<string.h>
#include<stdio.h>
#include<fstream>


size_t max_s(size_t a, size_t b) {
	return (a < b) ? b : a;
}





void create_rand_network(struct NLNetwork* network, struct NLFormat* format, size_t l_format)
{
	network->max_dim = 0;
	network->num_layers = l_format;
	network->layers = (struct NLLayer*)malloc(l_format * sizeof(struct NLLayer));
	alloc_test(network->layers, "Bad Alloc (CRN1)");
	for (size_t s = 0; s < l_format; ++s) {
		network->max_dim = max_s(network->max_dim, format[s].n);
		network->layers[s].activ = format[s].arg_act.activ;
		network->layers[s].der_activ = format[s].arg_act.der_activ;
		if (_nl_comb_init(&(network->layers[s].combin), format[s].n, format[s].p) == -1)
			alloc_test(NULL, "Bad alloc (CRN2)");
		_nl_comb_rand(&(network->layers[s].combin));
	}
	// Il faut que max_dim soit calcule avant de pouvoir allouer
	network->p_process1 = (float*)malloc(network->max_dim * sizeof(float));
	network->p_process2 = (float*)malloc(network->max_dim * sizeof(float));
	alloc_test(network->p_process1, "Bad Alloc (CRN1)");
	alloc_test(network->p_process2, "Bad Alloc (CRN1)");
}

// Cree une copie ==> Allocation de l'espace associe
void create_copy_network(struct NLNetwork* dest, struct NLNetwork* src)
{
	dest->max_dim = src->max_dim;
	dest->num_layers = src->num_layers;
	dest->layers = (struct NLLayer*)malloc(src->num_layers * sizeof(struct NLLayer));
	dest->p_process1 = (float*)malloc(src->max_dim * sizeof(float));
	dest->p_process2 = (float*)malloc(src->max_dim * sizeof(float));
	alloc_test(dest->layers, "Bad Alloc (CCN)");
	alloc_test(dest->p_process1, "Bad Alloc (CCN)");
	alloc_test(dest->p_process2, "Bad Alloc (CCN)");
	for (size_t s = 0; s < src->num_layers; ++s) {
		dest->layers[s].activ = src->layers[s].activ;
		dest->layers[s].der_activ = src->layers[s].der_activ;
		_nl_comb_copy(&(dest->layers[s].combin), &(src->layers[s].combin));
	}
}

void destroy_network(struct NLNetwork* network)
{
	for (size_t s = 0; s < network->num_layers; ++s) {
		_nl_comb_destroy(&(network->layers[s].combin));
	}
	free(network->layers);
	if (network->max_dim != 0) { // Cas ou le reseau est utilise comme simple amas de donnee
		free(network->p_process1);
		free(network->p_process2);
	}
}

// Attention : la fonction est souvent utilisee avec input = output
void eval_network(struct NLNetwork* network, const float* input, float* output)
{
	float sum;
	const float* entry = input;
	for (size_t s = 0; s < network->num_layers; ++s) { // Sur tous les niveaux
		struct NLLayer* lay = network->layers + s;
		for (size_t i = 0; i < lay->combin.n; ++i) { // A tout ordonnee
			// Evaluation de la matrice
			float* mat_layer = lay->combin.matrix + (i * lay->combin.p);
			sum = 0;
			for (size_t j = 0; j < lay->combin.p; ++j) // A tout abscisse
				sum += mat_layer[j] * entry[j];

			// Application du biais
			sum -= lay->combin.bias[i]; 

			// Application de la fonction d'activation
			network->p_process1[i] = lay->activ(sum);
		}

		// On fait une copie pour eviter les effets de bord en ayant `entry == p_process1`
		for (size_t i = 0; i < lay->combin.n; ++i)
			network->p_process2[i] = network->p_process1[i];

		entry = network->p_process2;
	}
	// On ne peut pas utiliser `output` avant car rien n'assure qu'il soit de bonne taille
	for (size_t i = 0; i < network->layers[network->num_layers - 1].combin.n; ++i)
		output[i] = network->p_process2[i];
}

void eval_network_mem(struct NLNetwork* network, const float* input, float** out_layer)
{
	float sum;
	const float* entry = input;
	for (size_t s = 0; s < network->num_layers; ++s) { // Sur tous les niveaux
		struct NLLayer* lay = network->layers + s;
		for (size_t i = 0; i < lay->combin.n; ++i) { // A tout ordonnee
			// Evaluation de la matrice
			float* mat_layer = lay->combin.matrix + (i * lay->combin.p);
			sum = 0;
			for (size_t j = 0; j < lay->combin.p; ++j) // A tout abscisse
				sum += mat_layer[j] * entry[j];

			// Application du biais
			sum -= lay->combin.bias[i];

			// Application de la fonction d'activation
			out_layer[2*s][i] = sum;
			out_layer[2*s + 1][i] = lay->activ(sum);
		}

		entry = out_layer[2*s + 1];
	}
}

void mix_network(struct NLNetwork* dest, struct NLNetwork* src, float regen_proba, float mut_proba, float mut_factor)
{
	for (size_t s = 0; s < src->num_layers; ++s)
		_nl_comb_mix(&(dest->layers[s].combin), &(src->layers[s].combin), regen_proba, mut_proba, mut_factor);
}


void matrix_reader(std::ifstream& stream, float* matrix, size_t n, size_t p) {
	stream.read((char*)matrix, sizeof(float) * n * p);
}

void nn_reader(std::ifstream& stream, struct NLNetwork* res) {
	stream.read((char*) &res->num_layers, sizeof(size_t));
	res->layers = (NLLayer*)malloc(res->num_layers * sizeof(NLLayer));


	if (res->layers == NULL) {
		stream.setstate(std::ios_base::badbit);
		return;
	}

	size_t ncolL, pcolL;
	size_t countIt = 0;

	NLLayer* wLayer;

	stream.read((char*)&pcolL, sizeof(size_t));
	res->max_dim = pcolL;
	for (
		stream.read((char*)&ncolL, sizeof(size_t));
		ncolL != 0;
		stream.read((char*)&ncolL, sizeof(size_t))
		)
	{
		if (res->max_dim < ncolL)
			res->max_dim = ncolL;

		wLayer = res->layers + countIt;
		wLayer->activ = nl_sigmoid;
		wLayer->der_activ = nl_der_sigmoid;
		if (_nl_comb_init(&(wLayer->combin), ncolL, pcolL)) {
			stream.setstate(std::ios_base::badbit);
			return;
		}

		matrix_reader(stream, wLayer->combin.matrix, ncolL, pcolL);
		matrix_reader(stream, wLayer->combin.bias, ncolL, 1);

		++countIt;
		pcolL = ncolL;
	}

	res->p_process1 = (float*)malloc(res->max_dim * sizeof(float));
	res->p_process2 = (float*)malloc(res->max_dim * sizeof(float));

	if (res->p_process1 == NULL || res->p_process2 == NULL) {
		stream.setstate(std::ios_base::badbit);
		return;
	}
}

void matrix_writer(std::ofstream& stream, float* matrix, size_t n, size_t p) {
	stream.write((char*)matrix, sizeof(float) * n * p);
}

void nn_serialize(std::ofstream& stream, struct NLNetwork* nn) {
	stream.write((char*)&(nn->num_layers), sizeof(size_t));

	NLLayer* w_layer;
	size_t n, p;

	stream.write((char*)&(nn->layers[0].combin.p), sizeof(size_t));

	for (size_t k = 0; k < nn->num_layers; ++k) {
		w_layer = nn->layers + k;
		n = w_layer->combin.n;
		p = w_layer->combin.p;

		stream.write((char*)&n, sizeof(size_t));
		matrix_writer(stream, w_layer->combin.matrix, n, p);
		matrix_writer(stream, w_layer->combin.bias, n, 1);
	}

	float zero = 0;
	stream.write((char*)&zero, sizeof(float));
}