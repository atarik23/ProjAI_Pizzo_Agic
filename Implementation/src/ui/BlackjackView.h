#pragma once
#include <gui/Canvas.h>
#include <gui/Shape.h>
#include <gui/DrawableString.h>
#include <gui/Font.h>
#include <gui/Sound.h>
#include <gui/Image.h>
#include <gui/Timer.h>

#include <map>
#include <string>
#include <algorithm>
#include <cmath>
#include <cstdlib>

#include "CommonTypes.h"
#include "../GameState.h"
#include "../blackjack/Advisor.h"
#include "../blackjack/BlackjackGame.h"

class BlackjackView : public gui::Canvas
{
protected:
    // Main buttons
    RectCordinates _hitBtn{ -1,-1,-1,-1 };
    RectCordinates _standBtn{ -1,-1,-1,-1 };
    RectCordinates _tryAgainBtn{ -1,-1,-1,-1 };
    RectCordinates _goBackBtn{ -1,-1,-1,-1 };
    RectCordinates _advisorBtn{ -1,-1,-1,-1 };

    // Top-left back button
    RectCordinates _topBackBtn{ -1,-1,-1,-1 };

    // betting buttons
    RectCordinates _betMinusBtn{ -1,-1,-1,-1 };
    RectCordinates _betPlusBtn{ -1,-1,-1,-1 };

    bool _showAdvisorTip = false;

    // Sounds
    gui::Sound _sfxWin;
    gui::Sound _sfxTryAgain;

    // Win sound
    bool _winSfxPlayed = false;

    // payout guard
    bool _payoutApplied = false;

    // Dealer hidden state
    bool showDealerUpCard = false;

    // flip animation for dealer hidden card (index 1)
    gui::Timer _flipTimer;
    bool _flipAnimating = false;
    float _flipT = 0.0f;

    //counting HUD animation (wallet/bet)
    gui::Timer _hudTimer;
    int _walletDisplay = 0;
    int _walletTarget = 0;

    int _betDisplay = 0;
    int _betTarget = 0;

    // Can user play (wallet>0)
    bool _canPlay = true;

    // round bet snapshot
    int _roundBet = 0;

    gui::Shape _uiShape;

    // Hover for buttons 
    enum class HoverID : int {
        None,
        Hit, Stand,
        BetMinus, BetPlus,
        TopBack,
        Advisor,
        TryAgain, GoBack
    };
    HoverID _hover = HoverID::None;

    std::map<Rank, std::vector<std::string>> cardImages = {
    { Rank::Two,   {":2_of_clubs",":2_of_diamonds",":2_of_hearts",":2_of_spades"} },
    { Rank::Three, {":3_of_clubs",":3_of_diamonds",":3_of_hearts",":3_of_spades"} },
    { Rank::Four,  {":4_of_clubs",":4_of_diamonds",":4_of_hearts",":4_of_spades"} },
    { Rank::Five,  {":5_of_clubs",":5_of_diamonds",":5_of_hearts",":5_of_spades"} },
    { Rank::Six,   {":6_of_clubs",":6_of_diamonds",":6_of_hearts",":6_of_spades"} },
    { Rank::Seven, {":7_of_clubs",":7_of_diamonds",":7_of_hearts",":7_of_spades"} },
    { Rank::Eight, {":8_of_clubs",":8_of_diamonds",":8_of_hearts",":8_of_spades"} },
    { Rank::Nine,  {":9_of_clubs",":9_of_diamonds",":9_of_hearts",":9_of_spades"} },
    { Rank::Ten,   {":10_of_clubs",":10_of_diamonds",":10_of_hearts",":10_of_spades"} },

    { Rank::Jack,  {":jack_of_clubs2",":jack_of_diamonds2",":jack_of_hearts2",":jack_of_spades2"} },
    { Rank::Queen, {":queen_of_clubs2",":queen_of_diamonds2",":queen_of_hearts2",":queen_of_spades2"} },
    { Rank::King,  {":king_of_clubs2",":king_of_diamonds2",":king_of_hearts2",":king_of_spades2"} },

    { Rank::Ace,   {":ace_of_clubs",":ace_of_diamonds",":ace_of_hearts",":ace_of_spades"} }
    };

