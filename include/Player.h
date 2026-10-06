#ifndef PLAYER_H
#define PLAYER_H

#include <cstdint>
#include <array>
#include "Card.h"
#include "HandRank.h"
#include "Config.h"

class Player {
    public:
        int id;
        std::array<Card, Config::CARDS_PER_PLAYER> cards{};
        std::array<Card, Config::CARDS_PER_PLAYER> activecards{};
        HandRank handrank = EMPTY;
        uint8_t rank = 0;
        uint32_t money = 0;
        uint32_t currentBet = 0;
        uint32_t totalBet = 0;
        bool isFolded = false;
        bool isAllIn = false;
        bool hasActed = false;
        bool isPlayer = false;

        Player();
        Player(int, int, std::array<Card, Config::CARDS_PER_PLAYER>);

        std::string_view GetHandRank() const;
};

#endif 