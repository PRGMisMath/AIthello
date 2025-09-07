#include<iostream>
#include<thread>
#include<cstdint>

#include "Random.hpp"
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

class MDTFWalker : public GameWalker {

};



int main() {
    int depth = 6;

    Othello logic{};
    GameTree tree0;
    MinMaxWalker min_max_walker{tree0};
    GameTree tree1;
    MinMaxWalker alphabeta_walker{tree1};
    GameTree tree2;
    MinMaxWalker mtdf_walker{tree2};
    long long mM_time = 0, ab_time = 0, mtdf_time = 0;
    std::chrono::time_point<std::chrono::high_resolution_clock> start;
    std::chrono::time_point<std::chrono::high_resolution_clock> end;

    while (!logic.isFinish()) {
        // MinMax
        // start = std::chrono::high_resolution_clock::now();
        // float mMEval = min_max_walker.minimax(nbposheur, depth);
        // end = std::chrono::high_resolution_clock::now();
        // mM_time += std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        // Pos mMPlay = min_max_walker.getMaxPlay();

        // AlphaBeta
        start = std::chrono::high_resolution_clock::now();
        float abEval = alphabeta_walker.alphabeta(nbposheur, depth);
        end = std::chrono::high_resolution_clock::now();
        ab_time += std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        Pos abPlay = alphabeta_walker.getProbaPlay(rd::src.uniform_real(0,1));

        // MTDF
        start = std::chrono::high_resolution_clock::now();
        float mtdfEval = mtdf_walker.mtdf(nbposheur, depth);
        end = std::chrono::high_resolution_clock::now();
        mtdf_time += std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        Pos mtdfPlay = mtdf_walker.getMaxPlay();

        std::cout << "\nMinMax : " << abEval << "\nAlphaBeta : " << abEval << "\nMTDF : " << mtdfEval << std::endl;
        if (abEval != abEval || abEval != mtdfEval) {
            std::cout << "\tErreur !!!!" << std::endl;
            return 1;
        }

        min_max_walker.down(abPlay);
        alphabeta_walker.down(abPlay);
        mtdf_walker.down(abPlay);
        logic.play(abPlay);
    }

    std::cout << "\nMinMax : " << mM_time << "ms\nAlphaBeta : " << ab_time << "ms\nMTDF : " << mtdf_time << "ms" << std::endl;

    return 0;

    start = std::chrono::high_resolution_clock::now();
    Benchmark benchmark{
        [] { return (TreeAI*) new EvolvedMinMaxAI(6, nbposheur); },
        [] { return (TreeAI*) new EvolvedPreferAI(10000, 2, nbposheur); }
    };
    benchmark.runBenchmark(std::cout, 10, nullptr, 0, 10);
    end = std::chrono::high_resolution_clock::now();
    long long duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << std::endl << "\nDuration : " << duration << "ms" << std::endl;
    return 0;

    WTHFileReader reader = WTHFileReader(R"(../datagames/WTH_FUSION.wtb)");

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