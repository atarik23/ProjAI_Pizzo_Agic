#include "BlackjackGame.h"
#include <random>
#include <array>

// all ranks
static const std::array<Rank, 13> ALL_RANKS = {
    Rank::Two, Rank::Three, Rank::Four, Rank::Five, Rank::Six, Rank::Seven,
    Rank::Eight, Rank::Nine, Rank::Ten, Rank::Jack, Rank::Queen, Rank::King, Rank::Ace
};

static Rank randomRank() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<int> dist(0, 12);

    return ALL_RANKS[dist(gen)];
}

BlackjackGame::BlackjackGame() {}

void BlackjackGame::startGame() {
    gameOver = false;
    result = GameResult::Tie;
    playerHand = Hand();
    dealerHand = Hand();

    // starting cards
    Card p1(randomRank());
    Card p2(randomRank());

    Card d1(randomRank()); // upcard
    Card d2(randomRank()); // hole card

    playerHand.addCard(p1);
    playerHand.addCard(p2);

    dealerHand.addCard(d1);
    dealerHand.addCard(d2);

    dealerUpcard = d1.rank;
}

Card BlackjackGame::hit() {
    return Card(randomRank());
}

void BlackjackGame::playerHit() {
    Card c = hit();
    playerHand.addCard(c);

    if (playerHand.total > 21) {
        result = GameResult::PlayerBust;
        gameOver = true;
    }
}
bool BlackjackGame::isGameDone() const {
    return gameOver;
}
GameResult BlackjackGame::getResult() const {
    return result;
}
GameResult BlackjackGame::playerStand() {

    while (dealerHand.total < 17) {
        dealerHand.addCard(hit());
    }

    if (dealerHand.total > 21)
        result = GameResult::DealerBust;
    else if (playerHand.total > 21)
        result = GameResult::PlayerBust;
    else if (playerHand.total > dealerHand.total)
        result = GameResult::PlayerWins;
    else if (dealerHand.total > playerHand.total)
        result = GameResult::DealerWins;
    else
        result = GameResult::Tie;

    gameOver = true;
    return result;
}
bool BlackjackGame::playerBust() const {
    return playerHand.total > 21;
}
void BlackjackGame::resetGame()
{
    // reset state
    gameOver = false;
    result = GameResult::Tie;

    // reset hands + redeal
    startGame();
}
