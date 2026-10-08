#include "Holdem.h"

Holdem::Holdem(uint8_t nbPlayer, int startMoney, uint32_t smallBlind)
{
    int nbCardsPerPlayers = 2;
    // Crée le jeu de cartes
    std::array<Card, 52> allCards{};
    int k = 0;
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 13; j++)
        {
            allCards[k] = Card(j, i, true);
            k++;
        }
    }

    // Mélange le paquet
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(allCards.begin(), allCards.end(), g);
    
    size_t deckIndex = 0;
    for (uint8_t i = 0; i < nbPlayer; i++)
    {
        std::array<Card, Config::CARDS_PER_PLAYER> plrCards{}; // Tableau initialisé par défaut (isCard = false)
        
        for (int c = 0; c < nbCardsPerPlayers; c++)
        {
            plrCards[c] = allCards[deckIndex++];
        }

        Players[i] = Player(i, startMoney, plrCards);
    }

    for (int i = 0; i < Config::COMMUNITY_CARDS; i++)
    {
        Context.setCommunnityCards(allCards[deckIndex++], i);
    }

    Context.setVisibleCommunityCards(GameStep::PreFlop);

    Context.smallBlind = smallBlind;
    Context.bigBlind = smallBlind * 2;
    Context.nbPlayer = nbPlayer;
    Context.nbPlayerInGame = nbPlayer;
    Context.currentTurn = 0;       // Siège du joueur actif (0 à 9)
    Context.dealerPos = 0;         // Siège du bouton
    Context.currentBet = 0;       // La mise globale à égaler
    Context.pot = 0;              // Pot total
    Context.currentStep = GameStep::PreFlop;
}

void Holdem::RestartGame()
{
    // Élimination des joueurs ruinés et comptage des joueurs actifs
    uint8_t activePlayersCount = 0;
    for (auto& plr : Players) {
        if (plr.isPlayer) {
            if (plr.money <= 0) {
                plr.isPlayer = false; // Le joueur est éliminé du tournoi/table
                std::cout << "[DEBUG] Player " << static_cast<int>(plr.id) 
                      << " eliminated! money: " << plr.money << std::endl;
            } else {
                activePlayersCount++;
            }
        }
    }

    Context.nbPlayerInGame = activePlayersCount;
    std::cout << "nombre de joueur actif : " << static_cast<int>(activePlayersCount) << std::endl;

    if (activePlayersCount <= 1) {
        // Fin de partie
        PartyEnded = true;
        return;
    }

    // Déplacer le Bouton (dealerPos) vers le prochain joueur actif
    do {
        Context.dealerPos = (Context.dealerPos + 1) % Context.nbPlayer;
    } while (!Players[Context.dealerPos].isPlayer);

    // Réinitialiser le deck et mélanger 52 cartes
    std::array<Card, 52> allCards{};
    int k = 0;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 13; j++) {
            allCards[k++] = Card(j, i, true);
        }
    }
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(allCards.begin(), allCards.end(), g);

    size_t deckIndex = 0;

    // Reset complet de l'état de chaque joueur et distribution des nouvelles cartes
    for (uint8_t i = 0; i < Config::MAX_PLAYERS; ++i) {
        if (Players[i].isPlayer) {
            Players[i].isFolded = false;
            Players[i].isAllIn = false;
            Players[i].hasActed = false;
            Players[i].currentBet = 0;
            Players[i].totalBet = 0;
            Players[i].rank = 0;
            
            // Distribuer 2 nouvelles cartes
            Players[i].cards[0] = allCards[deckIndex++];
            Players[i].cards[1] = allCards[deckIndex++];
        }
    }

    // Générer les 5 cartes communes
    for (int i = 0; i < Config::COMMUNITY_CARDS; ++i) {
        Context.setCommunnityCards(allCards[deckIndex++], i);
    }

    // Reset du contexte global
    Context.currentStep = GameStep::PreFlop;
    Context.setVisibleCommunityCards(GameStep::PreFlop);
    Context.pot = 0;
    Context.currentBet = 0;
    Context.lastBet = 0;

    // Prélever Small Blind et Big Blind
    uint8_t sbPos, bbPos;

    // Cas particulier : HU (Heads-Up / 2 joueurs)
    if (Context.nbPlayerInGame == 2) {
        sbPos = Context.dealerPos;                  // Le Dealer est Small Blind
        bbPos = GetNextActivePlayer(Context.dealerPos); // L'autre est Big Blind
        Context.currentTurn = sbPos;                // En HU PreFlop, le SB parle en premier
    } else {
        sbPos = GetNextActivePlayer(Context.dealerPos);
        bbPos = GetNextActivePlayer(sbPos);
        Context.currentTurn = GetNextActivePlayer(bbPos); // UTG parle en premier PreFlop
    }

    // Prélèvement Small Blind (gestion de l'All-In automatique si tapis < blind)
    uint32_t sbAmount = std::min(Players[sbPos].money, Context.smallBlind);
    Players[sbPos].money -= sbAmount;
    Players[sbPos].currentBet = sbAmount;
    Players[sbPos].totalBet = sbAmount;
    Context.pot += sbAmount;
    if (Players[sbPos].money == 0) Players[sbPos].isAllIn = true;

    // Prélèvement Big Blind
    uint32_t bbAmount = std::min(Players[bbPos].money, Context.bigBlind);
    Players[bbPos].money -= bbAmount;
    Players[bbPos].currentBet = bbAmount;
    Players[bbPos].totalBet = bbAmount;
    Context.pot += bbAmount;
    if (Players[bbPos].money == 0) Players[bbPos].isAllIn = true;

    // Initialiser les paris du tour
    Context.currentBet = Context.bigBlind;
    Context.lastBet = Context.bigBlind;
}

