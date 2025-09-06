#ifndef GAME_PLAYER_HPP
#define GAME_PLAYER_HPP

#include "GameBoard.hpp"
#include<string>


class GamePlayer {
public:
	GamePlayer() = default;
	virtual ~GamePlayer() = default;

	virtual bool hasNext() const = 0;
	virtual Pos next() = 0;
	virtual void reset() = 0;
	virtual void updateBoardFromPlay(Pos pos) = 0;
	virtual void passParam(const Pos& p) {}
	virtual void invalidPlay() {}

public:
	std::string name;

};

class HumanPlayer : public GamePlayer {
public:
	HumanPlayer() : GamePlayer(), waiting(true), pnext() {}
	bool hasNext() const override
	{
		return !waiting;
	}
	Pos next() override
	{
		waiting = true;
		return pnext;
	}
	void passParam(const Pos& p) override {
		pnext = p;
		waiting = false;
	}
	void updateBoardFromPlay(Pos pos) override {}
	void reset() override { waiting = true; }

private:
	Pos pnext;
	bool waiting;

};

class AIPlayer : public GamePlayer {
public:
	explicit AIPlayer(int wait_time = 25) :
		GamePlayer(),
		waiting(wait_time), wait_time(wait_time)
	{
	}

	virtual Pos calcNextMove() = 0;
	Pos next() override
	{
		waiting = wait_time;
		return calcNextMove();
	}
	bool hasNext() const override
	{
		--waiting;
		return !waiting;
	}

private:
	mutable int waiting;

public:
	int wait_time;

};


#endif // !GAME_PLAYER_HPP
