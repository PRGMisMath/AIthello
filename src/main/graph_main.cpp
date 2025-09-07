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
#include "../train/OthelloTrain.hpp"
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




class ChangeIndex : public Command {
public:
    ChangeIndex() : nb("id", "nouvel indice") {}

    const char* name() const override { return "setpid"; }
    virtual const char* description() const override { return "Change la partie consid�r�."; }
    virtual void execute(TerminalState& state) {
        wth_index = nb.value;
    }

protected:
    virtual void configure(CmdFormat& format) {
        format.addReq(&nb);
    }

    IntParam nb;
};




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

    texPion.loadFromFile("../ressource/pion.png");
    gFont.openFromFile("../ressource/arial.ttf");
    wthfr.readFile();

    sf::RenderWindow wndw(sf::VideoMode({ 800u, 600u }), "Othello Game");
    OthelloSave gameLogic{};
    GameView gameView{ GAOthello, gameLogic.getBoard(), 50 }; // Assume GameView is already implemented
    GameBoardManager gbman{ &gameLogic, new HumanPlayer(), new HumanPlayer() };
    OthelloUI othelloUI(wndw, gameView, gbman, gameLogic);
    othelloUI.ai1 = new EvolvedMinMaxAI(7, sigscorediff);
    othelloUI.ai2 = new EvolvedPreferAI(10000, 2, scorediff);



    // AI //
    // TODO : Compile data (to big for github) to use the train command)
    //train_manager.loadData(R"(..\data\MasterDatas100000.bin)", 100'000, 90'000);
    train_manager.save_folder = R"(..\data\save\)";
    

    Terminal term{};
    term.addCommand(new ChangeIndex());
    term.addCommand(new SaveNNCommand(train_manager));
    term.addCommand(new LoadNNCommand(train_manager));
    term.addCommand(new InfoNNCommand(train_manager));
    term.addCommand(new TestNNCommand(train_manager));
    term.addCommand(new BackPropNNCommand(train_manager));
    StringView sv{};
    term.addCommand(new PlayCMD(sv, wthfr));
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
            if (valid(gbman.playTurn())) { // Si le coup est valide
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
