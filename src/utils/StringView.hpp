#ifndef STRING_VIEW_HPP
#define STRING_VIEW_HPP

#include "../game/Othello.hpp"
#include "../game/GameBoardManager.hpp"
#include "Terminal.hpp"
#include "../tree/MinMaxTree.hpp"


class StringView {
public:
    StringView(AIPlayer* ai1, AIPlayer* ai2);
    ~StringView();


private:
    friend class PlayCMD;

    OthelloSave logic;
    AIPlayer* ai1, *ai2;
};

class PlayCMD : public Command {
public:
    explicit PlayCMD(StringView& sv, const WTHFileReader& reader);
    const char* name() const override;
    const char* description() const override;
    void execute(TerminalState& state) override;

protected:
    void configure(CmdFormat& format) override;

private:
    StringView& sv;
    IntParam startPos;
    IntParam idGame;
    const WTHFileReader& reader;

};



#endif // !STRING_VIEW_HPP
