#include "GameContext.h"

uint32_t GameContext::minBet()
{
    uint32_t delta = (currentBet >= lastBet) ? (currentBet - lastBet) : 0;
    if (delta < bigBlind) {
        delta = bigBlind;
    }
    return currentBet + delta;
}

void GameContext::setCommunnityCards(Card card , uint8_t index)
{
    communCards[index] = card;
}

void GameContext::setVisibleCommunityCards(GameStep step)
{
    switch (step)
    {
    case GameStep::PreFlop:
        // Aucune carte visible
        visibleCommunCards.fill(Card{});
        break;

    case GameStep::Flop:
        // Révèle les 3 premières cartes (Flop)
        visibleCommunCards[0] = communCards[0];
        visibleCommunCards[1] = communCards[1];
        visibleCommunCards[2] = communCards[2];
        break;

    case GameStep::Turn:
        // Révèle la 4e carte (Turn)
        visibleCommunCards[3] = communCards[3];
        break;

    case GameStep::River:
    case GameStep::Showdown:
        // Révèle la 5e carte (River / Showdown)
        visibleCommunCards[4] = communCards[4];
        break;
    }
}