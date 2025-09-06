#ifndef OTHELLO_UI_HPP
#define OTHELLO_UI_HPP

#include<TGUI/TGUI.hpp>
#include<TGUI/Backend/SFML-Graphics.hpp>
#include<SFML/Graphics.hpp>

#include "../game/Othello.hpp"
#include "../GameView.hpp"
#include "GamePlayer.hpp"
#include "../game/GameBoardManager.hpp"
#include "MinMaxTree.hpp"
#include "../tree/PreferTree.hpp"



static WTHFileReader wthfr{ "data\\games\\WTH_FUSION.wtb" };
inline int index = 0;

static sf::Texture texPion{};
static sf::Font gFont{};

constexpr GameAppear GAOthello = {
    {8,8},
    2,
    &texPion,
    {20,20}
};



class OthelloUI {
public:
    OthelloUI(sf::RenderWindow& window, GameView& gameView, GameBoardManager& gbman, OthelloSave& gameLogic);
    void setupUI();
    void handleEvent(sf::Event event);
    void updateUI();
    void draw();
    void updateCanvas();

private:
    sf::RenderWindow& window;
    tgui::Gui gui;

    // UI Elements
    tgui::Tabs::Ptr modeTabs;
    tgui::Label::Ptr aiEvaluationLabel;
    tgui::Button::Ptr newGameButton;
    tgui::Button::Ptr undoMoveButton;
    tgui::Panel::Ptr rightPanel;
    tgui::Label::Ptr possibleMovesLabel;
    tgui::CanvasSFML::Ptr canvas;
    sf::Color backColor;

    // Game Logic and View
    GameBoardManager& gbMan;
    GameView& gameView;
    OthelloSave& gameLogic;

    // Players
    GameReader rd;
    HumanPlayer h1, h2;

public:
    AIPlayer* ai1, * ai2;

private:
    // Mem
    tgui::String m_mode;


    void onNewGame();
    void onUndoMove();
    void onModeChange(const tgui::String& strSel);
};

OthelloUI::OthelloUI(sf::RenderWindow& window, GameView& gameView, GameBoardManager& gbman, OthelloSave& gameLogic)
    : window(window), gui(window), gameView(gameView), gbMan(gbman), gameLogic(gameLogic),
    rd(wthfr.getGames()[index].gameSave), h1(), h2(), ai1(), ai2()
{
    setupUI();
}

void OthelloUI::setupUI() {
    // Create a main group to hold all widgets
    auto mainGroup = tgui::Group::create();
    mainGroup->setPosition("0%", "0%");
    mainGroup->setSize("100%", "100%");
    gui.add(mainGroup);

    // Create mode selection tabs
    modeTabs = tgui::Tabs::create();
    modeTabs->add("2 Joueur");
    modeTabs->add("VS Bot");
    modeTabs->add("Bot VS Bot");
    modeTabs->add("Revue");
    modeTabs->setPosition("0%", "0%");
    modeTabs->setSize("100%", "5%");
    modeTabs->select("2 Joueur");
    modeTabs->onTabSelect([this](const tgui::String& strSel) {
        onModeChange(strSel); updateCanvas();
        });
    onModeChange("2 Joueur");
    mainGroup->add(modeTabs);

    // Create AI evaluation label
    aiEvaluationLabel = tgui::Label::create();
    aiEvaluationLabel->setPosition("20%", "6%");
    aiEvaluationLabel->setSize("100%", "13%");
    aiEvaluationLabel->setText("");
    aiEvaluationLabel->setTextSize(20);
    mainGroup->add(aiEvaluationLabel);

    // Create navigation buttons
    newGameButton = tgui::Button::create("New Game");
    newGameButton->setPosition("15%", "90%");
    newGameButton->setSize("20%", "8%");
    newGameButton->onClick([this]() {
        onNewGame(); updateCanvas();
        });
    mainGroup->add(newGameButton);

    undoMoveButton = tgui::Button::create("Undo Move");
    undoMoveButton->setPosition("40%", "90%");
    undoMoveButton->setSize("20%", "8%");
    undoMoveButton->onClick([this]() {
        onUndoMove(); updateCanvas();
        });
    mainGroup->add(undoMoveButton);

    // Create right panel for bot mode
    rightPanel = tgui::Panel::create();
    rightPanel->setPosition("80%", "5%");
    rightPanel->setSize("20%", "85%");
    rightPanel->getRenderer()->setBackgroundColor(tgui::Color(230, 230, 230));
    mainGroup->add(rightPanel);

    possibleMovesLabel = tgui::Label::create();
    possibleMovesLabel->setPosition("0%", "0%");
    possibleMovesLabel->setSize("100%", "10%");
    possibleMovesLabel->setText("Possible Moves:");
    rightPanel->add(possibleMovesLabel);

    // Create CanvasSFML for GameView
    canvas = tgui::CanvasSFML::create();
    canvas->setPosition("7%", "10%");
    updateCanvas();
    mainGroup->add(canvas);
}