// Fonction membre ou lambda interne pour trouver le siège du joueur actif suivant
uint8_t Holdem::GetNextActivePlayer(uint8_t startSeat)
{
    uint8_t seat = (startSeat + 1) % Context.nbPlayer;
    while (!Players[seat].isPlayer) {
        seat = (seat + 1) % Context.nbPlayer;
    }
    return seat;
}

PlayerContext Holdem::GetPlayerTurnAndPossibleAction()
{
    uint8_t pIdx = Context.currentTurn;
    PlayerContext plrCntxt;

    for (auto& aP : plrCntxt.actionsPossible)
    {
        aP.isAllow = false;
    }
    
    plrCntxt.gameContext = Context;
    plrCntxt.cards = Players[pIdx].cards;
    plrCntxt.money = Players[pIdx].money;
    plrCntxt.playerBet = Players[pIdx].currentBet;
    plrCntxt.minRaiseAmount = Context.minBet() - Players[pIdx].currentBet;

    uint32_t totalFunds = Players[pIdx].money + Players[pIdx].currentBet;

    // Check
    if (Context.currentBet == Players[pIdx].currentBet)
    {
        plrCntxt.actionsPossible[0].isAllow = true;
    }

    // Call
    if (totalFunds >= Context.currentBet && Context.currentBet > Players[pIdx].currentBet)
    {
        plrCntxt.actionsPossible[3].isAllow = true;
    }

    // Raise (appel direct de la méthode membre calculée)
    if (totalFunds >= Context.minBet())
    {
        plrCntxt.actionsPossible[1].isAllow = true;
    }

    // All-In
    if (Players[pIdx].money > 0)
    {
        plrCntxt.actionsPossible[2].isAllow = true;
    }

    // Fold
    plrCntxt.actionsPossible[4].isAllow = true;

    return plrCntxt;
}

