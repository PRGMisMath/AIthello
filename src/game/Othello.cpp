#include "Othello.hpp"

#include<cstring>
#include<cassert>
#include<array>



constexpr std::array<Pos, BOARD_COUNT> generateNeighboorhood(int dx, int dy) {
	Pos out_id = { (unsigned char)BOARD_COUNT };
	std::array<Pos, BOARD_COUNT> res{};
	for (int x = 0; x < BOARD_SIZE; x++) {
		for (int y = 0; y < BOARD_SIZE; y++) {
			if (x + dx >= 0 && x + dx < BOARD_SIZE && y + dy >= 0 && y + dy < BOARD_SIZE)
				res[x * BOARD_SIZE + y] = { (unsigned char) ((x + dx) * BOARD_SIZE + (y + dy)) };
			else
				res[x * BOARD_SIZE + y] = out_id;
		}
	}
	return res;
}

constexpr std::array<Pos, BOARD_COUNT> neighboors_map[8] = {
	generateNeighboorhood(1,0),
	generateNeighboorhood(1,1),
	generateNeighboorhood(0,1),
	generateNeighboorhood(-1,1),
	generateNeighboorhood(-1,0),
	generateNeighboorhood(-1,-1),
	generateNeighboorhood(0,-1),
	generateNeighboorhood(1,-1),
};

#ifdef _DEBUG
constexpr char playerToChar(PlayerID id) {
	switch (id) {
	case PlayerID::Noir:  return 'X';
	case PlayerID::Blanc: return 'O';
	default:              return '.';
	}
}
std::string toString(const PlayerID* grid) {
	constexpr int BOARD_SIZE = 8;
	constexpr int line_length = 2 * BOARD_SIZE + 6; // +5 pour le num�ro de ligne, l'espace et les bords
	constexpr int total_size = (line_length * (BOARD_SIZE + 3)) + 1; // +3 pour les lignes suppl�mentaires (haut, bordure, bas)
	std::string out;
	out.resize(total_size);
	char* p = &out[0];

	// Ligne des lettres en haut avec bordure
	*p++ = ' ';
	*p++ = ' ';
	*p++ = ' ';
	for (char c = 'A'; c < 'A' + BOARD_SIZE; ++c) {
		*p++ = ' ';
		*p++ = c;
	}
	*p++ = ' ';
	*p++ = '\n';

	// Bordure sup�rieure
	*p++ = ' ';
	*p++ = ' ';
	*p++ = ' ';
	for (int j = 0; j < BOARD_SIZE; ++j) {
		*p++ = '+';
		*p++ = '-';
	}
	*p++ = '+';
	*p++ = ' ';
	*p++ = '\n';

	// Plateau avec num�rotation des lignes et bords
	for (int i = 0; i < BOARD_SIZE; ++i) {
		*p++ = '0' + (i + 1); // Num�ro de ligne
		*p++ = ' ';
		*p++ = '|';
		for (int j = 0; j < BOARD_SIZE; ++j) {
			PlayerID val = grid[i * BOARD_SIZE + j];
			*p++ = ' ';
			*p++ = playerToChar(val);
		}
		*p++ = ' ';
		*p++ = '|';
		*p++ = '\n';
	}

	// Bordure inf�rieure
	*p++ = ' ';
	*p++ = ' ';
	*p++ = ' ';
	for (int j = 0; j < BOARD_SIZE; ++j) {
		*p++ = '+';
		*p++ = '-';
	}
	*p++ = '+';
	*p++ = ' ';
	*p++ = '\n';

	return out;
}


#endif // _DEBUG

Othello::Othello() :
	m_board(), m_curr(PlayerID::Noir),
	m_canPlayOn(), nb_piece(), nb_possibility(),
	m_pos_vec()
{
	this->reset();
}

Othello::Othello(const MiniOthello& mini) :
	m_canPlayOn(), m_curr(), m_pos_vec(), nb_possibility()
{
	mini.toRealBoard(m_board);
	m_board[BOARD_COUNT] = PlayerID::Nobody;
	nb_piece[*PlayerID::Noir] = mini.getScore(PlayerID::Noir);
	nb_piece[*PlayerID::Blanc] = mini.getScore(PlayerID::Blanc);
	m_curr = mini.lastPlayer();

#ifdef _DEBUG
	view = toString(m_board);
#endif // _DEBUG
}

void Othello::reset()
{
	memset(m_board, (char)PlayerID::Nobody, BOARD_COUNT + 1);

	// Plateau au debut du jeu //
	m_board[3 * BOARD_SIZE + 3] = PlayerID::Blanc; m_board[3 * BOARD_SIZE + 4] = PlayerID::Noir;
	m_board[4 * BOARD_SIZE + 3] = PlayerID::Noir; m_board[4 * BOARD_SIZE + 4] = PlayerID::Blanc;

	nb_piece[0] = 2; nb_piece[1] = 2;

	m_curr = PlayerID::Noir;

	this->CountPossibilitY(m_curr);

#ifdef _DEBUG
	view = toString(m_board);
#endif // _DEBUG
}

