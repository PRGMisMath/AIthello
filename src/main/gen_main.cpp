#include<cassert>
#include<thread>

#include "../utils/Random.hpp"
#include "../train/OthelloTrain.hpp"
#include "../tree/MinMaxTree.hpp"


///
/// Quelques considerations d'optimisation
/// 
/// 1. Les 1ers coups ne seront pas traites par l'IA (< 5)
/// pour eviter une surrepresentation et aussi car ils ne sont en pratique
/// jamais utilise. De plus, pour de bonnes performances,
/// on utilisera une table de prevision.
/// 
/// 2. Les derniers coups ne seront pas non plus traites (< 3)
/// le nombre de coup a regarder etant tres faible (< 3 * 2 * 1 = 6)
/// ce sera plus rapide que d'evaluer le reseau. De plus,
/// cela ameliore l'end game.
///



/// ///////////////////////////////////////////// ///
/// Version 1 - Augmentation par imitation minmax ///
/// 
/// --- Idee ---
/// Entrainer un reseau a retourner l'evaluation issue d'un minmax.
/// Obtenir davantage de profondeur de recherche pour peu cher.
/// 
/// --- Amelioration ---
/// Commencer par entrainer sur le bas puis remonter
/// en utilisant le reseau deja entraine.
/// 
/// ///////////////////////////////////////////// ///



V1DataSet V1generateRandTrainData(size_t sizeData, size_t depthCheck)
{
	float* _block = (float*)malloc(sizeof(float) * (nldatasize + 1) * sizeData);
	assert(_block != nullptr);
	NLTrainData* datas = (NLTrainData*)malloc(sizeof(NLTrainData) * sizeData);
	assert(datas != nullptr);

	size_t count = 0;
	Othello logic{};
	GameTree tree{};
	MinMaxWalker wlk{ tree };
	PlayerID lastPlayer = PlayerID::Blanc; // Player evaluating the game in the walker

	auto add_chance = rd::src.bernoulli_distributor(0.3f);
	auto skip_chance = rd::src.bernoulli_distributor(0.8f);
	auto playChooser = rd::src.uniform_real_distributor(0,1);

	while (count < sizeData) {
		if (logic.isFinish()) {
			logic.reset();
			tree.reset();
			wlk = MinMaxWalker(tree);
		}

		if (logic.getTurn() < 60 - depthCheck) {
			lastPlayer = logic.currentPlayer();
			wlk.alphabeta(nbposheur, 6);
			Pos next = wlk.getProbaPlay(playChooser.next(), 0.5f);
			bool valid = logic.play(next); assert(valid);
			wlk.down(next);
			continue;
		}

		float score = wlk.alphabeta(scorediff, depthCheck);
		score = score / 64.f;


		if (add_chance.next()) {
			for (int id_sym = 0; id_sym<8; ++id_sym) {
				for (int inverse = 0; inverse<2; ++inverse) {
					if (skip_chance.next()) {
						continue;
					}
					float* _curr_block = _block + count * (nldatasize + 1);

					datas[count].result = _curr_block + nldatasize;
					_curr_block[nldatasize - 1] = (inverse) ? (lastPlayer == PlayerID::Blanc) : (lastPlayer == PlayerID::Noir);
					_curr_block[nldatasize] = score;
					datas[count].entry = _curr_block;
					boardToData(logic, _curr_block, symetries[id_sym], inverse);

					++count;
					std::cout << count << ':' << logic.getTurn() << std::endl;

					if (count >= sizeData) {
						goto end_loop;
					}
				}
			}
		}

		lastPlayer = logic.currentPlayer();
		Pos next = wlk.getProbaPlay(playChooser.next(), 0.9f);
		bool valid = logic.play(next); assert(valid);
		wlk.down(next);
	}

	end_loop:
	V1DataSet ret;
	ret.set = datas;
	ret.sset = sizeData;
	ret.strain = nldatasize;
	ret._block = _block;

	return ret;
}


