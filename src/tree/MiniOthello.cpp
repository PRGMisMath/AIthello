#include "MiniOthello.hpp"

#include<cassert>
#include<array>


constexpr uint64_t playerFilter[2] = {
	UINT64_C(0b01010101'01010101'01010101'01010101'01010101'01010101'01010101'01010101),
	UINT64_C(0b10101010'10101010'10101010'10101010'10101010'10101010'10101010'10101010)
};

// Remarque : si pos == 64 (position invalide), alors res = 0
constexpr _uint128_t posToMask(unsigned char pos) {
	_uint128_t res{};
	if (pos >= 32) {
		res.low = 0;
		res.high = ((uint64_t)3) << (2 * (pos - 32));
	}
	else {
		res.low = ((uint64_t)3) << (2 * pos);
		res.high = 0;
	}
	return res;
}

constexpr std::array<Pos, BOARD_COUNT> generateNeighboorhood(int dx, int dy) {
	Pos out_id = { (unsigned char)BOARD_COUNT };
	std::array<Pos, BOARD_COUNT> res{};
	for (int x = 0; x < BOARD_SIZE; x++) {
		for (int y = 0; y < BOARD_SIZE; y++) {
			if (x + dx >= 0 && x + dx < BOARD_SIZE && y + dy >= 0 && y + dy < BOARD_SIZE)
				res[x * BOARD_SIZE + y] = { (unsigned char)((x + dx) * BOARD_SIZE + (y + dy)) };
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

constexpr std::array<_uint128_t, BOARD_COUNT + 1> generateMaskMap() {
	Pos out_id = { (unsigned char)BOARD_COUNT };
	std::array<_uint128_t, BOARD_COUNT + 1> res{};
	for (int pos = 0; pos < BOARD_COUNT; pos++) {
		res[pos] = posToMask(pos);
	}
	res[BOARD_COUNT] = { UINT64_C(0), UINT64_C(0) };
	return res;
}

constexpr std::array<_uint128_t, BOARD_COUNT + 1> mask_map = generateMaskMap();



MiniOthello::MiniOthello() :
	m_flag(0), m_board({UINT64_C(0),UINT64_C(0)})
{
	PlacePiece(mask_map[3 * BOARD_SIZE + 3], playerFilter[*PlayerID::Blanc]);
	PlacePiece(mask_map[4 * BOARD_SIZE + 3], playerFilter[*PlayerID::Noir]);
	PlacePiece(mask_map[3 * BOARD_SIZE + 4], playerFilter[*PlayerID::Noir]);
	PlacePiece(mask_map[4 * BOARD_SIZE + 4], playerFilter[*PlayerID::Blanc]);

	nb_piece[0] = 2; nb_piece[1] = 2;

	m_curr = PlayerID::Blanc;
}

inline void MiniOthello::FlipPiecE(const _uint128_t& maskPos)
{
	m_board.low ^= maskPos.low;
	m_board.high ^= maskPos.high;
}

inline void MiniOthello::PlacePiece(const _uint128_t& maskPos, const uint64_t& maskPlayer)
{
	m_board.low |= (maskPos.low & maskPlayer);
	m_board.high |= (maskPos.high & maskPlayer);
}

inline bool MiniOthello::IsPlayerHerE(const _uint128_t& maskPos, const uint64_t& maskPlayer) const
{
	return ((m_board.low & maskPos.low & maskPlayer) != 0) | ((m_board.high & maskPos.high & maskPlayer) != 0);
}

inline bool MiniOthello::IsEmptyHerE(const _uint128_t& maskPos) const
{
	return ((m_board.low & maskPos.low) == 0) & ((m_board.high & maskPos.high) == 0);
}

const std::vector<MiniOthello> MiniOthello::GenerateNextBoardS(PlayerID curr) const
{
	std::vector<MiniOthello> res{};
	MiniOthello work = *this;
	work.m_curr = curr;
	for (Pos p = START; valid(p); ++p) {
		if (work.play(p)) {
			work.m_l_play = p;
			res.push_back(work);
			work = *this;
			work.m_curr = curr;
		}
	}
	return res;
}


bool MiniOthello::play(Pos pos)
{
	assert(valid(pos)); // Erreur dans l'implementation

	if (!IsEmptyHerE(mask_map[*pos]))
		return false;

	bool isValidPlay = false;

	uint64_t currFilter = playerFilter[*m_curr],
		oppFilter = playerFilter[*!m_curr];


	_uint128_t Dchange[7]; // Sauvegarde les pieces
	int_fast8_t sDchange = 0;
	for (int d = 0; d < 8; ++d) {
		Pos tID = pos;
		const auto& dir_neighboor = neighboors_map[d];
		while (true) {
			tID = dir_neighboor[*tID]; // Voisin dans la direction d
			_uint128_t mask = mask_map[*tID];
			if (IsPlayerHerE(mask, oppFilter)) {
				Dchange[sDchange] = mask;
				++sDchange;
			}
			else if (IsPlayerHerE(mask, currFilter) && (sDchange != 0)) {
				// Indique qu'au moins une direction a retourne une piece ==> Coup valide
				isValidPlay = true;

				// Met a jour les pieces
				nb_piece[*m_curr] += sDchange;
				nb_piece[*!m_curr] -= sDchange;

				// Effectue les changements
				do {
					FlipPiecE(Dchange[--sDchange]);
				} while (sDchange != 0);
				break;
			}
			else break; // Cas ou m_board[tID] == PlayerID::Nobody (comprend aussi le cas ou l'on sort du plateau)
		}
		sDchange = 0;
	}

	if (!isValidPlay)
		return false;

	PlacePiece(mask_map[*pos], currFilter);
	++nb_piece[*m_curr];

	return true;
}

bool MiniOthello::isFinish() const
{
	return m_flag == 1;
}

int MiniOthello::getScore(PlayerID p) const
{
	return (p == PlayerID::Nobody) ? 0 : nb_piece[*p];
}

int MiniOthello::getTurn() const {
	return nb_piece[0] + nb_piece[1] - 4;
}

Pos MiniOthello::lastPlay() const
{
	return m_l_play;
}

PlayerID MiniOthello::lastPlayer() const
{
	return m_curr;
}

const std::vector<MiniOthello> MiniOthello::nextBoards() const
{
	// On essaie de generer les coups pour l'adversaire
	const std::vector<MiniOthello> res = GenerateNextBoardS(!m_curr);

	if (!res.empty())
		return res;

	// Puis pour celui qui vient de jouer
	const std::vector<MiniOthello> res2 = GenerateNextBoardS(m_curr);

	if (res2.empty())
		m_flag = 1;

	return res2;
}

int MiniOthello::countMoves(PlayerID player) const {
	return GenerateNextBoardS(player).size();
}

void MiniOthello::toRealBoard(PlayerID* dest) const
{
	for (Pos p = START; valid(p); ++p) {
		if (IsPlayerHerE(mask_map[*p], playerFilter[*PlayerID::Noir]))
			dest[*p] = PlayerID::Noir;
		else if (IsPlayerHerE(mask_map[*p], playerFilter[*PlayerID::Blanc]))
			dest[*p] = PlayerID::Blanc;
		else
			dest[*p] = PlayerID::Nobody;
	}
}
