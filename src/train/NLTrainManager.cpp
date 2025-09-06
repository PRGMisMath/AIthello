#include "NLTrainManager.hpp"

#include "../utils/Random.hpp"
#include<cstddef>
#include<string>

using namespace std::literals;

// Implementation des methodes statiques
void NLTrainManager::repr_mat(std::ostream& os, float* val, size_t n, size_t p, size_t precision) {
    os << std::fixed << std::setprecision((std::streamsize)std::min((size_t)4, precision - 2));
    std::string del(precision, '-');
    for (int i = 0; i < (int)p; ++i) { os << "+" << del; } os << "+\n";
    for (size_t i = 0; i < n; ++i) {
        for (size_t j = 0; j < p; ++j) {
            os << '|';
            os << std::setw(precision) << val[i * p + j];
        }
        os << "|\n";
        for (int i = 0; i < (int)p; ++i) { os << "+" << del; } os << "+\n";
    }
    os << std::endl << std::setprecision(10);
}

void NLTrainManager::repr_nn(std::ostream& os, NLNetwork* nn) {
    for (size_t l = 0; l < nn->num_layers; ++l) {
        NLFoncComb* w_comb = &(nn->layers[l].combin);
        os << "\n\n[[[Layer " << l << "]]] {\n\n";
        os << "[Matrix] {\n";
        repr_mat(os, w_comb->matrix, w_comb->n, w_comb->p);
        os << "\n}\n\n[Bias] {\n";
        repr_mat(os, w_comb->bias, w_comb->n, 1);
        os << "\n}\n}\n\n";
    }
}

void NLTrainManager::repr(NLTrainAI* tr_ai) {
    std::ofstream rfile{ save_folder + "repr_nn.json" };
    repr_nn(rfile, tr_ai->netw);
    rfile.close();
    std::ofstream rdfile{ save_folder + "repr_der_nn.json" };
    repr_nn(rdfile, &(tr_ai->calc.der_netw));
    rdfile.close();
    std::ofstream rcfile{ save_folder + "repr_calc.json" };
    rcfile << "\n\n[[[Entry (0)]]] {\n\n";
    repr_mat(rcfile, tr_ai->calc.calc_mem[0], tr_ai->netw->layers[0].combin.p, 1, 4);
    rcfile << "\n}\n\n";
    for (size_t d = 0; d < tr_ai->netw->num_layers; ++d) {
        rcfile << "\n\n[[[Couche " << d << "]]] {\n\n";
        rcfile << "[Sum] {\n";
        repr_mat(rcfile, tr_ai->calc.calc_mem[d * 2 + 1], tr_ai->netw->layers[d].combin.n, 1);
        rcfile << "\n}\n\n[Activ] {\n";
        repr_mat(rcfile, tr_ai->calc.calc_mem[d * 2 + 2], tr_ai->netw->layers[d].combin.n, 1);
        rcfile << "\n}\n\n";
    }
    rcfile.close();
}

void NLTrainManager::shuffle_train_data(NLTrainData* tds, size_t lenData, size_t maxStep) {
    size_t max_shuffle = (maxStep < lenData) ? maxStep : lenData;
    for (size_t iShuffle = 0; iShuffle < max_shuffle; ++iShuffle) {
        size_t rShuffle = (rd::src.uniform_int(0, lenData - iShuffle - 1)) + iShuffle;
        float* swap;
        swap = tds[rShuffle].entry;
        tds[rShuffle].entry = tds[iShuffle].entry;
        tds[iShuffle].entry = swap;
        swap = tds[rShuffle].result;
        tds[rShuffle].result = tds[iShuffle].result;
        tds[iShuffle].result = swap;
    }
}

