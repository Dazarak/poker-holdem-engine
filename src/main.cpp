#include <iostream>
#include <vector>
#include <random>
#include "Holdem.h"

int main() {
    Holdem game(10, 1000, 10);
    
    std::random_device rd;
    std::mt19937 gen(rd());

    while (!game.PartyEnded) {

        PlayerContext plcntx = game.GetPlayerTurnInformations();
        
        if (plcntx.actionsPossible[0].isAllow)
        {
            game.ExecutePossibleAction(PlayerAction{.actionIndex = 0, .amount = 0});
        }else{
            game.ExecutePossibleAction(PlayerAction{.actionIndex = 3, .amount = 0});
        }
        
        game.AdvanceToNextPlayer();

        if (game.HandEnded && !game.PartyEnded)
        {
            game.RestartGame();
        }
    }
    std::cout << "WINNER : " << static_cast<int>(game.GetWinner()) << std::endl;

    return 0;
}