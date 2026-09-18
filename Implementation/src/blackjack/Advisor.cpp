#include "Advisor.h"
#include "Hand.h"
#include "Card.h"
#include <array>
#include <string>
#include <iostream>
#include <unordered_map>
#include <utility>

// 13 ranks, each 1/13 in infinite deck when iterating by rank
static const std::array<Rank, 13> ALL_RANKS = {
    Rank::Two, Rank::Three, Rank::Four, Rank::Five, Rank::Six, Rank::Seven,
    Rank::Eight, Rank::Nine, Rank::Ten, Rank::Jack, Rank::Queen, Rank::King, Rank::Ace
};

// Dealer distribution: P(bust), P(17..21) 

struct DealerDist {
    double bust = 0.0;
    double p17 = 0.0;
    double p18 = 0.0;
    double p19 = 0.0;
    double p20 = 0.0;
    double p21 = 0.0;

    void addScaled(const DealerDist& other, double scale) {
        bust += other.bust * scale;
        p17 += other.p17 * scale;
        p18 += other.p18 * scale;
        p19 += other.p19 * scale;
        p20 += other.p20 * scale;
        p21 += other.p21 * scale;
    }
};

// memo key for dealer recursion
static std::unordered_map<int, DealerDist> memoDealer;

static int dealerKey(int total, bool soft) {
    return total * 2 + (soft ? 1 : 0);
}

static DealerDist dealerDistFromState(int total, bool soft) {
    // dealer stands on all 17+ (including soft 17)
    if (total > 21) {
        DealerDist d; d.bust = 1.0; return d;
    }
    if (total >= 17) {
        DealerDist d;
        if (total == 17) d.p17 = 1.0;
        else if (total == 18) d.p18 = 1.0;
        else if (total == 19) d.p19 = 1.0;
        else if (total == 20) d.p20 = 1.0;
        else if (total == 21) d.p21 = 1.0;
        else { // >21 already handled, so shouldn't happen
            d.bust = 1.0;
        }
        return d;
    }

    int k = dealerKey(total, soft);
    auto it = memoDealer.find(k);
    if (it != memoDealer.end()) return it->second;

    DealerDist out;
    for (Rank r : ALL_RANKS) {
        Hand h;
        h.total = total;
        h.soft = soft;
        h.addCard(Card(r));

        DealerDist next = dealerDistFromState(h.total, h.soft);
        out.addScaled(next, Card::prob(r)); // 1/13 each rank
    }

    memoDealer[k] = out;
    return out;
}

static DealerDist dealerDistFromUpcard(Rank upcard) {
    // unknown hole card
    DealerDist out;
    for (Rank hole : ALL_RANKS) {
        Hand h;
        h.addCard(Card(upcard));
        h.addCard(Card(hole));

        DealerDist d = dealerDistFromState(h.total, h.soft);
        out.addScaled(d, Card::prob(hole)); // 1/13 each rank
    }
    return out;
}

// Player EV (Expectimax: max over actions, chance over cards)

// memo key for player recursion: (total, soft, dealerUp)
static std::unordered_map<int, double> memoPlayer;

static int playerKey(int total, bool soft, Rank dealerUp) {
    // dealerUp is 0..12 (enum order), pack into int
    return (total * 2 + (soft ? 1 : 0)) * 20 + static_cast<int>(dealerUp);
}

static double EVstand_internal(int playerTotal, Rank dealerUp) {
    DealerDist d = dealerDistFromUpcard(dealerUp);

    // payoff: win +1, lose -1, push 0
    double ev = 0.0;

    // dealer bust -> player wins
    ev += d.bust * 1.0;

    auto evalDealerTotal = [&](int dealerTotal, double p) {
        if (dealerTotal > 21) { ev += p * 1.0; return; }
        if (dealerTotal < playerTotal) ev += p * 1.0;
        else if (dealerTotal > playerTotal) ev += p * -1.0;
        else ev += 0.0; // push
        };

    evalDealerTotal(17, d.p17);
    evalDealerTotal(18, d.p18);
    evalDealerTotal(19, d.p19);
    evalDealerTotal(20, d.p20);
    evalDealerTotal(21, d.p21);

    return ev;
}

static double PlayerEV(int playerTotal, bool playerSoft, Rank dealerUp) {
    if (playerTotal > 21) return -1.0;

    int k = playerKey(playerTotal, playerSoft, dealerUp);
    auto it = memoPlayer.find(k);
    if (it != memoPlayer.end()) return it->second;

    double standEV = EVstand_internal(playerTotal, dealerUp);

    // HIT: chance over next rank
    double hitEV = 0.0;
    for (Rank r : ALL_RANKS) {
        Hand h;
        h.total = playerTotal;
        h.soft = playerSoft;
        h.addCard(Card(r));

        double branch = (h.total > 21) ? -1.0 : PlayerEV(h.total, h.soft, dealerUp);
        hitEV += Card::prob(r) * branch;
    }

    double best = (hitEV > standEV) ? hitEV : standEV;
    memoPlayer[k] = best;
    return best;
}

// Public API

double Advisor::EVstand(int playerTotal, bool /*playerSoft*/, Rank dealerUp) {
    return EVstand_internal(playerTotal, dealerUp);
}

double Advisor::EVhit(int playerTotal, bool playerSoft, Rank dealerUp) {
    // Expected value if you choose HIT now (and then play optimally afterwards)
    if (playerTotal > 21) return -1.0;

    double hitEV = 0.0;
    for (Rank r : ALL_RANKS) {
        Hand h;
        h.total = playerTotal;
        h.soft = playerSoft;
        h.addCard(Card(r));

        double branch = (h.total > 21) ? -1.0 : PlayerEV(h.total, h.soft, dealerUp);
        hitEV += Card::prob(r) * branch;
    }
    return hitEV;
}

std::pair<bool, std::string> Advisor::shouldHit(int playerTotal, bool playerSoft, Rank dealerUp) {
    double standEV = EVstand_internal(playerTotal, dealerUp);
    double hitEV = EVhit(playerTotal, playerSoft, dealerUp);

    bool hit = hitEV > standEV;

    std::string message;
    message += "Expected Value (STAND) = " + std::to_string(standEV) + "\n";
    message += "Expected Value (HIT) = " + std::to_string(hitEV) + "\n";
    message += "I suggest you to: ";
    message += hit ? "hit!" : "stand!";

    return { hit, message };
}