NLTrainManager::AnalysisResult NLTrainManager::analyzeResults(const TestResult& testRes, double success_threshold, int bin_size) {
    AnalysisResult result{};

    const std::vector<TestResult::Element>& data = testRes.tests;

    if (data.empty()) return result;

    double sum_abs_error = 0.0;
    double sum_sq_error = 0.0;
    int success_count = 0;
    int fail_count = 0;

    double sum_ai = 0.0, sum_target = 0.0;
    double sum_ai_sq = 0.0, sum_target_sq = 0.0, sum_ai_target = 0.0;
    double sum_error = 0.0;

    double player_error_sum[2]{ 0,0 };
    int player_count[2]{ 0,0 };

    for (const auto& e : data) {
        double err = e.ai_ev - e.target_ev;
        double abs_err = std::fabs(err);

        sum_abs_error += abs_err;
        sum_sq_error += err * err;
        sum_error = err;

        if (abs_err <= success_threshold) {
            success_count++;
        }
        else {
            fail_count++;
            result.fails_by_player[*e.player]++;
            int bin = (e.turn / bin_size) * bin_size;
            result.fails_by_turn_bin[bin]++;
            int binf = (((int)(e.target_ev * 99.99f)) / bin_size) * bin_size;
            result.fails_by_target_bin[binf]++;
        }

        sum_ai += e.ai_ev;
        sum_target += e.target_ev;
        sum_ai_sq += e.ai_ev * e.ai_ev;
        sum_target_sq += e.target_ev * e.target_ev;
        sum_ai_target += e.ai_ev * e.target_ev;

        player_error_sum[*e.player] += abs_err;
        player_count[*e.player]++;
    }

    int n = data.size();

    result.mae = sum_abs_error / n;
    result.rmse = std::sqrt(sum_sq_error / n);
    result.merr = sum_error / n;
    result.success_rate = static_cast<double>(success_count) / n;
    result.total_success = success_count;
    result.total_fail = fail_count;

    // Corr�lation de Pearson
    double numerator = (n * sum_ai_target - sum_ai * sum_target);
    double denominator = std::sqrt((n * sum_ai_sq - sum_ai * sum_ai) *
        (n * sum_target_sq - sum_target * sum_target));
    if (denominator != 0.0)
        result.correlation = numerator / denominator;
    else
        result.correlation = 0.0;

    // Erreur moyenne par joueur
    result.mae_by_player[0] = player_error_sum[0] / player_count[0];
    result.mae_by_player[1] = player_error_sum[1] / player_count[1];

    return result;
}

void printHistogram(std::ostream& os, const std::unordered_map<int, int>& hist, const std::string& label, int bin_size) {
    os << "\n" << label << ":\n";
    if (hist.empty()) {
        os << "  Aucun echec enregistre.\n";
        return;
    }
    int max_count = 0;
    for (auto& kv : hist) {
        max_count = std::max(max_count, kv.second);
    }
    for (auto& kv : hist) {
        int bin = kv.first;
        int count = kv.second;
        int bar_len = static_cast<int>(50.0 * count / max_count);
        os << "  Tour " << std::setw(3) << bin << "-" << std::setw(3) << (bin + bin_size - 1)
            << " : " << std::setw(3) << count << " "
            << std::string(bar_len, '#') << "\n";
    }
}




// Constructeur/Destructeur
NLTrainManager::NLTrainManager(NLFormat* format, size_t sformat) :
    format(format), sformat(sformat), entrysize(format->p)
{
    createNetwork();
}

NLTrainManager::~NLTrainManager() {
    destroyResources();
}

void NLTrainManager::createNetwork() {
    create_rand_network(&nn, format, sformat);
    createTrainAI(&tai, &nn);
    createTrainAI(&eai, &nn);

    p_needFreeAI = true;
}

bool NLTrainManager::loadData(const std::string& data_path, size_t lenfds, size_t lentds) {
    lfds = lenfds;
    ltds = lentds;
    lvds = lenfds - lentds;

    std::ifstream dread{ data_path, std::ios_base::binary };
    if (!dread.is_open()) {
        dread.close();
        return false;
    }
    v1ds.set = (NLTrainData*)malloc(sizeof(NLTrainData) * lfds);
    v1ds.sset = lfds;
    v1ds._block = (float*)malloc(sizeof(float) * (nldatasize + 1) * lfds);
    dread.read((char*)v1ds._block, sizeof(float) * (nldatasize + 1) * lfds);
    NLTrainData* datas = v1ds.set;
    for (int count = 0; count < lfds; ++count) {
        float* _curr_block = v1ds._block + count * (nldatasize + 1);
        datas[count].result = _curr_block + nldatasize;
        datas[count].entry = _curr_block;
    }
    dread.close();
    shuffle_train_data(v1ds.set, lfds);
    fds = v1ds.set;
    tds = fds;
    vds = fds + ltds;

    p_needFreeData = true;

    return true;
}

