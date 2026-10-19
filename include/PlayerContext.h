#ifndef PLAYERCONTEXT_H
#define PLAYERCONTEXT_H

#include "GameContext.h"
#include <cstdint>
#include <vector>
#include <array>
#include <ostream>

enum class Action : uint8_t {
    Check,
    Raise,
    AllIn,
    Call,
    Fold
};

inline std::ostream& operator<<(std::ostream& os, const Action& act) {
    switch (act) {
        case Action::Check: os << "Check"; break;
        case Action::Raise: os << "Raise"; break;
        case Action::AllIn: os << "AllIn"; break;
        case Action::Call:  os << "Call";  break;
        case Action::Fold:  os << "Fold";  break;
        default:            os << "Unknown"; break;
    }
    return os;
}

class ActionPossible {
    public:
        bool isAllow = false;
        Action action;
};

inline std::ostream& operator<<(std::ostream& os, const ActionPossible& act) {
    os << "action : " << act.action << " autorisé : " << (act.isAllow ? "true" : "false");
    return os;
}

class PlayerContext {
    public:
        std::array<Card, Config::CARDS_PER_PLAYER> cards{};
        std::vector<ActionPossible> actionsPossible 
        {
            ActionPossible{true, Action::Fold},
            ActionPossible{true, Action::Check},
            ActionPossible{true, Action::Call},
            ActionPossible{true, Action::Raise},
            ActionPossible{true, Action::AllIn}
        };
        uint32_t money = 0;
        uint32_t playerBet = 0;
        uint32_t minRaiseAmount = 0;
        GameContext gameContext;
};

inline std::ostream& operator<<(std::ostream& os, const PlayerContext& ctx) {
    os << "PlayerContext { money: " << ctx.money 
       << ", currentBet: " << ctx.gameContext.currentBet
       << ", playerBet: " << ctx.playerBet 
       << ", minRaise: " << ctx.minRaiseAmount 
       << ", actions: [";
    
    for (size_t i = 0; i < ctx.actionsPossible.size(); ++i) {
        os << (ctx.actionsPossible[i]);
        if (i + 1 < ctx.actionsPossible.size()) os << ", ";
    }
    os << "]}";
    
    return os;
}

#endif