    BlackjackGame game;

    // ---------- helpers ----------
    bool outOfMoney() const { return GameState::Wallet <= 0; }

    bool canInteract() const
    {
        if (outOfMoney()) return false;
        return _canPlay;
    }

    void closeWindow()
    {
        auto parentWnd = getParentWindow();
        if (parentWnd) parentWnd->close();
    }

    void syncHudNow()
    {
        _walletDisplay = GameState::Wallet;
        _walletTarget = GameState::Wallet;

        _betDisplay = GameState::CurrentBet;
        _betTarget = GameState::CurrentBet;
    }

    void animateHudToTargets()
    {
        _walletTarget = GameState::Wallet;
        _betTarget = GameState::CurrentBet;

        if (!_hudTimer.isRunning())
            _hudTimer.start();
    }

    static bool inRect(const gui::Point& p, const RectCordinates& r)
    {
        if (r.left == -1) return false;
        return p.x >= r.left && p.x <= r.right && p.y >= r.top && p.y <= r.bottom;
    }

    HoverID hitTestHover(const gui::Point& p) const
    {
        if (inRect(p, _topBackBtn)) return HoverID::TopBack;

        if (inRect(p, _tryAgainBtn)) return HoverID::TryAgain;
        if (inRect(p, _goBackBtn))   return HoverID::GoBack;

        if (inRect(p, _hitBtn))   return HoverID::Hit;
        if (inRect(p, _standBtn)) return HoverID::Stand;

        if (inRect(p, _betMinusBtn)) return HoverID::BetMinus;
        if (inRect(p, _betPlusBtn))  return HoverID::BetPlus;

        if (inRect(p, _advisorBtn)) return HoverID::Advisor;

        return HoverID::None;
    }

    void setHover(HoverID h)
    {
        if (_hover == h) return;
        _hover = h;
        reDraw();
    }

    void drawButton(gui::Shape& sh,
        const gui::Rect& r,
        const std::string& text,
        bool hovered,
        bool enabled,
        td::ColorID fillEnabled,
        td::ColorID fillDisabled,
        td::ColorID wireNormal,
        td::ColorID wireHover,
        td::ColorID textEnabled,
        td::ColorID textDisabled)
    {
        td::ColorID fill = enabled ? fillEnabled : fillDisabled;
        td::ColorID wire = hovered ? wireHover : wireNormal;
        td::ColorID tcol = enabled ? textEnabled : textDisabled;

        sh.createRoundedRect(r, 10);
        sh.drawFillAndWire(fill, wire, hovered ? 3 : 2);

        if (hovered && enabled) {
            gui::Rect inner(gui::Point(r.left + 6, r.top + 6),
                gui::Size(r.width() - 12, r.height() - 12));
            gui::Shape hi;
            hi.createRoundedRect(inner, 8);
            hi.drawWire(td::ColorID::Gray, 1);
        }

        gui::DrawableString s(text);
        s.draw(r, gui::Font::ID::SystemBold, tcol,
            td::TextAlignment::Center, td::VAlignment::Center);
    }

    void startRevealFlip()
    {
        showDealerUpCard = true;
        _flipAnimating = true;
        _flipT = 0.0f;
        _flipTimer.start();
    }