void OthelloUI::handleEvent(sf::Event event) {
    if (event.is<sf::Event::MouseButtonPressed>() && m_mode != "Revue")
    {
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        auto canvasPos = canvas->getAbsolutePosition();
        float relX = (mousePos.x - canvasPos.x) / gameView.getCaseSize() - 1.f;
        float relY = (mousePos.y - canvasPos.y) / gameView.getCaseSize() - 1.f;

        // V�rifier si la souris est bien dans le canvas
        if (relX >= 0 && relY >= 0 && relX < 8 && relY < 8)
        {
            gbMan.passParam({ (unsigned char)((int)relX + 8 * (int)relY) });
        }
    }
    else if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
        if (m_mode == "Revue") {
            if (key->scancode == sf::Keyboard::Scancode::Left) {
                gbMan.passParam(INVALID);
                onUndoMove(); updateCanvas();
            }
            else if (key->scancode == sf::Keyboard::Scancode::Right) {
                gbMan.passParam(START);
            }
        }
    }
    else if (const auto* size = event.getIf<sf::Event::Resized>()) {
        const float fnewCaseSize = std::min(size->size.x * 0.5f / BOARD_SIZE, size->size.y * 0.67f / BOARD_SIZE);
        gameView.setCaseSize((int)fnewCaseSize);
        updateCanvas();
    }
    gui.handleEvent(event);
}

void OthelloUI::updateUI() {
    std::string textEv{};
    std::string scoreN = std::to_string(gameLogic.getScore(PlayerID::Noir)),
        scoreB = std::to_string(gameLogic.getScore(PlayerID::Blanc));
    std::string nameN, nameB;
    if (m_mode == "Revue") {
        nameN = "N*" + std::to_string(wthfr.getGames()[index].blackPlayerNumber);
        nameB = "N*" + std::to_string(wthfr.getGames()[index].whitePlayerNumber);
    }
    else if (m_mode == "2 Joueur") {
        nameN = "Noir";
        nameB = "Blanc";
    }
    else if (m_mode == "VS Bot") {
        nameN = "Humain";
        nameB = "AI Bot";
    }
    else if (m_mode == "Bot VS Bot") {
        nameN = "MinMax7";
        nameB = "AI6->5";
    }

    nameN += " (N)";
    nameB += " (B)";

    if (gameLogic.isFinish()) {
        switch (gameLogic.winner())
        {
        case PlayerID::Nobody:
            textEv = "Egalite !";
            break;
        case PlayerID::Noir:
            textEv = "Victoire de " + nameN + " (" + scoreN + " vs " + scoreB + ") !";
            break;
        case PlayerID::Blanc:
            textEv = "Victoire de " + nameB + " (" + scoreB + " vs " + scoreN + ") !";
            break;
        }
    }
    else
        textEv = nameN + " : " + scoreN + " --- " + nameB + " : " + scoreB;
    
    aiEvaluationLabel->setText(textEv);

    // Update possible moves list in right panel
    // Example: possibleMovesLabel->setText("Possible Moves: " + gameLogic->getPossibleMoves());
}

void OthelloUI::draw() {
    window.clear(backColor);
    gui.draw();
}

void OthelloUI::onNewGame() {
    gbMan.reset();
    if (m_mode == "Revue") {
        rd = GameReader(wthfr.getGames()[index].gameSave);
    }
}

void OthelloUI::onUndoMove() {
    gameLogic.unplay();
}

void OthelloUI::onModeChange(const tgui::String& strSel) {
    m_mode = strSel;
    if (strSel == "Revue") {
        backColor = tgui::Color(169, 169, 169); // Blue-gray color
        rd = GameReader(wthfr.getGames()[index].gameSave);
        gbMan.reset(&rd, &rd);
    }
    else if (strSel == "2 Joueur") {
        backColor = tgui::Color(222, 184, 135); // Beige color
        gbMan.reset(&h1, &h2);
    }
    else if (strSel == "VS Bot") {
        gbMan.reset(&h1, ai2);
    }
    else if (strSel == "Bot VS Bot") {
        gbMan.reset(ai1, ai2);
    }
}

void OthelloUI::updateCanvas() {
    canvas->setSize((BOARD_SIZE + 1.5f) * gameView.getCaseSize(), (BOARD_SIZE + 1.5f) * gameView.getCaseSize());

    gameView.update();
    canvas->clear(backColor);

    float caseWidth = gameView.getCaseSize();
    gameView.setPosition({ caseWidth, caseWidth });
    canvas->draw(gameView);

    sf::RectangleShape rpound{ { .8f * caseWidth, .8f * caseWidth } };
    rpound.setPosition({ .1f * caseWidth, .1f * caseWidth });
    rpound.setTexture(&texPion);
    rpound.setTextureRect(GameView::poundTextureRect(gameLogic.currentPlayer()));
    canvas->draw(rpound);

    // --- Labels --- //
    sf::Text text{ gFont };
    text.setCharacterSize(15);
    text.setStyle(sf::Text::Bold);
    text.setFillColor(sf::Color::Black);

    for (int col = 0; col < BOARD_SIZE; ++col) {
        text.setString(std::string(1, 'A' + col));
        sf::FloatRect textBounds = text.getLocalBounds();
        text.setPosition({ (col + 1.5f) * caseWidth - textBounds.size.x / 2, .5f * caseWidth });
        canvas->draw(text);
    }
    for (int row = 0; row < BOARD_SIZE; ++row) {
        text.setString(std::to_string(row + 1));
        sf::FloatRect textBounds = text.getLocalBounds();
        text.setPosition({ .5f * caseWidth, (row + 1.5f) * caseWidth - textBounds.size.y / 2 });
        canvas->draw(text);
    }
    // -------------- //

    canvas->display();
}


#endif // !OTHELLO_UI_HPP
