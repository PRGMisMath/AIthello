#ifndef NL_TRAIN_MANAGER_HPP
#define NL_TRAIN_MANAGER_HPP


#include<string>
#include<vector>
#include<tuple>
#include<unordered_map>
#include<ostream>
#include<fstream>
#include<iomanip>
#include<cstdio>
#include<cstdlib>
#include<cmath>
#include<algorithm>
#include<locale>

#include "../utils/Terminal.hpp"
#include "../game/GameBoard.hpp"
#include "OthelloTrain.hpp"



class NLTrainManager {
public:
    NLTrainManager(NLFormat* format, size_t sformat);
    ~NLTrainManager();

    bool loadData(const std::string& data_path, size_t lfds, size_t ltds);
    void createNetwork();
    void destroyResources();


    // Structures internes
    struct TrainingInfo {
        size_t totGen = 0;
        size_t miniGen = 0;
        std::vector<float> couts;
        std::vector<std::tuple<int, float>> vcouts;
    };

    struct TestResult {
        struct Element {
            int turn;
            PlayerID player;
            float target_ev;
            float ai_ev;
        };
        int id;
        int gen_nb;
        NLNetwork* netw;
        NLTrainData* data;
        size_t len_data;
        float mean_cost;
        std::vector<Element> tests;
    };

    struct AnalysisResult {
        double mae; // Moyenne de la valeur absolue des erreurs
        double rmse; // Deviation moyenne par rapport a l'objectif
        double merr; // Moyenne des erreurs
        double correlation;
        double success_rate;
        int total_success = 0;
        int total_fail = 0;
        int fails_by_player[2];
        double mae_by_player[2];
        std::unordered_map<int, int> fails_by_turn_bin;
        std::unordered_map<int, int> fails_by_target_bin;
    };

    // Methodes utilitaires
    static void repr_mat(std::ostream& os, float* val, size_t n, size_t p, size_t precision = 9);
    static void repr_nn(std::ostream& os, NLNetwork* nn);
    void repr(NLTrainAI* tr_ai);
    static void shuffle_train_data(NLTrainData* tds, size_t lenData, size_t maxStep = UINT64_MAX);
    static AnalysisResult analyzeResults(const TestResult& testRes, double success_threshold = 0.1, int bin_size = 10);

    // Variables publiques
    size_t entrysize;
    size_t sformat;
    NLFormat* format;
    std::string save_folder = R"(??)";
    size_t lfds;
    size_t ltds;
    size_t lvds;

    NLNetwork nn;
    NLTrainAI tai;
    NLTrainAI eai;
    NLTrainData* fds;
    NLTrainData* tds;
    NLTrainData* vds;
    V1DataSet v1ds;
    TrainingInfo infos;

private:
    bool p_needFreeData = false;
    bool p_needFreeAI = false;

};


class SaveNNCommand : public Command {
public:
    explicit SaveNNCommand(NLTrainManager& manager);
    const char* name() const override;
    const char* description() const override;
    void execute(TerminalState& state) override;

protected:
    void configure(CmdFormat& format) override;

private:
    NLTrainManager& manager;
    StringParam qname;
    int count = 0;
};

class LoadNNCommand : public Command {
public:
    explicit LoadNNCommand(NLTrainManager& manager);
    const char* name() const override;
    const char* description() const override;
    void execute(TerminalState& state) override;

protected:
    void configure(CmdFormat& format) override;

private:
    NLTrainManager& manager;
    StringParam qname;
};

class InfoNNCommand : public Command {
public:
    explicit InfoNNCommand(NLTrainManager& manager);
    const char* name() const override;
    const char* description() const override;
    void execute(TerminalState& state) override;

protected:
    void configure(CmdFormat& format) override;

private:
    NLTrainManager& manager;
    BoolParam plot;
    BoolParam full;
};

class TestNNCommand : public Command {
public:
    explicit TestNNCommand(NLTrainManager& manager);
    const char* name() const override;
    const char* description() const override;
    void execute(TerminalState& state) override;

protected:
    void configure(CmdFormat& format) override;

private:
    NLTrainManager& manager;
};

class BackPropNNCommand : public Command {
public:
    explicit BackPropNNCommand(NLTrainManager& manager);
    const char* name() const override;
    const char* description() const override;
    void execute(TerminalState& state) override;

protected:
    void configure(CmdFormat& format) override;

private:
    NLTrainManager& manager;
    BoolParam test;
    BoolParam verbose;
    IntParam count;
    IntParam miniBatchSize;
};



#endif //!NL_TRAIN_MANAGER_HPP