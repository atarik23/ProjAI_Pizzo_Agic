#include "Card.h"
#include <random>

static int randomInt(int min, int max) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(min, max);
    return dist(gen);
}
Card::Card(Rank r) : rank(r) {
    suit = static_cast<Suit>(randomInt(0, 3));
}

int Card::value() const {
    switch (rank) {
    case Rank::Two:   return 2;
    case Rank::Three: return 3;
    case Rank::Four:  return 4;
    case Rank::Five:  return 5;
    case Rank::Six:   return 6;
    case Rank::Seven: return 7;
    case Rank::Eight: return 8;
    case Rank::Nine:  return 9;

    case Rank::Ten:
    case Rank::Jack:
    case Rank::Queen:
    case Rank::King:
        return 10;

    case Rank::Ace:
        return 11;
    }
    return 0;
}

double Card::prob(Rank /*rank*/) {
    // Infinite deck:
    // Ten/J/Q/K share 4/13 total when collapsed to 10-value
    // But since we're iterating each rank individually, each of Ten/J/Q/K is 1/13
    return 1.0 / 13.0;
}