    void drawCardWithFlip(gui::Rect cardRect, const std::string& backPath, const std::string& facePath)
    {
        float t = _flipT;
        float factor = (t < 0.5f) ? (1.0f - 2.0f * t) : (2.0f * t - 1.0f);
        factor = std::max(0.02f, factor);

        int fullW = cardRect.width();
        int w = (int)std::round((float)fullW * factor);
        int x = cardRect.left + (fullW - w) / 2;

        gui::Rect r(gui::Point(x, cardRect.top), gui::Size(w, cardRect.height()));

        if (t < 0.5f) {
            gui::Image back(backPath);
            back.draw(r);
        }
        else {
            gui::Image face(facePath);
            face.draw(r);
        }
    }

public:
    BlackjackView()
        : Canvas({ gui::InputDevice::Event::PrimaryClicks, gui::InputDevice::Event::CursorMove })
        , _sfxWin(":win")
        , _sfxTryAgain(":tryagain")
        , _flipTimer(this, 0.016f, false)
        , _hudTimer(this, 0.016f, false)
    {
        _flipTimer.onTimer([this]() {
            if (!_flipAnimating) return;

            _flipT += 0.08f;
            if (_flipT >= 1.0f) {
                _flipT = 1.0f;
                _flipAnimating = false;
                _flipTimer.stop();
            }
            reDraw();
            });

        _hudTimer.onTimer([this]() {
            _walletTarget = GameState::Wallet;
            _betTarget = GameState::CurrentBet;

            bool changed = false;

            auto stepTowards = [&](int& value, int target) {
                if (value == target) return false;

                int diff = target - value;
                int ad = std::abs(diff);
                int step = std::max(1, ad / 10);

                value += (diff > 0) ? step : -step;

                if ((diff > 0 && value > target) || (diff < 0 && value < target))
                    value = target;

                return true;
                };

            changed |= stepTowards(_walletDisplay, _walletTarget);
            changed |= stepTowards(_betDisplay, _betTarget);

            if (!changed)
                _hudTimer.stop();

            reDraw();
            });

        startGame();
        syncHudNow();
    }

    void onCursorMoved(const gui::InputDevice& inputDevice) override
    {
        const gui::Point& p = inputDevice.getModelPoint();
        setHover(hitTestHover(p));
    }

    void startGame()
    {
        // reset per-round guards
        _winSfxPlayed = false;
        _payoutApplied = false;

        _canPlay = (GameState::Wallet > 0);

        if (!_canPlay) {
            GameState::CurrentBet = 0;
            _roundBet = 0;
            showDealerUpCard = false;
            _flipAnimating = false;
            _flipT = 0.0f;
            return;
        }

        GameState::ClampBet();
        _roundBet = GameState::CurrentBet;

        if (_roundBet > 0 && GameState::CanAfford(_roundBet)) {
            GameState::AddMoney(-_roundBet);
        }
        else {
            _roundBet = 0;
        }

        showDealerUpCard = false;
        _flipAnimating = false;
        _flipT = 0.0f;

        game.startGame();
        animateHudToTargets();
    }

    void applyPayoutIfNeeded()
    {
        if (!_payoutApplied && game.isGameDone())
        {
            _payoutApplied = true;

            GameResult r = game.getResult();

            if (_roundBet <= 0) {
                animateHudToTargets();
                return;
            }

            if (r == GameResult::PlayerWins || r == GameResult::DealerBust)
            {
                GameState::AddMoney(_roundBet * 2);
            }
            else if (r == GameResult::Tie)
            {
                GameState::AddMoney(_roundBet);
            }

            animateHudToTargets();
        }
    }

    void drawDealerCards(gui::Point dealersHandPt)
    {
        const auto& cards = game.dealerHand.cards;

        gui::Point pt = dealersHandPt;
        pt.x = dealersHandPt.x - 120;
        pt.y = dealersHandPt.y + 45;

        gui::Size sz(150, 150);
        gui::Rect dealerRect(pt, sz);

        for (size_t i = 0; i < cards.size(); i++)
        {
            const Card& card = cards[i];

            if (i == 1 && !showDealerUpCard)
            {
                gui::Image back(":back");
                back.draw(dealerRect);
            }
            else if (i == 1 && showDealerUpCard && _flipAnimating)
            {
                std::string facePath = cardImages[card.rank][static_cast<int>(card.suit)];
                drawCardWithFlip(dealerRect, ":back", facePath);
            }
            else
            {
                gui::Image img(cardImages[card.rank][static_cast<int>(card.suit)]);
                img.draw(dealerRect);
            }

            dealerRect.translate(sz.width - 10, 0);
        }
    }

    void drawPlayerCards(gui::Point dealersPlayerPt)
    {
        const auto& cards = game.playerHand.cards;

        gui::Point pt = dealersPlayerPt;
        pt.x = dealersPlayerPt.x - 120;
        pt.y = dealersPlayerPt.y + 45;

        gui::Size sz(150, 150);
        gui::Rect playerRect(pt, sz);

        gui::Image toniImg(":toni");
        toniImg.draw(playerRect);
        playerRect.translate(sz.width - 10, 0);

        for (size_t i = 0; i < cards.size(); i++)
        {
            const Card& card = cards[i];
            gui::Image img(cardImages[card.rank][static_cast<int>(card.suit)]);
            img.draw(playerRect);
            playerRect.translate(sz.width - 10, 0);
        }
    }

