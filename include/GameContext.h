#ifndef GAMECONTEXT_H
#define GAMECONTEXT_H

#include "Config.h"
#include "Card.h"

#include <cstdint>
#include <array>

enum class GameStep : uint8_t {
    PreFlop,
    Flop,
    Turn,
    River,
    Showdown
};

class GameContext {
public:
    std::array<Card, Config::COMMUNITY_CARDS> visibleCommunCards{};
    uint8_t currentTurn;       // Siège du joueur actif (0 à 9)
    uint8_t dealerPos;         // Siège du bouton
    uint8_t nbPlayer;
    uint8_t nbPlayerInGame;
    uint32_t currentBet;       // La mise globale à égaler
    uint32_t lastBet;          // Mise qui précèdent la dernière mise
    uint32_t pot;              // Pot total
    uint32_t smallBlind;
    uint32_t bigBlind;
    
    GameStep currentStep = GameStep::PreFlop;
    uint32_t minBet();
    void setCommunnityCards(Card, uint8_t);
    void setVisibleCommunityCards(GameStep);
private:
    std::array<Card, Config::COMMUNITY_CARDS> communCards{};
};

#endif