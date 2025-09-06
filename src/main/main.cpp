#include<iostream>
#include<thread>
#include<cstdint>

#include "../utils/Benchmark.hpp"
#include "../game/Othello.hpp"
#include "../game/GamePlayer.hpp"
#include "../game/GameBoardManager.hpp"
#include "../tree/MinMaxTree.hpp"
#include "../tree/PreferTree.hpp"
#include "../nl/NLNetwork.h"
#include "../train/OthelloTrain.hpp"
#include "../train/NLTrainManager.hpp"
#include "../utils/StringView.hpp"


constexpr size_t entrysize = 129;
constexpr size_t sformat = 3;
NLFormat format[sformat] = {
    { BOARD_SIZE * BOARD_COUNT, entrysize, Tanh },
    { 32, BOARD_SIZE * BOARD_COUNT, Tanh },
    { 1, 32, Tanh }
};

NLTrainManager train_manager{ format, sformat };

float aiheur(const MiniOthello& game) {
    auto nn = train_manager.nn;
    boardToData(game, nn.p_process1);
    float res;
    eval_network(&nn, nn.p_process1, &res);
    return 2 * res - 1;
}




class EvolvedMinMaxAI : public MinMaxAI {
public:
    EvolvedMinMaxAI(size_t depth = 3, EvalHeur heval = scorediff) :
        MinMaxAI(depth, heval)
    {
        name = "EvolvedMinMaxAI" + std::to_string(depth);
    }
    ~EvolvedMinMaxAI() = default;

    void reset() override
    {
        MinMaxAI::reset();
    }

protected:
    void treeEvoluate() override
    {
        if (m_wlk.getRoot()->savestate.getTurn() <= 5) {
            m_wlk.alphabeta(noheur, 1);
        }
        else if (m_wlk.getRoot()->savestate.getTurn() >= 49) {
            m_wlk.alphabeta(scorediff, 12);
        }
        else {
            MinMaxAI::treeEvoluate();
        }
    }

};
class EvolvedPreferAI : public PreferAI {
public:
    EvolvedPreferAI(size_t length = 50, float temp = 100, EvalHeur heval = scorediff) :
        PreferAI(length, temp, heval)
    {
        name = "EvolvedPreferAI" + std::to_string(length);
    }
    ~EvolvedPreferAI() = default;

    void reset() override
    {
        PreferAI::reset();
    }

protected:
    void treeEvoluate() override
    {
        if (m_wlk.getRoot()->savestate.getTurn() <= 5) {
            m_wlk.preferSearch(noheur, 10);
        }
        else if (m_wlk.getRoot()->savestate.getTurn() >= 49) {
            m_wlk.getRoot()->alphabeta(scorediff, 12);
        }
        else {
            PreferAI::treeEvoluate();
        }
    }

};


int main() {
    WTHFileReader reader = WTHFileReader(R"(data\games\WTH_FUSION.wtb)");

    // AI //
    train_manager.loadData(R"(data\gen\backlearn\dataS100000.bin)", 100'000, 90'000);
    train_manager.save_folder = R"(data\save\)";
    

    Terminal term{};
    term.addCommand(new SaveNNCommand(train_manager));
    term.addCommand(new LoadNNCommand(train_manager));
    term.addCommand(new InfoNNCommand(train_manager));
    term.addCommand(new TestNNCommand(train_manager));
    term.addCommand(new BackPropNNCommand(train_manager));
    StringView sv{};
    term.addCommand(new PlayCMD(sv, reader));

    term.run();


    train_manager.destroyResources();
    
    return 0;
}