    void drawDealer(const gui::Rect& rect)
    {
        gui::Shape bg;

        gui::Size dealersHandSz(140, 30);
        gui::Point dealersHandPt((int)((rect.width() - rect.width() / 3.5) / 2 - dealersHandSz.width / 2), 70);
        gui::Rect dealersHandRect(dealersHandPt, dealersHandSz);

        bg.createRoundedRect(dealersHandRect, 5);
        bg.drawFillAndWire(td::ColorID::Black, td::ColorID::Gold, 1);

        gui::DrawableString dealerStr("DEALER'S HAND");
        dealerStr.draw(dealersHandRect, gui::Font::ID::SystemBold, td::ColorID::Gold,
            td::TextAlignment::Center, td::VAlignment::Center);

        drawDealerCards(dealersHandPt);

        gui::Size dealersTotalSz(120, 30);
        gui::Point dealersTotalPt(dealersHandPt.x + dealersHandSz.width / 2 - dealersTotalSz.width / 2,
            dealersHandPt.y + 160 + 60);
        gui::Rect dealersTotalRect(dealersTotalPt, dealersTotalSz);

        bg.createRoundedRect(dealersTotalRect, 5);
        bg.drawFillAndWire(td::ColorID::Black, td::ColorID::Gold, 1);

        std::string totalText = showDealerUpCard ? std::to_string(game.dealerHand.total) : "?";
        gui::DrawableString totalStr("TOTAL: " + totalText);
        totalStr.draw(dealersTotalRect, gui::Font::ID::SystemBold, td::ColorID::Gold,
            td::TextAlignment::Center, td::VAlignment::Center);
    }

    void drawPlayer(const gui::Rect& rect)
    {
        gui::Shape bg;

        gui::Size playerHandSz(120, 30);
        gui::Point playersHandPt((int)((rect.width() - rect.width() / 3.5) / 2 - playerHandSz.width / 2), 330);
        gui::Rect playerHandRect(playersHandPt, playerHandSz);

        bg.createRoundedRect(playerHandRect, 5);
        bg.drawFillAndWire(td::ColorID::Black, td::ColorID::Gold, 1);

        gui::DrawableString playerStr("TONY'S HAND");
        playerStr.draw(playerHandRect, gui::Font::ID::SystemBold, td::ColorID::Gold,
            td::TextAlignment::Center, td::VAlignment::Center);

        drawPlayerCards(playersHandPt);

        gui::Size playerTotalSz(120, 30);
        gui::Point playerTotalPt(playersHandPt.x + playerHandSz.width / 2 - playerTotalSz.width / 2,
            playersHandPt.y + 160 + 60);
        gui::Rect playerTotalRect(playerTotalPt, playerTotalSz);

        bg.createRoundedRect(playerTotalRect, 5);
        bg.drawFillAndWire(td::ColorID::Black, td::ColorID::Gold, 1);

        gui::DrawableString totalStr("TOTAL: " + std::to_string(game.playerHand.total));
        totalStr.draw(playerTotalRect, gui::Font::ID::SystemBold, td::ColorID::Gold,
            td::TextAlignment::Center, td::VAlignment::Center);
    }