void NLTrainManager::destroyResources() {
    if (p_needFreeData) {
        free(v1ds.set);
        free(v1ds._block);
    }
    if (p_needFreeAI) {
        destroyTrainAI(&tai);
        destroyTrainAI(&eai);
        destroy_network(&nn);
    }

    p_needFreeAI = false;
    p_needFreeData = false;
}




// SaveNNCommand

SaveNNCommand::SaveNNCommand(NLTrainManager& manager) : manager(manager), qname("qname", "nom de la sauvegarde (par d�faut un nombre)") {}

const char* SaveNNCommand::name() const { return "save"; }

const char* SaveNNCommand::description() const { return "Sauvegarde le reseau de neurone."; }

void SaveNNCommand::execute(TerminalState& state) {
    std::ostream& os = *(state.fluxOut);
    std::string aname = qname.value;
    if (!qname) {
        aname = "def"s + std::to_string(count++);
    }
    os << "\nSauvegarde... ";
    std::ofstream fnn{ manager.save_folder + aname + "nn.nn", std::ios_base::binary };
    if (!fnn.is_open()) {
        os << "(echec)" << std::endl;
        fnn.close();
        return;
    }
    nn_serialize(fnn, &(manager.nn));
    fnn.close();
    os << "(finie)" << std::endl;
}

void SaveNNCommand::configure(CmdFormat& format) {
    qname.giveDefValue("");
    format.addReq(&qname);
}



// LoadNNCommand

LoadNNCommand::LoadNNCommand(NLTrainManager& manager) : manager(manager), qname("qname", "nom du reseau a charger") {}

const char* LoadNNCommand::name() const { return "load"; }

const char* LoadNNCommand::description() const { return "Charge le reseau de nom [qname]."; }

void LoadNNCommand::execute(TerminalState& state) {
    std::ostream& os = *(state.fluxOut);
    std::ifstream fnn{ manager.save_folder + qname.value + "nn.nn", std::ios_base::binary };
    if (!fnn.is_open()) {
        os << "Erreur : Fichier non trouve !\n";
        fnn.close();
        return;
    }
    destroy_network(&(manager.nn));
    nn_reader(fnn, &(manager.nn));
    fnn.close();
    os << "NN charge en memoire !\n";
}

void LoadNNCommand::configure(CmdFormat& format) {
    format.addReq(&qname);
}




// InfoNNCommand

InfoNNCommand::InfoNNCommand(NLTrainManager& manager) : manager(manager), plot("plot", "drapeau pour afficher la courbe des couts"), full("full", "drapeau pour des informations exhaustives") {}

const char* InfoNNCommand::name() const { return "info"; }

const char* InfoNNCommand::description() const { return "Affiche des infos."; }

void InfoNNCommand::execute(TerminalState& state) {
    *(state.fluxOut) << "\n=== Training Info ==="
        << "\nGenerations totales : " << manager.infos.totGen
        << "\n\tMini-generations : " << manager.infos.miniGen
        << "\n=====================\n";
    if (plot) {
        std::ofstream cost_file{ manager.save_folder + "cost.csv"s };
        cost_file.imbue(std::locale("C"));
        cost_file << std::fixed << std::setprecision(4);
        for (size_t i = 0; i < manager.infos.vcouts.size(); ++i) {
            cost_file << std::get<0>(manager.infos.vcouts[i]);
            if (i != manager.infos.vcouts.size() - 1) cost_file << ",";
        }
        cost_file << std::endl;
        for (size_t i = 0; i < manager.infos.vcouts.size(); ++i) {
            cost_file << std::get<1>(manager.infos.vcouts[i]);
            if (i != manager.infos.vcouts.size() - 1) cost_file << ",";
        }
        cost_file.close();
        system(("start py \""s + manager.save_folder + "cplot.py\""s).c_str());
    }
    if (full) {
        manager.repr(&(manager.tai));
    }
}

void InfoNNCommand::configure(CmdFormat& format) {
    format.addOpt(&plot).addOpt(&full);
}




// TestNNCommand

