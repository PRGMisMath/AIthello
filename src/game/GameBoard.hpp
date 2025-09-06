#ifndef GAME_BOARD_HPP
#define GAME_BOARD_HPP


constexpr int BOARD_SIZE = 8;
constexpr int BOARD_COUNT = BOARD_SIZE * BOARD_SIZE;


struct Pos { unsigned char x_p8y; };
constexpr Pos START = { 0u };
constexpr Pos INVALID = { 255u };
inline constexpr bool operator==(Pos p1, Pos p2) { return p1.x_p8y == p2.x_p8y; }
inline constexpr unsigned char operator*(Pos pos) { return pos.x_p8y; }
inline constexpr void operator++(Pos& pos) { ++pos.x_p8y; }
inline constexpr int x(Pos pos) { return pos.x_p8y % BOARD_SIZE; }
inline constexpr int y(Pos pos) { return pos.x_p8y / BOARD_SIZE; }
inline constexpr bool valid(Pos pos) { return pos.x_p8y < BOARD_COUNT; }


enum class PlayerID : char {
	Nobody = 0,
	Noir = -1,
	Blanc = 1
};

inline constexpr PlayerID operator!(PlayerID p) { return (PlayerID)(-(char)p); }
inline constexpr int operator*(PlayerID p) { return ((char)p + 1) >> 1; }



class GameBoard {
public:
	virtual bool play(Pos pos) = 0;
	virtual bool canPlay(Pos pos) const = 0;
	
	virtual bool isFinish() const = 0;
	virtual PlayerID currentPlayer() const = 0;
	virtual PlayerID winner() const = 0;

	virtual void reset() = 0;

};





#endif