    void drawWinner(const gui::Rect& rect)
    {
        // panel geometry
        const gui::Size boxSize(700, 420);
        const gui::Point boxPt(
            (int)rect.width() / 2 - boxSize.width / 2,
            (int)rect.height() / 2 - boxSize.height / 2
        );
        const gui::Rect boxRect(boxPt, boxSize);

        // result
        const GameResult r = game.getResult();

        //border color by outcome 
        td::ColorID borderColor = td::ColorID::Red; // lose default
        if (r == GameResult::PlayerWins || r == GameResult::DealerBust) borderColor = td::ColorID::Green;
        else if (r == GameResult::Tie) borderColor = td::ColorID::Yellow;

        // panel draw
        gui::Shape boxShape;
        boxShape.createRoundedRect(boxRect, 15);
        boxShape.drawFillAndWire(td::ColorID::Black, borderColor, 4);

        // title
        gui::Font titleFont;

#ifdef _WIN32

        titleFont.create("Roboto", 36.0f, gui::Font::Style::Bold, gui::Font::Unit::LogicalPixel);

#elif defined(_linux_)
        _fontExplanation.create("DejaVu Sans", 36.0f, gui::Font::Style::Bold, gui::Font::Unit::LogicalPixel);

#else
        titleFont.create("Helvetica", 36.0f, gui::Font::Style::Bold, gui::Font::Unit::LogicalPixel);

#endif

        std::string title;
        td::ColorID titleColor = td::ColorID::Red;

        if (r == GameResult::PlayerBust) { title = "YOU BUSTED!"; }
        else if (r == GameResult::DealerBust) { title = "DEALER BUSTED!"; titleColor = td::ColorID::Green; }
        else if (r == GameResult::PlayerWins) { title = "YOU WIN!";       titleColor = td::ColorID::Green; }
        else if (r == GameResult::DealerWins) { title = "YOU LOSE!"; }
        else { title = "TIE!";           titleColor = td::ColorID::Yellow; }

        const gui::Rect titleRect(gui::Point(boxPt.x, boxPt.y + 60), gui::Size(boxSize.width, 100));
        gui::DrawableString(title).draw(
            titleRect, &titleFont, titleColor,
            td::TextAlignment::Center, td::VAlignment::Center
        );

        // subtitle
        const bool win = (r == GameResult::PlayerWins || r == GameResult::DealerBust);
        const std::string subtitle = win
            ? "You're printin' your own currency now."
            : "Better luck next time boss!";

        const gui::Rect subRect(gui::Point(boxPt.x, boxPt.y + 140), gui::Size(boxSize.width, 50));
        gui::DrawableString(subtitle).draw(
            subRect, gui::Font::ID::SystemBold, td::ColorID::Gray,
            td::TextAlignment::Center, td::VAlignment::Center
        );

        // buttons
        const gui::Size btnSize(500, 70);

        // TRY AGAIN
        const gui::Rect tryRect(
            gui::Point((int)rect.width() / 2 - btnSize.width / 2, boxPt.y + 230),
            btnSize
        );
        _tryAgainBtn = RectCordinates(tryRect.top, tryRect.bottom, tryRect.left, tryRect.right);

        gui::Shape tryShape;
        drawButton(
            tryShape, tryRect, "TRY AGAIN",
            (_hover == HoverID::TryAgain),
            true,
            td::ColorID::DarkOrange, td::ColorID::DarkGray,
            td::ColorID::Gold, td::ColorID::Yellow,
            td::ColorID::Black, td::ColorID::Gray
        );

        // GO BACK
        const gui::Rect backRect(
            gui::Point((int)rect.width() / 2 - btnSize.width / 2, boxPt.y + 320),
            btnSize
        );
        _goBackBtn = RectCordinates(backRect.top, backRect.bottom, backRect.left, backRect.right);

        gui::Shape backShape;
        drawButton(
            backShape, backRect, "GO COLLECT THE PIZZO",
            (_hover == HoverID::GoBack),
            true,
            td::ColorID::DarkGray, td::ColorID::DarkGray,
            td::ColorID::Gold, td::ColorID::Yellow,
            td::ColorID::White, td::ColorID::Gray
        );
    }

