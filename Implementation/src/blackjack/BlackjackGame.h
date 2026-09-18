#ifndef BLACKJACKGAME_H
#define BLACKJACKGAME_H

#include "Hand.h"
#include "Card.h"
enum class GameResult {
    PlayerWins,
    DealerWins,
    PlayerBust,
    DealerBust,
    Tie
};
class BlackjackGame {
private:
    bool gameOver = false;
    GameResult result = GameResult::Tie;
public:
    Hand playerHand;
    Hand dealerHand;

    Rank dealerUpcard;

    BlackjackGame();

    void startGame();
    void resetGame();
    Card hit();          // random card
    void playerHit();
    GameResult playerStand();
    bool isGameDone() const;
    GameResult getResult() const;
    bool playerBust() const;
};

#endif
