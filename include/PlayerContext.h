#ifndef PLAYERCONTEXT_H
#define PLAYERCONTEXT_H

#include "GameContext.h"
#include <cstdint>
#include <array>

enum class Action : uint8_t {
    Check,
    Raise,
    AllIn,
    Call,
    Fold
};

class ActionPossible {
    public:
        bool isAllow = false;
        Action action;
};


class PlayerContext {
    public:
        std::array<Card, Config::CARDS_PER_PLAYER> cards{};
        std::array<ActionPossible, 5> actionsPossible 
        {
            ActionPossible{true, Action::Check},ActionPossible{true, Action::Raise},ActionPossible{true, Action::AllIn},ActionPossible{true, Action::Call},ActionPossible{true, Action::Fold},
        };
        uint32_t money = 0;
        uint32_t playerBet = 0;
        uint32_t minRaiseAmount = 0;
        GameContext gameContext;
};

#endif