#include "Hand.h"

Hand::Hand() {
    reset();
}

void Hand::reset() {
    cards.clear();
    total = 0;
    soft = false;
}

void Hand::addCard(const Card& card) {
    cards.push_back(card);

    const int value = card.value();

    if (value == 11) {
        if (soft) {
            // Only one Ace can count as 11 at a time.
            total += 1;
        }
        else {
            total += 11;
            if (total > 21) {
                total -= 10;
            }
            else {
                soft = true;
            }
        }
    }
    else {
        total += value;
    }

    if (total > 21 && soft) {
        total -= 10;
        soft = false;
    }
}
