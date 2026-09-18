#ifndef CARD_H
#define CARD_H

enum class Rank {
    Two,
    Three,
    Four,
    Five,
    Six,
    Seven,
    Eight,
    Nine,
    Ten,
    Jack,
    Queen,
    King,
    Ace
};
enum class Suit {
    Clubs = 0,
    Diamonds = 1,
    Hearts=2,
    Spades=3
};
class Card {
public:
    Rank rank;
    Suit suit;
    Card(Rank r);

    int value() const;               // blackjack value: 2..10 or 11 for Ace
    static double prob(Rank r);      // infinite-deck prob
};

#endif