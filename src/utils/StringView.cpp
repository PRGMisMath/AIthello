#include "StringView.hpp"

#include "Random.hpp"

StringView::~StringView() {
    delete ai1;
    delete ai2;
}

PlayCMD::PlayCMD(StringView& sv, const WTHFileReader& reader) :
	sv(sv), reader(reader),
    startPos("turn", "Nombre de tour a passer"),
    idGame("game", "ID de la partie (random par defaut)")
{
}

const char* PlayCMD::name() const
{
	return "play";
}

const char* PlayCMD::description() const
{
	return "Permet de jouer une partie.";
}


constexpr char playerToChar(PlayerID id) {
    switch (id) {
    case PlayerID::Noir:  return 'X';
    case PlayerID::Blanc: return 'O';
    default:              return '.';
    }
}
// Fonction pour dessiner une grille de 8x8 avec des cases de taille a*b
void drawGrid(std::ostream& os, const PlayerID* board, const std::vector<TreeNode::Rope>* annot = nullptr, int a = 1, int b = 3) {
    constexpr int gridSize = 8;
    constexpr char corner = '+', vBorder = '|', hBorder = '-';

    float eval[64];
    for (Pos p = START; valid(p); ++p) {
        eval[*p] = NAN;
    }
    if (annot != nullptr) {
        for (const auto& rope: *annot) {
            eval[*rope.pos] = rope.eval.minMax;
        }
    }

    // Imprimer les lettres de colonnes
    os << "   ";
    for (int i = 0; i < gridSize; ++i) {
        os << std::setw((b+1) / 2) << static_cast<char>('A' + i) << std::string(b / 2 + 1, ' ');
    }
    os << std::endl;

    // Imprimer la grille
    for (int row = 0; row < gridSize; ++row) {
        os << "  ";

        // Bordure supérieure de chaque case de la ligne
        for (int col = 0; col < gridSize; ++col) {
            os << corner;
            for (int i = 0; i < b; ++i) {
                os << hBorder;
            }
        }
        os << corner << std::endl;

        // Contenu de chaque cellule
        for (int line = 0; line < a; ++line) {
            if (line == a / 2)
                os << row + 1 << " " << vBorder;
            else
                os << "  " << vBorder; // Bordure gauche de la ligne
            for (int col = 0; col < gridSize; ++col) {
                int index = row * gridSize + col;
                std::string cellContent = std::string(1,playerToChar(board[index]));
                if (!isnan(eval[index]))
                    cellContent = std::to_string((int)eval[index]);
                // Centrer le texte verticalement
                if (line == a / 2) {
                    // Calculer le padding pour centrer horizontalement
                    int padding = (b - cellContent.length()) / 2;
                    padding = padding > 0 ? padding : 0;
                    std::cout << std::setw(padding + cellContent.length()) << cellContent;
                    // Remplir avec des espaces si nécessaire
                    for (int i = cellContent.length() + padding; i < b; ++i) {
                        std::cout << " ";
                    }
                } else {
                    // Remplir avec des espaces si on n'est pas sur la ligne centrale
                    for (int i = 0; i < b; ++i) {
                        os << " ";
                    }
                }
                os << vBorder; // Bordure droite de la case
            }
            os << std::endl;
        }
    }

    // Bordure inférieure de la grille (optionnel pour compléter la bordure)
    os << "  ";
    for (int col = 0; col < gridSize; ++col) {
        os << corner;
        for (int i = 0; i < b; ++i) {
            os << hBorder;
        }
    }
    os << corner << std::endl;
}

void PlayCMD::execute(TerminalState& state)
{
    std::ostream& os = *(state.fluxOut);

    GameBoardManager gbman{ &sv.logic, sv.ai1, sv.ai2 };
    gbman.reset();
    GameTree tree{};
    MinMaxWalker wlk{tree};

    if (startPos) {
        int id = idGame.value;
        if (!idGame || id < 0 || id >= reader.getGames().size())
            id = rd::src.uniform_int(0, reader.getGames().size() - 1);
        if (startPos >= reader.getGames()[id].gameSave.listOfMoves.size()) {
            os << "Position de depart trop eleve \n";
            return;
        }
        (GamePointer(&(reader.getGames()[id].gameSave)) + startPos).readTo(&sv.logic);
    }

    wlk.alphabeta(scorediff, 1);

    drawGrid(os, sv.logic.getBoard(), &(wlk.getRoot()->ropes));

    while (!gbman.isFinish()) {
        Pos pos = gbman.playTurn();
        if (!valid(pos)) {
            int x = -1, y = -1;
            std::string buff;
            while (true) {
                os << "\nCoup : ";
                std::getline(*(state.fluxIn), buff);
                if (buff == "exit" || buff == "end" || buff == "stop")
                    return;
                if (buff.size() != 2) {
                    os << "Erreur dans le format ! (ex : a8 ; H3; d7)\n\n";
                    continue;
                }
                if (buff[0] >= 'a' && buff[0] <= 'h')
                    x = buff[0] - 'a';
                else
                    x = buff[0] - 'A';
                y = buff[1] - '1';
                if (x < 0 || x >= BOARD_SIZE || y < 0 || y >= BOARD_SIZE) {
                    os << "Erreur dans le format ! (ex : a8 ; H3; d7)\n\n";
                    continue;
                }
                Pos play = { (unsigned char)(x + BOARD_SIZE * y) };
                if (!sv.logic.canPlay(play)) {
                    os << "Coup invalide !\n\n";
                    continue;
                }
                gbman.passParam(play);
				break;
            }

        }
        else {
            wlk.down(pos);
            if (sv.logic.getTurn() <= 50)
                wlk.alphabeta(scorediff, 8);
            else
                wlk.alphabeta(scorediff, 14);
            drawGrid(os, sv.logic.getBoard(), &(wlk.getRoot()->ropes));
        }
    }
}

void PlayCMD::configure(CmdFormat& format)
{
}

StringView::StringView(GamePlayer* ai1, GamePlayer* ai2) :
    logic(), ai1(ai1), ai2(ai2)
{
}
