#ifndef HAND_H
#define HAND_H

#include <vector>
#include "Card.h"

class Hand {
public:
    std::vector<Card> cards;
    int total;
    bool soft;

    Hand();

    void addCard(const Card& card);
    void reset();
};

#endif