void Holdem::ExecutePossibleAction(PlayerAction action)
{
    uint8_t pIdx = Context.currentTurn;
    Player& plr = Players[pIdx];

    plr.hasActed = true;

    switch (action.actionIndex)
    {
    case 0: // Check
        break;

    case 3: // Call
    {
        uint32_t needed = Context.currentBet - plr.currentBet;        
        if (needed >= plr.money)
        {
            action.actionIndex = 2; // Bascule en All-In
        }
        else
        {
            plr.money -= needed;
            plr.currentBet += needed;
            plr.totalBet += needed;
            Context.pot += needed;
            break;
        }
    }
    [[fallthrough]];

    case 1: // Raise
    {
        uint32_t minNeeded = Context.minBet() - plr.currentBet;
        if (action.amount < minNeeded) {
            action.amount = minNeeded;
        }

        // Si la relance consomme tout le tapis, c'est un All-In
        if (action.amount >= plr.money)
        {
            action.actionIndex = 2; // Bascule en All-In
        }
        else
        {
            plr.money -= action.amount;
            plr.currentBet += action.amount;
            plr.totalBet += action.amount;
            Context.pot += action.amount;

            Context.lastBet = Context.currentBet;
            Context.currentBet = plr.currentBet;
            break;
        }
    }
    [[fallthrough]];

    case 2: // AllIn
    {
        uint32_t allInAmount = plr.money;
        plr.currentBet += allInAmount;
        plr.totalBet += allInAmount;
        Context.pot += allInAmount;
        plr.money = 0;
        plr.isAllIn = true;

        if (plr.currentBet > Context.currentBet)
        {
            uint32_t currentDelta = Context.currentBet - Context.lastBet;
            if (currentDelta < Context.bigBlind) {
                currentDelta = Context.bigBlind;
            }

            uint32_t raiseIncrement = plr.currentBet - Context.currentBet;

            if (raiseIncrement >= currentDelta)
            {
                Context.lastBet = Context.currentBet;
                Context.currentBet = plr.currentBet;
            }
            else
            {
                Context.currentBet = plr.currentBet;
                Context.lastBet = Context.currentBet - currentDelta;
            }
        }
        break;
    }

    case 4: // Fold
        plr.isFolded = true;
        break;
    }
}

uint8_t Holdem::GetCurrentTurn()
{
    return Context.currentTurn;
}

bool Holdem::CanPlayerAct(const Player& plr) const noexcept
{
    return !plr.isFolded && !plr.isAllIn;
}

uint8_t Holdem::GetActivePlayersCount() const noexcept
{
    uint8_t count = 0;
    for (const auto& plr : Players) {
        if (CanPlayerAct(plr)) {
            count++;
        }
    }
    return count;
}

// Vérifie si le tour de mise actuel est terminé
bool Holdem::IsStreetComplete() const noexcept
{
    for (const auto& plr : Players) {
        if (plr.isFolded) continue;

        // Si le joueur peut jouer mais n'a pas encore égalisé la mise maximale
        if (CanPlayerAct(plr) && (plr.currentBet < Context.currentBet || !plr.hasActed)) {
            return false;
        }
    }
    return true;
}

void Holdem::AdvanceToNextPlayer()
{
    // Si le tour de mise est fini, on passe au tour suivant (Flop/Turn/River/Showdown)
    if (IsStreetComplete())
    {
        AdvanceStreet();
        return;
    }

    // Recherche du prochain joueur actif (dans le sens horaire)
    uint8_t nextIdx = Context.currentTurn;
    do {
        nextIdx = (nextIdx + 1) % Players.size();
    } while (!CanPlayerAct(Players[nextIdx]) && nextIdx != Context.currentTurn);

    Context.currentTurn = nextIdx;
}

uint8_t Holdem::GetUnfoldedPlayersCount() const noexcept
{
    uint8_t unfoldedPlayer = 0;
    for (const auto& plr : Players) 
        if (!plr.isFolded) unfoldedPlayer++;
    return unfoldedPlayer;
}

