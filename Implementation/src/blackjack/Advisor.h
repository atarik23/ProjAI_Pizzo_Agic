#pragma once
#ifndef ADVISOR_H
#define ADVISOR_H

#include "Card.h"

#include <utility>   // for std::pair
#include <string>    // for std::string
class Advisor {
public:
    // EV for stand/hit, with infinite deck, dealer stands on all 17 (S17)
    static double EVstand(int playerTotal, bool playerSoft, Rank dealerUp);
    static double EVhit(int playerTotal, bool playerSoft, Rank dealerUp);

    static std::pair<bool, std::string>  shouldHit(int playerTotal, bool playerSoft, Rank dealerUp);
};

#endif