bool Othello::play(Pos pos)
{
	assert(valid(pos)); // Erreur dans l'implementation 

	if (!m_canPlayOn[*pos]) {
		return false;
	}

	std::vector<Pos> Dchange{};
	Dchange.reserve(7);
	for (int d = 0; d < 8; ++d) {
		Pos tID = pos;
		const auto& dir_neighboor = neighboors_map[d];
		while (true) {
			tID = dir_neighboor[*tID]; // Voisin dans la direction d
			if (m_board[*tID] == !m_curr)
				Dchange.push_back(tID);
			else if (m_board[*tID] == m_curr && !Dchange.empty()) {
				// Effectue les changements
				for (Pos DposID : Dchange) {
					m_board[*DposID] = m_curr;
				}
				nb_piece[*m_curr] += Dchange.size();
				nb_piece[*!m_curr] -= Dchange.size();
				break;
			}
			else break; // Cas ou m_board[tID] == PlayerID::Nobody (comprend aussi le cas ou l'on sort du plateau)
		}
		Dchange.clear();
	}

	m_board[*pos] = m_curr;
	++nb_piece[*m_curr];


	this->CountPossibilitY(!m_curr);

	if (nb_possibility == 0) { // Cas ou l'on rejoue
		this->CountPossibilitY(m_curr);
		return true;
	}

	m_curr = !m_curr;

#ifdef _DEBUG
	view = toString(m_board);
#endif // _DEBUG

	return true;
}


bool Othello::canPlay(Pos pos) const
{
	return (m_curr != PlayerID::Nobody) && m_canPlayOn[*pos];
}

const std::vector<Pos>& Othello::getPlayableSpots() const
{
	return m_pos_vec;
}

void Othello::loadFromSavePtr(const GamePointer& gp)
{
	this->reset();
	for (auto cgp = gp.m_game_save->listOfMoves.cbegin(); cgp != gp.m_state; ++cgp) {
		this->play(*cgp);
	}
}

bool Othello::isFinish() const
{
	return nb_possibility == 0;
}

PlayerID Othello::currentPlayer() const
{
	return m_curr;
}

int Othello::getTurn() const
{
	return nb_piece[0] + nb_piece[1] - 4;
}

int Othello::getScore(PlayerID p) const
{
	return (p == PlayerID::Nobody) ? 0 : nb_piece[*p];
}

PlayerID Othello::winner() const
{
	return (nb_piece[0] == nb_piece[1]) ? PlayerID::Nobody : ((nb_piece[0] > nb_piece[1]) ? PlayerID::Noir : PlayerID::Blanc);
}

const PlayerID* Othello::getBoard() const
{
	return m_board;
}


// Pas tres optimal
void Othello::CountPossibilitY(PlayerID p)
{
	memset(m_canPlayOn, false, BOARD_COUNT);
	nb_possibility = 0;
	m_pos_vec.clear();

	if (nb_piece[0] == 0 || nb_piece[1] == 0 || nb_piece[0] + nb_piece[1] == BOARD_COUNT) {
		return;
	}

	for (Pos x_p8y = START; valid(x_p8y); ++x_p8y)
		if (this->IsPlayable(p, x_p8y)) {
			++nb_possibility;
			m_canPlayOn[*x_p8y] = true;
			m_pos_vec.push_back(x_p8y);
		}
}

bool Othello::IsPlayable(PlayerID p, Pos pos) const
{
	if (m_board[*pos] != PlayerID::Nobody)
		return false;

	for (int d = 0; d < 8; ++d) {
		Pos tID = pos;
		bool has_opon = false;
		const auto& dir_neighboor = neighboors_map[d];
		while (true) {
			tID = dir_neighboor[*tID];
			if (m_board[*tID] == !p)
				has_opon = true;
			else if (m_board[*tID] == p && has_opon)
				return true;
			else break;
		}
	}

	return false;
}

#ifdef _DEBUG
Othello::Othello(float *src) {
	assert(src != nullptr);
	for (int id = 0; id < BOARD_COUNT; ++id) {
		if (src[2 * id] > 0) {
			m_board[id] = PlayerID::Noir;
		}
		else if (src[2 * id + 1] > 0) {
			m_board[id] = PlayerID::Blanc;
		}
		else {
			m_board[id] = PlayerID::Nobody;
		}
	}
	view = toString(m_board);
}
#endif // _DEBUG

void OthelloSave::reset()
{
	Othello::reset();
	m_game_save.listOfMoves.clear();
}

bool OthelloSave::play(Pos pos)
{
	if (!Othello::play(pos))
		return false;

	m_game_save.listOfMoves.push_back(pos);
	return true;
}

bool OthelloSave::unplay()
{
	if (m_game_save.listOfMoves.empty())
		return false;
	auto copy = m_game_save;
	this->loadFromSavePtr(GamePointer(&copy, copy.listOfMoves.cend() - 1));
	return true;
}

const GameSave& OthelloSave::getGameSave() const
{
	return m_game_save;
}

GamePointer OthelloSave::getGamePointer() const
{
	return GamePointer(&m_game_save, m_game_save.listOfMoves.cend());
}