void Holdem::DistributePot()
{
    while (Context.pot > 0)
    {
        // Trouver le plus petit totalBet non nul
        uint32_t minContrib = UINT32_MAX;
        for (const auto& plr : Players) {
            if (plr.totalBet > 0 && plr.totalBet < minContrib) {
                minContrib = plr.totalBet;
            }
        }

        if (minContrib == UINT32_MAX || minContrib == 0) break;

        // Prélever la tranche et repérer le meilleur score éligible pour cette tranche
        uint32_t subPot = 0;
        uint8_t bestScoreInSubPot = UINT8_MAX; // Plus le chiffre est petit, meilleure est la main

        for (uint8_t i = 0; i < Context.nbPlayer; ++i) {
            if (Players[i].totalBet > 0) {
                uint32_t amount = std::min(Players[i].totalBet, minContrib);
                subPot += amount;
                Players[i].totalBet -= amount;

                // Si le joueur n'a pas fold et a un score valide
                if (Players[i].isPlayer && !Players[i].isFolded && Players[i].rank > 0) {
                    if (Players[i].rank < bestScoreInSubPot) {
                        bestScoreInSubPot = Players[i].rank;
                    }
                }
            }
        }

        // Trouver tous les joueurs qui possèdent ce meilleur score pour cette tranche
        std::vector<uint8_t> winners;
        if (bestScoreInSubPot != UINT8_MAX) {
            for (uint8_t i = 0; i < Context.nbPlayer; ++i) {
                // On vérifie qu'il avait bien participé à ce subPot (totalBet diminué au-dessus)
                if (Players[i].isPlayer && !Players[i].isFolded && Players[i].rank == bestScoreInSubPot) {
                    winners.push_back(i);
                }
            }
        }

        // Distribuer la tranche
        if (!winners.empty()) {
            uint32_t share = subPot / winners.size();
            uint32_t remainder = subPot % winners.size(); // Le reste indivisible

            // On donne la part égale à tout le monde
            for (uint8_t idx : winners) {
                Players[idx].money += share;
            }

            // On distribue les jetons restants (1 par 1) selon la position par rapport au dealer
            if (remainder > 0) {
                // Parcourir les joueurs à partir de Small Blind (dealer + 1)
                for (uint8_t i = 0; i < Context.nbPlayer && remainder > 0; ++i) {
                    uint8_t pos = (Context.dealerPos + 1 + i) % Context.nbPlayer;
                    
                    // Si ce joueur fait partie des gagnants de cette tranche
                    if (std::find(winners.begin(), winners.end(), pos) != winners.end()) {
                        Players[pos].money += 1;
                        remainder--;
                    }
                }
            }
        }

        // Déduire du pot global
        if (Context.pot >= subPot) {
            Context.pot -= subPot;
        } else {
            Context.pot = 0;
        }
    }
    std::cout << "============================================" << std::endl << std::endl;
    for (const auto& plr : Players) {
        std::cout << "player : " << plr.id << " money : " << static_cast<unsigned int>(plr.money) << "is already player : " << static_cast<bool>(plr.isPlayer) << std::endl ; 
    }
    Context.pot = 0;
    RestartGame();
}