    void drawNoMoneyPopup(const gui::Rect& rect)
    {
        // panel geometry
        const gui::Size boxSize(780, 360);
        const gui::Point boxPt(
            (int)rect.width() / 2 - boxSize.width / 2,
            (int)rect.height() / 2 - boxSize.height / 2
        );
        const gui::Rect boxRect(boxPt, boxSize);

        // panel draw
        gui::Shape boxShape;
        boxShape.createRoundedRect(boxRect, 15);
        boxShape.drawFillAndWire(td::ColorID::Black, td::ColorID::Gold, 4);

        // title
        gui::Font fontTitle;

#ifdef _WIN32

        fontTitle.create("Roboto", 28.0f, gui::Font::Style::Bold, gui::Font::Unit::LogicalPixel);

#elif defined(_linux_)
        _fontExplanation.create("DejaVu Sans", 28.0f, gui::Font::Style::Bold, gui::Font::Unit::LogicalPixel);

#else
        fontTitle.create("Helvetica", 28.0f, gui::Font::Style::Bold, gui::Font::Unit::LogicalPixel);

#endif


        const gui::Rect titleRect(gui::Point(boxPt.x, boxPt.y + 55), gui::Size(boxSize.width, 70));
        gui::DrawableString("BROKE AS A JOKE").draw(
            titleRect, &fontTitle, td::ColorID::Red,
            td::TextAlignment::Center, td::VAlignment::Center
        );

        // message (2 lines)
        const gui::Rect msgRect1(gui::Point(boxPt.x + 40, boxPt.y + 135), gui::Size(boxSize.width - 80, 45));
        const gui::Rect msgRect2(gui::Point(boxPt.x + 40, boxPt.y + 175), gui::Size(boxSize.width - 80, 45));

        gui::DrawableString("You've got exactly 0 to your name.").draw(
            msgRect1, gui::Font::ID::SystemBold, td::ColorID::White,
            td::TextAlignment::Center, td::VAlignment::Center
        );
        gui::DrawableString("You can't sit at this table with empty pockets.").draw(
            msgRect2, gui::Font::ID::SystemBold, td::ColorID::White,
            td::TextAlignment::Center, td::VAlignment::Center
        );

        // button
        const gui::Size btnSize(520, 74);
        const gui::Rect backRect(
            gui::Point((int)rect.width() / 2 - btnSize.width / 2, boxPt.y + 255),
            btnSize
        );

        _goBackBtn = RectCordinates(backRect.top, backRect.bottom, backRect.left, backRect.right);

        gui::Shape btnShape;
        drawButton(
            btnShape, backRect, "GO COLLECT THE PIZZO",
            (_hover == HoverID::GoBack),
            true,
            td::ColorID::DarkGray, td::ColorID::DarkGray,
            td::ColorID::Gold, td::ColorID::Yellow,
            td::ColorID::White, td::ColorID::Gray
        );
    }

    void drawAdvisor(const gui::Rect& rect)
    {
        gui::Size advisorSize(140, 180);
        gui::Point advisorPt((int)rect.width() - advisorSize.width - 40, (int)rect.height() - advisorSize.height - 150);
        gui::Rect advisorRect(advisorPt, advisorSize);

        gui::Image advisorImg(":advisor");
        advisorImg.draw(advisorRect);

        _advisorBtn = RectCordinates(advisorRect.top, advisorRect.bottom, advisorRect.left, advisorRect.right);

        if (_showAdvisorTip && !game.isGameDone() && !outOfMoney())
        {
            auto advice = Advisor::shouldHit(game.playerHand.total, game.playerHand.soft, game.dealerHand.cards[0].rank);

            gui::Shape shape;
            gui::Size bubbleSize(300, 80);
            gui::Point bubblePt(advisorPt.x - bubbleSize.width + advisorSize.width / 2, advisorPt.y - 90);
            gui::Rect bubbleRect(bubblePt, bubbleSize);

            shape.createRoundedRect(bubbleRect, 10);
            shape.drawFillAndWire(td::ColorID::White, td::ColorID::Black, 2);

            gui::DrawableString tipStr(advice.second);
            tipStr.draw(bubbleRect, gui::Font::ID::SystemBold, td::ColorID::Black,
                td::TextAlignment::Center, td::VAlignment::Center);
        }
    }

    void drawTopLeftBackButton(const gui::Rect&)
    {
        gui::Shape shape;
        const int size = 44;
        const int pad = 18;

        gui::Rect r(gui::Point(pad, pad), gui::Size(size, size));

        bool hovered = (_hover == HoverID::TopBack);

        shape.createRoundedRect(r, 10);
        shape.drawFillAndWire(td::ColorID::Black, hovered ? td::ColorID::Yellow : td::ColorID::Gold, hovered ? 3 : 2);

        gui::DrawableString s("<");
        s.draw(r, gui::Font::ID::SystemBold, td::ColorID::Gold,
            td::TextAlignment::Center, td::VAlignment::Center);

        _topBackBtn = RectCordinates(r.top, r.bottom, r.left, r.right);
    }

