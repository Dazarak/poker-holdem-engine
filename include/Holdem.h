#ifndef HOLDEM_H
#define HOLDEM_H

#include "Engine.h"
#include "Config.h"
#include "HandRank.h"
#include "Player.h"
#include "PlayerContext.h"

#include <iostream>
#include <algorithm> 
#include <random>
#include <array>

struct PlayerAction {
    uint8_t actionIndex; // 0: Check, 1: Raise, 2: AllIn, 3: Call, 4: Fold
    uint32_t amount{0};  // Montant TOTAL de la mise visée (ex: 501 pour un Raise à 501)
};


class Holdem
{
    public:

        Holdem(uint8_t nbPlayer = 10, int startMoney = 1000, uint32_t smallBlind = 50);
        PlayerContext GetPlayerTurnAndPossibleAction();
        void ExecutePossibleAction(PlayerAction action);
        uint8_t GetCurrentTurn();
        void AdvanceToNextPlayer();
        bool PartyEnded = false;

    private:
        GameContext Context;
        Engine pokerEngine;
        std::array<Player, Config::MAX_PLAYERS> Players {};
        void givePlayersScore();
        bool CanPlayerAct(const Player& plr) const noexcept;
        uint8_t GetActivePlayersCount() const noexcept;
        bool IsStreetComplete() const noexcept;
        uint8_t GetUnfoldedPlayersCount() const noexcept;
        void DistributePot();
        void AdvanceStreet();
        uint8_t GetNextActivePlayer(uint8_t startSeat);
        void RestartGame();
}; 

#endif