/// :::::::::::::::::::::::::::::::: ///
/// Version 2 - Coupe assiste par IA ///
///
/// --- Idee ---
/// On injecte 2 plateaux voisins puis on donne un score de validite
/// de coupe pour ce coup.
/// Factoriser les calculs en separant le reseau en un analyseur
/// et en un comparateur.
///
/// --- Perspective ---
/// Ajouter des contraintes de diversifiaction et d'interpretabilite
/// pour l'analyseur.
/// Entrainer les 2 en //e pour un meilleur resultat.
///
/// :::::::::::::::::::::::::::::::: ///


/// :::::::::::::::::::::::::::::::::::::::::::: ///
/// Version 3 - Apprentissage des grands maitres ///
/// --- Idee ---
/// :::::::::::::::::::::::::::::::::::::::::::: ///

V1DataSet V3generateMasterData(const WTHFileReader& wth, size_t sizeData)
{
	float* _block = (float*)malloc(sizeof(float) * (nldatasize + 1) * sizeData);
	assert(_block != nullptr);
	NLTrainData* datas = (NLTrainData*)malloc(sizeof(NLTrainData) * sizeData);
	assert(datas != nullptr);

	Othello logic{};

	size_t lenwth = wth.getGames().size();
	size_t count = 0;

	while (count < sizeData) {
		size_t rd_index = rd::src.uniform_int(0, lenwth - 1);

		const auto& listOfMoves = wth.getGames()[rd_index].gameSave.listOfMoves;

		size_t rd_play = rd::src.uniform_int(0, (listOfMoves.size() - 12)) + 4;

		logic.reset();
		size_t i = 0;
		for (; i < rd_play; ++i) {
			logic.play(listOfMoves[i]);
		}

		for (Pos pos : logic.getPlayableSpots()) {
			Othello f_game = logic;
			f_game.play(pos);

			float* _curr_block = _block + count * (nldatasize + 1);

			datas[count].result = _curr_block + nldatasize;
			_curr_block[nldatasize - 1] = (logic.currentPlayer() == PlayerID::Noir);
			_curr_block[nldatasize] = (pos == listOfMoves[i]) ? 1 : 0;
			datas[count].entry = _curr_block;
			boardToData(f_game, _curr_block);

			++count;
		}
	}

	V1DataSet ret;
	ret.set = datas;
	ret.sset = sizeData;
	ret.strain = nldatasize;
	ret._block = _block;

	return ret;
}


int main() {
	constexpr size_t sizeData = 100000;
	constexpr int nbThreads = 25;
	constexpr size_t sizeDataPerThread = sizeData / nbThreads;

	V1DataSet l_set[nbThreads];
	std::thread l_thread[nbThreads]{};

	for (size_t i = 0; i < nbThreads; ++i) {
		V1DataSet* p_set = &l_set[i];
		l_thread[i] = std::thread([p_set] { *p_set = V1generateRandTrainData(sizeDataPerThread, 12); });
	}
	std::ofstream out{ R"(../data/gen/backlearn/dataS100000.bin)", std::ios_base::binary };
	for (size_t i = 0; i < nbThreads; ++i) {
		V1DataSet* p_set = &l_set[i];
		l_thread[i].join();
		out.write((char*)p_set->_block, p_set->sset * p_set->strain * sizeof(float));
		free(p_set->_block);
		free(p_set->set);
	}
	out.close();


	// for (int count = 0; count < sizeData; ++count) {
	// 	NLTrainData* curr_data = set.set + count;
	// 	Othello view{ curr_data->entry };
	// 	std::cout << "Board number: " << count << std::endl;
	// 	std::cout << view.view << std::endl;
	// 	std::cout << "Eval : " << *curr_data->result << std::endl;
	// 	std::cout << "Player : " << ((curr_data->entry[nldatasize - 1] > 0) ? "Noir" : "Blanc") << "\n" << std::endl;
	// }


}