    void drawBetUI(const gui::Rect& rect)
    {
        gui::Shape shape;

        const int margin = 22;
        const int barH = 64;

        int w = (int)rect.width();
        int h = (int)rect.height();

        const int padX = 16;
        const int gap = 12;

        const int walletW = 210;
        const int elemH = barH - 20;

        const int sideBtn = elemH;
        const int betW = 260;

        int neededW =
            padX +
            walletW + gap +
            sideBtn + gap +
            betW + gap +
            sideBtn +
            padX;

        int barX = w / 2 - neededW / 2;
        int barY = h - barH - margin;

        gui::Rect barRect(gui::Point(barX, barY), gui::Size(neededW, barH));

        shape.createRoundedRect(barRect, 10);
        shape.drawFillAndWire(td::ColorID::DarkGray, td::ColorID::Gold, 2);

        gui::Rect inner(gui::Point(barRect.left + 6, barRect.top + 6),
            gui::Size(barRect.width() - 12, barRect.height() - 12));
        gui::Shape glow;
        glow.createRoundedRect(inner, 8);
        glow.drawWire(td::ColorID::Gray, 1);

        int x = barRect.left + padX;
        int y = barRect.top + 10;

        gui::Rect walletRect(gui::Point(x, y), gui::Size(walletW, elemH));
        x += walletW + gap;

        gui::Shape wsh;
        wsh.createRoundedRect(walletRect, 10);
        wsh.drawFillAndWire(td::ColorID::Black, td::ColorID::Gold, 2);

        gui::DrawableString walletTxt("BANKROLL: " + std::to_string(_walletDisplay));
        walletTxt.draw(walletRect, gui::Font::ID::SystemBold, td::ColorID::Gold,
            td::TextAlignment::Center, td::VAlignment::Center);

        bool betEnabled = (!outOfMoney() && !game.isGameDone());

        gui::Rect minusRect(gui::Point(x, y), gui::Size(sideBtn, sideBtn));
        x += sideBtn + gap;

        _betMinusBtn = RectCordinates(minusRect.top, minusRect.bottom, minusRect.left, minusRect.right);

        gui::Shape msh;
        drawButton(msh, minusRect, "-",
            (_hover == HoverID::BetMinus),
            betEnabled,
            td::ColorID::DarkGray, td::ColorID::DarkGray,
            td::ColorID::Gold, td::ColorID::Yellow,
            td::ColorID::White, td::ColorID::Gray);

        gui::Rect betRect(gui::Point(x, y), gui::Size(betW, sideBtn));
        x += betW + gap;

        gui::Shape bsh;
        bsh.createRoundedRect(betRect, 10);
        bsh.drawFillAndWire(td::ColorID::Black, td::ColorID::Gold, 2);

        gui::DrawableString betTxt("BET: " + std::to_string(_betDisplay));
        betTxt.draw(betRect, gui::Font::ID::SystemBold, td::ColorID::Gold,
            td::TextAlignment::Center, td::VAlignment::Center);

        gui::Rect plusRect(gui::Point(x, y), gui::Size(sideBtn, sideBtn));

        _betPlusBtn = RectCordinates(plusRect.top, plusRect.bottom, plusRect.left, plusRect.right);

        gui::Shape psh;
        drawButton(psh, plusRect, "+",
            (_hover == HoverID::BetPlus),
            betEnabled,
            td::ColorID::DarkGray, td::ColorID::DarkGray,
            td::ColorID::Gold, td::ColorID::Yellow,
            td::ColorID::White, td::ColorID::Gray);
    }

    void drawHitStandRight(const gui::Rect& rect)
    {
        int btnW = 220;
        int btnH = 70;
        int rightMargin = 150;
        int topY = (int)rect.height() / 2 - 200;

        bool enabled = (!outOfMoney() && !game.isGameDone());

        gui::Rect hitRect(gui::Point((int)rect.width() - rightMargin - btnW, topY),
            gui::Size(btnW, btnH));
        _hitBtn = RectCordinates(hitRect.top, hitRect.bottom, hitRect.left, hitRect.right);

        drawButton(_uiShape, hitRect, "HIT",
            (_hover == HoverID::Hit),
            enabled,
            td::ColorID::Black, td::ColorID::DarkGray,
            td::ColorID::Gold, td::ColorID::Yellow,
            td::ColorID::White, td::ColorID::Gray);

        gui::Rect standRect(gui::Point((int)rect.width() - rightMargin - btnW, topY + btnH + 25),
            gui::Size(btnW, btnH));
        _standBtn = RectCordinates(standRect.top, standRect.bottom, standRect.left, standRect.right);

        drawButton(_uiShape, standRect, "STAND",
            (_hover == HoverID::Stand),
            enabled,
            td::ColorID::Black, td::ColorID::DarkGray,
            td::ColorID::Gold, td::ColorID::Yellow,
            td::ColorID::White, td::ColorID::Gray);
    }