TestNNCommand::TestNNCommand(NLTrainManager& manager) : manager(manager) {}

const char* TestNNCommand::name() const { return "test"; }

const char* TestNNCommand::description() const { return "Teste le reseau."; }

void execute_test(TerminalState& state, NLTrainManager& manager, bool verbose = true) {
    std::ostream& os = *(state.fluxOut);
    NLTrainManager::TestResult testRes{};
    float cost = 0.f;
    for (size_t d = 0; d < manager.lvds; ++d) {
        eval_network_mem(manager.eai.netw, manager.vds[d].entry, manager.eai.calc.calc_mem + 1);
        int turn = -4;
        for (int i = 0; i < nldatasize - 1; ++i) {
            turn += manager.vds[d].entry[i];
        }
        testRes.tests.push_back({
            turn,
            (manager.vds[d].entry[nldatasize - 1] ? PlayerID::Noir : PlayerID::Blanc),
            manager.vds[d].result[0],
            manager.eai.calc.calc_mem[2 * manager.eai.netw->num_layers][0]
            });
    }

    auto res = NLTrainManager::analyzeResults(testRes, 0.1, 5);
    manager.infos.vcouts.push_back({ manager.infos.totGen, res.rmse });

    if (verbose) {
        os << "\n===== Resume de l'analyse =====\n\n";
        os << "MAE            : " << res.mae << "\n";
        os << "RMSE           : " << res.rmse << "\n";
        os << "Erreur moyenne : " << res.merr << "\n";
        os << "Correlation    : " << res.correlation << "\n";
        os << "Succes         : " << res.total_success << "\n";
        os << "Echecs         : " << res.total_fail << "\n";
        os << "Taux de succes : " << res.success_rate * 100 << "%\n\n";
        os << "Erreur moyenne par joueur :\n";
        os << "  Joueur Noir " << res.mae_by_player[0] << "\n";
        os << "  Joueur Blanc " << res.mae_by_player[1] << "\n";
        os << "\nEchecs par joueur :\n";
        os << "  Joueur Noir " << res.fails_by_player[0] << "\n";
        os << "  Joueur Blanc " << res.fails_by_player[1] << "\n";
        printHistogram(os, res.fails_by_turn_bin, "Repartition des echecs par tranche de tours", 5);
        printHistogram(os, res.fails_by_target_bin, "Repartition des echecs par rapport a l'objectif (en %)", 5);
    }
}

void TestNNCommand::execute(TerminalState& state) {
    execute_test(state, manager);
}

void TestNNCommand::configure(CmdFormat& format) {}




// BackPropNNCommand

BackPropNNCommand::BackPropNNCommand(NLTrainManager& manager) : manager(manager),
test("t", "drapeau activant les tests"),
verbose("v", "drapeau controlant les logs"),
count("count", "nombre de tours"),
miniBatchSize("minibatchsize", "active le batching") {
}

const char* BackPropNNCommand::name() const { return "train"; }

const char* BackPropNNCommand::description() const { return "Entraine le reseau."; }

void BackPropNNCommand::execute(TerminalState& state) {
    for (int it = 0; it < count.value; ++it) {
        std::ostream& os = *(state.fluxOut);
        if (verbose)
            os << "Backpropagation N*" << manager.infos.totGen << " : \n";
        float cost;
        if (miniBatchSize.value != -1) {
            NLTrainManager::shuffle_train_data(manager.tds, manager.ltds);
            for (int k = 0; k + miniBatchSize.value <= manager.ltds; k += miniBatchSize.value) {
                cost = evalCostAndDeriv(&(manager.tai), manager.tds + k, miniBatchSize.value);
                manager.infos.couts.push_back(cost);
                applyDeriv(&(manager.tai));
            }
            ++manager.infos.miniGen;
        }
        else {
            cost = evalCostAndDeriv(&(manager.tai), manager.tds, manager.ltds);
            applyDeriv(&(manager.tai));
        }
        manager.infos.totGen++;
        if (test) {
            execute_test(state, manager, verbose);
        }
    }
}

void BackPropNNCommand::configure(CmdFormat& format) {
    miniBatchSize.giveDefValue(-1);
    format.addReq(&count).addReq(&miniBatchSize).addOpt(&verbose).addOpt(&test);
}