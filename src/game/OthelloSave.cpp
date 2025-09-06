#include "OthelloSave.hpp"

constexpr Pos WTHDecodeMove(uint8_t move, int rowColCoeff) {
    move -= rowColCoeff + 1;
    return { (unsigned char) ((move / rowColCoeff) + (move % rowColCoeff) * BOARD_SIZE) };
}

WTHFileReader::WTHFileReader(const std::string& filepath)
    : m_filepath(filepath), m_header(), m_games() {
}

void WTHFileReader::readFile() {
    std::ifstream is(m_filepath, std::ios_base::binary);
    if (!is) {
        throw std::runtime_error("Failed to read file: " + m_filepath);
    }
    m_header = parseHeader(is);
    parseGames(is);
}

WTHHeader WTHFileReader::getHeader() const {
    return m_header;
}

const std::vector<WTHGame>& WTHFileReader::getGames() const {
    return m_games;
}

WTHHeader WTHFileReader::parseHeader(std::ifstream& is) {
    WTHHeader header;
    is.read(reinterpret_cast<char*>(&header.creationCentury), 1);
    is.read(reinterpret_cast<char*>(&header.creationYear), 1);
    is.read(reinterpret_cast<char*>(&header.creationMonth), 1);
    is.read(reinterpret_cast<char*>(&header.creationDay), 1);
    is.read(reinterpret_cast<char*>(&header.numberOfGames), 4);
    is.read(reinterpret_cast<char*>(&header.numberOfPlayers), 2);
    is.read(reinterpret_cast<char*>(&header.gameYear), 2);
    is.read(reinterpret_cast<char*>(&header.boardSize), 1);
    is.read(reinterpret_cast<char*>(&header.isSolitaire), 1);
    is.read(reinterpret_cast<char*>(&header.theoreticalEstimationDepth), 1);
    is.read(reinterpret_cast<char*>(&header.reservedByte), 1);
    return header;
}

void WTHFileReader::parseGames(std::ifstream& is) {
    const int moveCount = (m_header.boardSize == 0 || m_header.boardSize == 8) ? 60 : 96;
    const int rowColCoeff = (m_header.boardSize == 0 || m_header.boardSize == 8) ? 10 : 12;

    for (int i = 0; i < m_header.numberOfGames; ++i) {
        WTHGame game;
        is.read(reinterpret_cast<char*>(&game.tournamentNumber), 2);
        is.read(reinterpret_cast<char*>(&game.blackPlayerNumber), 2);
        is.read(reinterpret_cast<char*>(&game.whitePlayerNumber), 2);
        is.read(reinterpret_cast<char*>(&game.blackDiscsActual), 1);
        is.read(reinterpret_cast<char*>(&game.blackDiscsTheoretical), 1);

        std::vector<uint8_t> rawMoves(moveCount);
        is.read(reinterpret_cast<char*>(rawMoves.data()), moveCount);

        for (uint8_t move : rawMoves) {
            if (move != 0) {
                game.gameSave.listOfMoves.push_back(WTHDecodeMove(move, rowColCoeff));
            }
        }

        m_games.push_back(game);
    }
}

GameReader::GameReader(const GameSave& game_save) :
    m_pointer(&game_save), p_has_next(false), m_next() { }

void GameReader::changeGame(const GameSave& game_save)
{
    m_pointer = GamePointer(&game_save);
}

void GameReader::reset()
{
    m_pointer = GamePointer(m_pointer.m_game_save);
}

bool GameReader::hasNext() const
{
    if (p_has_next)
    {
        p_has_next = false;
        return true;
    }
    return false;
}

Pos GameReader::next()
{
    return m_next;
}

void GameReader::updateBoardFromPlay(Pos pos)
{
}


void GameReader::passParam(const Pos& p) {
    if (p == START && m_pointer.m_state != m_pointer.m_game_save->listOfMoves.cend()) {
        p_has_next = true;
        m_next = *(m_pointer++);
    }
    if (p == INVALID && m_pointer.m_state != m_pointer.m_game_save->listOfMoves.cbegin()) {
        m_pointer--; 
        m_next = INVALID;
    }
}

GamePointer::GamePointer() :
    m_game_save(nullptr), m_state()
{
}

GamePointer::GamePointer(const GameSave* game_save) :
    m_game_save(game_save), m_state(game_save->listOfMoves.cbegin())
{
}

GamePointer::GamePointer(const GameSave* game_save, std::vector<Pos>::const_iterator state) :
    m_game_save(game_save), m_state(state)
{
}

GamePointer& GamePointer::operator++() {
    ++m_state;
    return *this;
}

GamePointer& GamePointer::operator--() {
    --m_state;
    return *this;
}

GamePointer GamePointer::operator++(int) {
    const auto _Temp = *this;
    ++m_state;
    return _Temp;
}

GamePointer GamePointer::operator--(int) {
    const auto _Temp = *this;
    --m_state;
    return _Temp;
}

GamePointer GamePointer::operator+(size_t count) const {
    auto _Copy = *this;
    _Copy.m_game_save = m_game_save;
    _Copy.m_state = m_state + count;
    return _Copy;
}

GamePointer GamePointer::operator-(size_t count) const {
    auto _Copy = *this;
    _Copy.m_game_save = m_game_save;
    _Copy.m_state = m_state - count;
    return _Copy;
}

bool GamePointer::operator==(const GamePointer &other) const {
    return m_game_save == other.m_game_save && m_state == other.m_state;
}

bool GamePointer::operator!=(const GamePointer &other) const {
    return !(*this == other);
}

Pos GamePointer::operator*() const {
    return *m_state;
}

void GamePointer::readTo(GameBoard *board) const {
    if (m_game_save == nullptr)
        return;
    auto iter = m_game_save->listOfMoves.cbegin();
    while (iter != m_state) {
        board->play(*iter);
    }
}
