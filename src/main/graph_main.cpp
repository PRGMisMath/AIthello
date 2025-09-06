#include<iostream>
#include<thread>
#include<cstdint>

#include "../game/Othello.hpp"
#include "GameView.hpp"
#include "GamePlayer.hpp"
#include "../game/GameBoardManager.hpp"
#include "MinMaxTree.hpp"
#include "../tree/PreferTree.hpp"
#include "../utils/Random.hpp"
#include "../nl/NLNetwork.h"
#include "../nl/NLBackprop.h"
#include "OthelloTrain.h"
#include "../graphics/OthelloUI.hpp"
#include "../train/NLTrainManager.hpp"
#include "../utils/StringView.hpp"


constexpr size_t entrysize = 129;
constexpr size_t sformat = 3;
NLFormat format[sformat] = {
    { BOARD_SIZE * BOARD_COUNT, entrysize, Sigmoid },
    { 32, BOARD_SIZE * BOARD_COUNT, Sigmoid },
    { 1, 32, Sigmoid }
};

NLTrainManager train_manager{ format, sformat };

float aiheur(const MiniOthello& game) {
    auto nn = train_manager.nn;
    V1boardToData(game, nn.p_process1);
    float res;
    eval_network(&nn, nn.p_process1, &res);
    return 2 * res - 1;
}


class ChangeIndex : public Command {
public:
    ChangeIndex() : nb("id", "nouvel indice") {}

    const char* name() const override { return "setpid"; }
    virtual const char* description() const override { return "Change la partie consid�r�."; }
    virtual void execute(TerminalState& state) {
        index = nb.value;
    }

protected:
    virtual void configure(CmdFormat& format) {
        format.addReq(&nb);
    }

    IntParam nb;
};




class EvolvedAI : public TreeAI {
public:
    EvolvedAI(size_t depth = 3, EvalHeur heval = scorediff) :
        TreeAI(&m_wlk, heval), depth(depth), m_wlk(m_tree), turn(0)
    {
    }
    ~EvolvedAI() = default;

    void reset() override
    {
        m_tree.reset();
        turn = 0;
        m_wlk = MinMaxWalker(m_tree);
    }

protected:
    void treeEvoluate() override
    {
        ++++turn;
        if (turn <= 5) {
            m_wlk.alphabeta(noheur, 1);
        }
        else if (turn >= 49) {
            m_wlk.alphabeta(scorediff, 11);
        }
        else {
            m_wlk.alphabeta(heval, depth);
        }
    }

protected:
    MinMaxWalker m_wlk;
    size_t depth;
    int turn;

};


int main() {

    texPion.loadFromFile("ressource\\pion.png");
    gFont.openFromFile("ressource\\arial.ttf");
    wthfr.readFile();

    sf::RenderWindow wndw(sf::VideoMode({ 800u, 600u }), "Othello Game");
    OthelloSave gameLogic{};
    GameView gameView{ GAOthello, gameLogic.getBoard(), 50 }; // Assume GameView is already implemented
    GameBoardManager gbman{ &gameLogic, new HumanPlayer(), new HumanPlayer() };
    OthelloUI othelloUI(wndw, gameView, gbman, gameLogic);
    othelloUI.ai1 = new EvolvedAI(7, sigscorediff);
    othelloUI.ai2 = new EvolvedAI(7, sigscorediff);



    // AI //
    train_manager.loadData(R"(C:\Users\theop\source\repos\GameBoard\GameBoard\data\MasterDatas100000.bin)", 100'000, 90'000);
    train_manager.save_folder = R"(C:\Users\theop\source\repos\GameBoard\GameBoard\data\save\)";
    

    Terminal term{};
    term.addCommand(new ChangeIndex());
    term.addCommand(new SaveNNCommand(train_manager));
    term.addCommand(new LoadNNCommand(train_manager));
    term.addCommand(new InfoNNCommand(train_manager));
    term.addCommand(new TestNNCommand(train_manager));
    term.addCommand(new BackPropNNCommand(train_manager));
    StringView sv{};
    term.addCommand(new PlayCMD(sv));
    std::thread th{ [term]() mutable { term.run(); } };

    //    //



    while (wndw.isOpen()) {
        while (const std::optional<sf::Event> event = wndw.pollEvent()) {
            if (event->is<sf::Event::Closed>())
                wndw.close();
            if (event.has_value())
                othelloUI.handleEvent(event.value());
        }

        // --- Gestion des tours de jeux --- //
        if (!gbman.isFinish() && !gbman.waiting()) { // Si la partie n'est pas fini et si le choix du coup a ete effectue
            if (gbman.playTurn()) { // Si le coup est valide
                othelloUI.updateCanvas();
            }
        }

        othelloUI.updateUI();
        othelloUI.draw();
        wndw.display();
    }

    th.join();

    train_manager.destroyResources();
    
    return 0;
}