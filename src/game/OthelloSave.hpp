#ifndef WTHFILEREADER_HPP
#define WTHFILEREADER_HPP

#include <fstream>
#include <iostream>
#include <vector>
#include <cstdint>

#include "GameBoard.hpp"
#include "GamePlayer.hpp"


struct GameSave {
    std::vector<Pos> listOfMoves;
};

struct GamePointer {
    GamePointer();
    explicit GamePointer(const GameSave* game_save);
    GamePointer(const GameSave* game_save, std::vector<Pos>::const_iterator state);
    GamePointer(const GamePointer&) = default;
    GamePointer& operator=(const GamePointer&) = default;

    GamePointer& operator++();
    GamePointer& operator--();
    GamePointer operator++(int);
    GamePointer operator--(int);
    GamePointer operator+(size_t count) const;
    GamePointer operator-(size_t count) const;
    bool operator==(const GamePointer& other) const;
    bool operator!=(const GamePointer& other) const;
    Pos operator*() const;

    void readTo(GameBoard* board) const;

    const GameSave* m_game_save;
    std::vector<Pos>::const_iterator m_state;

};

class GameReader : public GamePlayer {
public:
    GameReader(const GameSave& game_save);

    void changeGame(const GameSave& game_save);

    void reset() override;
    bool hasNext() const override;
    Pos next() override;
    void updateBoardFromPlay(Pos pos) override;

    void passParam(const Pos& p) override;
    


private:
    GamePointer m_pointer;

    Pos m_next;
    mutable bool p_has_next;

};

struct WTHHeader {
    uint8_t creationCentury;
    uint8_t creationYear;
    uint8_t creationMonth;
    uint8_t creationDay;
    uint32_t numberOfGames;
    uint16_t numberOfPlayers;
    uint16_t gameYear;
    uint8_t boardSize;
    uint8_t isSolitaire;
    uint8_t theoreticalEstimationDepth;
    uint8_t reservedByte;
};

struct WTHGame {
    uint16_t tournamentNumber;
    uint16_t blackPlayerNumber;
    uint16_t whitePlayerNumber;
    uint8_t blackDiscsActual;
    uint8_t blackDiscsTheoretical;
    GameSave gameSave;
};

class WTHFileReader {
public:
    WTHFileReader(const std::string& filepath);

    void readFile();
    WTHHeader getHeader() const;
    const std::vector<WTHGame>& getGames() const;

private:
    std::string m_filepath;
    WTHHeader m_header;
    std::vector<WTHGame> m_games;

    WTHHeader parseHeader(std::ifstream& is);
    void parseGames(std::ifstream& is);
};

#endif // WTHFILEREADER_HPP
