#include "Player.h"

Player::Player()
{
    id = 0;
    cards = { Card(), Card() };
    isPlayer = false;
}

Player::Player(int _id, int _money, std::array<Card, Config::CARDS_PER_PLAYER> _cards)
{
    id = _id;
    money = _money;
    cards = _cards;
    isPlayer = true;
}

std::string_view Player::GetHandRank() const {
    return HandRankToString(this->handrank);
}