void Holdem::AdvanceStreet()
{
    if (GetUnfoldedPlayersCount() == 1)
    {
        uint8_t plrIdx = 0;

        for (auto& plr : Players) 
            if (!plr.isFolded) {
                plr.rank = 1;
                break;
            }
        DistributePot(); 
        return;
    }
    

    // Réinitialisation des mises pour la nouvelle street
    Context.currentBet = 0;
    Context.lastBet = 0;

    for (auto& plr : Players) {
        plr.currentBet = 0;
        plr.hasActed = false;
    }

    
    // Évolution de l'état de la partie
    switch (Context.currentStep)
    {
    case GameStep::PreFlop:
        Context.currentStep = GameStep::Flop;
        // Dévoiler 3 cartes communes
        Context.setVisibleCommunityCards(GameStep::Flop);
        break;

    case GameStep::Flop:
        Context.currentStep = GameStep::Turn;
        // Dévoiler la 4ᵉ carte
        Context.setVisibleCommunityCards(GameStep::Turn);
        break;

    case GameStep::Turn:
        Context.currentStep = GameStep::River;
        // Dévoiler la 5ᵉ carte
        Context.setVisibleCommunityCards(GameStep::River);
        break;

    case GameStep::River:
        Context.currentStep = GameStep::Showdown;
        // Calcul des gagnants et distribution du pot
        givePlayersScore();
        DistributePot();
        return;
    }

    if (GetActivePlayersCount() <= 1)
    {
        AdvanceStreet(); // Appel récursif pour passer la street suivante
        return;
    }

    // Reprise de parole : premier joueur actif à gauche du Bouton (Dealer)
    uint8_t nextIdx = Context.dealerPos;
    do {
        nextIdx = (nextIdx + 1) % Players.size();
    } while (!CanPlayerAct(Players[nextIdx]));

    Context.currentTurn = nextIdx;
}

void Holdem::givePlayersScore()
{
    for (auto& plr : Players)
    {
        std::array<Card, Config::MAX_CARDS> allCards = {
            Context.visibleCommunCards[0],Context.visibleCommunCards[1],Context.visibleCommunCards[2],Context.visibleCommunCards[3],Context.visibleCommunCards[4],
            plr.cards[0], plr.cards[1], plr.cards[2], plr.cards[3], plr.cards[4]
        };
        HandResult HR = pokerEngine.EvaluatePlayersHands(allCards);
        plr.activecards = HR.activeCards;
        plr.handrank = HR.handRank;
    }

    // Réinitialiser les scores (0 = foldé / hors jeu)
    for (Player& plr : Players) {
        plr.rank = 0;
    }

    // Récupérer les index des joueurs non-foldés
    std::vector<uint8_t> activeIndices;
    for (uint8_t i = 0; i < Context.nbPlayer; ++i) {
        if (!Players[i].isFolded) {
            activeIndices.push_back(i);
        }
    }

    if (activeIndices.empty()) return;

    uint8_t currentRank = 1;

    // Tant qu'il reste des joueurs à classer
    while (!activeIndices.empty())
    {
        // Trouver la HandRank maximale parmi les joueurs restants
        HandRank maxRank = EMPTY;
        for (uint8_t idx : activeIndices) {
            if (Players[idx].handrank > maxRank) {
                maxRank = Players[idx].handrank;
            }
        }

        // Filtrer les candidats ayant cette HandRank
        std::vector<uint8_t> candidates;
        for (uint8_t idx : activeIndices) {
            if (Players[idx].handrank == maxRank) {
                candidates.push_back(idx);
            }
        }

        // Départager les candidats carte par carte (kickers)
        for (uint8_t cardIndex = 0; cardIndex < Config::CARDS_PER_PLAYER; ++cardIndex) {
            uint8_t maxValue = 0;
            for (uint8_t idx : candidates) {
                if (Players[idx].activecards[cardIndex].value > maxValue) {
                    maxValue = Players[idx].activecards[cardIndex].value;
                }
            }

            std::vector<uint8_t> winners;
            for (uint8_t idx : candidates) {
                if (Players[idx].activecards[cardIndex].value == maxValue) {
                    winners.push_back(idx);
                }
            }

            candidates = winners;
            if (candidates.size() == 1) break;
        }

        // 'candidates' contient le/les joueurs ayant la même meilleure main
        for (uint8_t idx : candidates) {
            Players[idx].rank = currentRank; // 1 = 1er, 2 = 2e, etc.
            
            // Retirer le joueur de activeIndices
            activeIndices.erase(
                std::remove(activeIndices.begin(), activeIndices.end(), idx),
                activeIndices.end()
            );
        }

        // Le rang suivant prend en compte le nombre de joueurs ex-æquo
        currentRank += candidates.size();
    }
}