#pragma once
#include <algorithm>

struct GameState
{
    // GLOBAL WALLET (shared between map and blackjack)
    inline static int Wallet = 0;

    // Betting
    inline static int CurrentBet = 50;

    static void AddMoney(int delta)
    {
        Wallet = std::max(0, Wallet + delta);
    }

    static bool CanAfford(int amount)
    {
        return Wallet >= amount;
    }

    static void ClampBet()
    {
        if (CurrentBet < 0) CurrentBet = 0;
        if (CurrentBet > Wallet) CurrentBet = Wallet;
    }
};