    void onDraw(const gui::Rect& rect) override
    {
        gui::Image bgImg(":blackjacktable");
        bgImg.draw(rect);

        drawTopLeftBackButton(rect);

        applyPayoutIfNeeded();

        drawDealer(rect);
        drawPlayer(rect);

        drawHitStandRight(rect);
        drawBetUI(rect);

        if (game.isGameDone())
            drawWinner(rect);
        else {
            _tryAgainBtn = RectCordinates(-1, -1, -1, -1);
            _goBackBtn = RectCordinates(-1, -1, -1, -1);
        }

        drawAdvisor(rect);
        _showAdvisorTip = false;

        if (outOfMoney())
            drawNoMoneyPopup(rect);
    }

    void onPrimaryButtonPressed(const gui::InputDevice& inputDevice) override
    {
        const gui::Point& pt = inputDevice.getModelPoint();

        // Back always works
        if (inRect(pt, _topBackBtn))
        {
            applyPayoutIfNeeded();
            closeWindow();
            return;
        }

        // If out of money only allow GO BACK
        if (outOfMoney())
        {
            if (inRect(pt, _goBackBtn))
            {
                applyPayoutIfNeeded();
                closeWindow();
            }
            return;
        }

        // Betting controls
        if (!game.isGameDone())
        {
            if (inRect(pt, _betMinusBtn))
            {
                GameState::CurrentBet = std::max(0, GameState::CurrentBet - 25);
                GameState::ClampBet();
                animateHudToTargets();
                reDraw();
                return;
            }
            if (inRect(pt, _betPlusBtn))
            {
                GameState::CurrentBet += 25;
                GameState::ClampBet();
                animateHudToTargets();
                reDraw();
                return;
            }
        }

        // HIT
        if (inRect(pt, _hitBtn))
        {
            if (!canInteract()) return;

            if (!game.isGameDone())
            {
                game.playerHit();

                // If round ended, decide win sound ONCE here
                if (game.isGameDone() && !_winSfxPlayed)
                {
                    GameResult r = game.getResult();
                    if (r == GameResult::PlayerWins || r == GameResult::DealerBust)
                    {
                        _sfxWin.play();
                    }
                    _winSfxPlayed = true;
                }

                if (game.isGameDone() && !showDealerUpCard)
                    startRevealFlip();
            }

            reDraw();
            return;
        }

        // STAND
        if (inRect(pt, _standBtn))
        {
            if (!canInteract()) return;

            if (!game.isGameDone())
            {
                startRevealFlip();
                game.playerStand();

                // If round ended, decide win sound ONCE here
                if (game.isGameDone() && !_winSfxPlayed)
                {
                    GameResult r = game.getResult();
                    if (r == GameResult::PlayerWins || r == GameResult::DealerBust)
                    {
                        _sfxWin.play();
                    }
                    _winSfxPlayed = true;
                }
            }

            reDraw();
            return;
        }

        // TRY AGAIN
        if (inRect(pt, _tryAgainBtn))
        {
            _sfxTryAgain.play();

            game.resetGame();
            _tryAgainBtn = RectCordinates(-1, -1, -1, -1);
            _goBackBtn = RectCordinates(-1, -1, -1, -1);

            startGame();
            reDraw();
            return;
        }

        // GO BACK
        if (inRect(pt, _goBackBtn))
        {
            applyPayoutIfNeeded();
            closeWindow();
            return;
        }

        // ADVISOR
        if (inRect(pt, _advisorBtn))
        {
            _showAdvisorTip = true;
            reDraw();
            return;
        }
    }
};
