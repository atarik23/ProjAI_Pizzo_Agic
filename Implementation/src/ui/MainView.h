//
// MainView.h
//
#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

#include <algorithm>
#include <cmath>
#include <array>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include <cstdint>
#include <map>

#include <gui/Canvas.h>
#include <gui/Shape.h>
#include <gui/DrawableString.h>
#include <gui/Font.h>
#include <gui/Timer.h>
#include <gui/Image.h>
#include <gui/Sound.h>
#include <gui/NatObject.h>

#include "../core/CPP.h"
#include "../core/Loader.h"

#include "../BlackjackWindow.h"
#include "CommonTypes.h"
#include "../GameState.h"

struct Node {
    int x;
    int y;
    std::string name;
    Node(int _x, int _y, std::string _name) : x(_x), y(_y), name(std::move(_name)) {}
};

class MainView : public gui::Canvas
{
protected:
    // UI rects
    std::map<std::string, RectCordinates> _neighCordinates; // updated each draw
    RectCordinates _btnPlayPause{ -1,-1,-1,-1 };
    RectCordinates _speedTrack{ -1,-1,-1,-1 };
    RectCordinates _speedThumb{ -1,-1,-1,-1 };

    RectCordinates _blackjackBtn{ -1,-1,-1,-1 };

    // Neighborhood hover
    std::string _hoverNeigh = "";

    // HUD
    int _hudH = 74;
    float _speed01 = 0.50f;         
    float _animSpeedMin = 0.02f;
    float _animSpeedMax = 0.18f;

    // Cached stats
    int _oddVertices = 0;
    int _pathCostUnique = 0;        // sum of original edges once
    long long _expectedCostCPP = 0; // route.totalCost()

    // Graph
    LoadedNeighborhood komsije;
    CPPResult rjes;
    bool _graphLoaded = false;

    std::vector<Node> nodes;

    int _startNode = -1;
    int _hoverNode = -1;

    gui::Rect _graphRect{ gui::Point(0,0), gui::Size(0,0) };
    gui::Rect _rightRect{ gui::Point(0,0), gui::Size(0,0) };

    // animation
    int _animStep = 0;
    bool _animating = false;
    gui::Timer _timer;

    float _animT = 0.0f;
    float _animSpeed = 0.08f;

    // Toni visible when paused
    bool _toniValid = false;
    gui::Rect _toniDstLast{ gui::Point(0,0), gui::Size(0,0) };

    // audio during traversal
    gui::Sound _sfxTraversal;

    // per-step occurrence and multiplicity
    std::vector<int> _stepOcc;
    std::unordered_map<uint64_t, int> _multiplicity;

    // reward guard for money
    std::unordered_set<uint64_t> _rewardedEdges;

    // weight labels
    struct EdgeLabel {
        gui::Rect rect;
        std::string text;
        gui::Point anchor;
    };
    std::vector<EdgeLabel> _edgeLabels;

    // visuals tuning
    const int NODE_R = 38;
    const int NODE_HIT_R = 46;
    const int GLOW_R = 48;
    const int TONI_W = 44;
    const int TONI_H = 44;

    // Neighborhood labels
    std::array<std::string, 3> _neighLabels = {
        "LITTLE ITALY",
        "NORTH JERSEY",
        "PINE BARRENS"
    };

    std::map<std::string, const char*> _neighFiles = {
        {"LITTLE ITALY",  "neigh1"},
        {"NORTH JERSEY",  "neigh2"},
        {"PINE BARRENS",  "neigh3"},
    };

    // avoid <numbers> issues
    static constexpr double PI = 3.14159265358979323846;

    // ========= helpers =========
    static bool inRect(const gui::Point& p, const RectCordinates& r)
    {
        if (r.left < 0) return false;
        return p.x >= r.left && p.x <= r.right && p.y >= r.top && p.y <= r.bottom;
    }

    static uint64_t edgeKey(int u, int v)
    {
        if (u > v) std::swap(u, v);
        return ((uint64_t)(uint32_t)u << 32) | (uint32_t)v;
    }

    static gui::Rect rectCentered(int cx, int cy, int w, int h)
    {
        return gui::Rect(gui::Point(cx - w / 2, cy - h / 2), gui::Size(w, h));
    }

    static bool rectOverlap(const gui::Rect& a, const gui::Rect& b)
    {
        if (a.right < b.left) return false;
        if (a.left > b.right) return false;
        if (a.bottom < b.top) return false;
        if (a.top > b.bottom) return false;
        return true;
    }

    static inline int imax(int a, int b) { return (a > b) ? a : b; }
    static inline int imin(int a, int b) { return (a < b) ? a : b; }

    static inline float fmaxf(float a, float b) { return (a > b) ? a : b; }
    static inline float fminf(float a, float b) { return (a < b) ? a : b; }

    void applySpeedFromSlider()
    {
        _animSpeed = _animSpeedMin + _speed01 * (_animSpeedMax - _animSpeedMin);
    }

    void drawLineThick(int x1, int y1, int x2, int y2, td::ColorID color)
    {
        auto drawOnce = [&](int ox, int oy) {
            gui::Point pts[2] = { gui::Point(x1 + ox, y1 + oy), gui::Point(x2 + ox, y2 + oy) };
            gui::Shape s;
            s.createLines(pts, 2);
            s.drawWire(color);
            };

        drawOnce(0, 0);
        drawOnce(1, 0);
        drawOnce(-1, 0);
        drawOnce(0, 1);
        drawOnce(0, -1);
    }

    // bezier utilities
    struct FPoint { float x; float y; };

    static FPoint bezierPoint(float x1, float y1, float cx, float cy, float x2, float y2, float t)
    {
        float u = 1.0f - t;
        return {
            u * u * x1 + 2 * u * t * cx + t * t * x2,
            u * u * y1 + 2 * u * t * cy + t * t * y2
        };
    }

    void controlPointCanonical(int a, int b, int copyIndex, int& outCx, int& outCy)
    {
        const Node& A = nodes[a];
        const Node& B = nodes[b];

        float x1 = (float)A.x, y1 = (float)A.y;
        float x2 = (float)B.x, y2 = (float)B.y;
        float mx = 0.5f * (x1 + x2);
        float my = 0.5f * (y1 + y2);

        float dx = x2 - x1;
        float dy = y2 - y1;
        float len = std::sqrt(dx * dx + dy * dy);
        if (len < 1.0f) { outCx = (int)mx; outCy = (int)my; return; }

        float nx = -dy / len;
        float ny = dx / len;

        float side = (copyIndex % 2 == 1) ? 1.0f : -1.0f;
        float base = fminf(46.0f, fmaxf(24.0f, 0.22f * len));
        float off = base + 16.0f * (float)((copyIndex - 1) / 2);

        outCx = (int)(mx + side * off * nx);
        outCy = (int)(my + side * off * ny);
    }

    void drawPolylineThick(const std::vector<gui::Point>& pts, td::ColorID color)
    {
        auto drawOnce = [&](int ox, int oy) {
            std::vector<gui::Point> shifted;
            shifted.reserve(pts.size());
            for (const auto& p : pts) shifted.emplace_back(p.x + ox, p.y + oy);

            gui::Shape s;
            s.createLines(shifted.data(), (int)shifted.size());
            s.drawWire(color);
            };

        drawOnce(0, 0);
        drawOnce(1, 0);
        drawOnce(-1, 0);
        drawOnce(0, 1);
        drawOnce(0, -1);
    }

    void drawDuplicateCurveThick(int u, int v, int copyIndex, td::ColorID color)
    {
        int a = (std::min)(u, v);
        int b = (std::max)(u, v);

        int cx, cy;
        controlPointCanonical(a, b, copyIndex, cx, cy);

        const int SAMPLES = 26;
        std::vector<gui::Point> pts;
        pts.reserve(SAMPLES);

        for (int i = 0; i < SAMPLES; i++)
        {
            float t = (float)i / (float)(SAMPLES - 1);
            FPoint p = bezierPoint((float)nodes[a].x, (float)nodes[a].y,
                (float)cx, (float)cy,
                (float)nodes[b].x, (float)nodes[b].y, t);
            pts.emplace_back((int)p.x, (int)p.y);
        }

        drawPolylineThick(pts, color);
    }

    FPoint toniPosOnStep(int u, int v, float t, int occ)
    {
        const Node& U = nodes[u];
        const Node& V = nodes[v];

        if (occ <= 1)
        {
            return {
                (1.0f - t) * U.x + t * V.x,
                (1.0f - t) * U.y + t * V.y
            };
        }

        int copyIndex = occ - 1;
        int a = (std::min)(u, v);
        int b = (std::max)(u, v);

        int cx, cy;
        controlPointCanonical(a, b, copyIndex, cx, cy);

        float tt = (u == a && v == b) ? t : (1.0f - t);

        return bezierPoint((float)nodes[a].x, (float)nodes[a].y,
            (float)cx, (float)cy,
            (float)nodes[b].x, (float)nodes[b].y, tt);
    }

    int getEdgeCost(int u, int v) const
    {
        if (!_graphLoaded) return 0;
        if (u < 0 || v < 0) return 0;
        if (u >= komsije.graph.vertexCount() || v >= komsije.graph.vertexCount()) return 0;

        try {
            return komsije.graph.minEdgeCost(u, v);
        }
        catch (...) {
            // fallback search
            int best = 0;
            bool found = false;
            for (const auto& e : komsije.graph.adj(u)) {
                if (e.to == v) {
                    if (!found) { best = e.cost; found = true; }
                    else best = (std::min)(best, e.cost);
                }
            }
            return found ? best : 0;
        }
    }

    void buildStepOcc()
    {
        _stepOcc.clear();
        _multiplicity.clear();

        std::unordered_map<uint64_t, int> seen;
        _stepOcc.reserve(rjes.route.steps.size());

        for (const auto& st : rjes.route.steps)
        {
            uint64_t k = edgeKey(st.from, st.to);
            int occ = ++seen[k];
            _stepOcc.push_back(occ);
        }

        _multiplicity = std::move(seen);
    }

    void buildEdgeLabels(const gui::Rect& graphRect)
    {
        _edgeLabels.clear();
        if (!_graphLoaded) return;

        int N = komsije.graph.vertexCount();
        if (N <= 0) return;

        std::vector<gui::Rect> placed;

        // avoid node circles
        std::vector<gui::Rect> nodeBoxes;
        nodeBoxes.reserve(nodes.size());
        for (const auto& n : nodes)
            nodeBoxes.push_back(rectCentered(n.x, n.y, 2 * (NODE_R + 8), 2 * (NODE_R + 8)));

        int centerX = graphRect.left + graphRect.width() / 2;
        int centerY = graphRect.top + graphRect.height() / 2;

        for (int u = 0; u < N; u++)
        {
            const auto& adjU = komsije.graph.adj(u);
            for (const auto& e : adjU)
            {
                int v = e.to;
                if (u >= v) continue;
                if (v < 0 || v >= N) continue;

                const Node& A = nodes[u];
                const Node& B = nodes[v];

                float dx = (float)(B.x - A.x);
                float dy = (float)(B.y - A.y);
                float len = std::sqrt(dx * dx + dy * dy);
                if (len < 1.0f) continue;

                float nx = -dy / len;
                float ny = dx / len;

                float midx = 0.5f * (A.x + B.x);
                float midy = 0.5f * (A.y + B.y);
                float distToCenter = std::sqrt((midx - centerX) * (midx - centerX) + (midy - centerY) * (midy - centerY));

                float f = 0.5f;
                if (distToCenter < 80.0f)
                    f = ((u + v) % 2 == 0) ? 0.35f : 0.65f;

                int ax = (int)((1.0f - f) * A.x + f * B.x);
                int ay = (int)((1.0f - f) * A.y + f * B.y);

                int mult = 1;
                auto itM = _multiplicity.find(edgeKey(u, v));
                if (itM != _multiplicity.end())
                    mult = (std::max)(1, itM->second);

                float baseOff = 20.0f;
                float extraOff = (mult > 1) ? (46.0f + 16.0f * (mult - 2)) : 0.0f;
                float off = baseOff + extraOff;

                float side = ((u + v) % 2 == 0) ? 1.0f : -1.0f;

                std::string txt = std::to_string(e.cost);

                const int boxW = 46;
                const int boxH = 20;

                auto candidate = [&](float curOff) {
                    int cx = (int)(ax + side * curOff * nx);
                    int cy = (int)(ay + side * curOff * ny);
                    return rectCentered(cx, cy, boxW, boxH);
                    };

                gui::Rect cand = candidate(off);

                for (int tries = 0; tries < 18; tries++)
                {
                    bool ok = true;
                    for (auto& r : placed) { if (rectOverlap(cand, r)) { ok = false; break; } }
                    if (ok) for (auto& nb : nodeBoxes) { if (rectOverlap(cand, nb)) { ok = false; break; } }
                    if (ok) break;

                    off += 14.0f;
                    cand = candidate(off);
                }

                placed.push_back(cand);
                _edgeLabels.push_back({ cand, txt, gui::Point(ax, ay) });
            }
        }
    }

    int computeOddVertices() const
    {
        if (!_graphLoaded) return 0;
        int N = komsije.graph.vertexCount();
        int odd = 0;
        for (int u = 0; u < N; u++)
        {
            int deg = (int)komsije.graph.adj(u).size();
            if (deg % 2 != 0) odd++;
        }
        return odd;
    }

    // Path Cost = sum of original edges once
    int computePathCostUnique() const
    {
        if (!_graphLoaded) return 0;
        int N = komsije.graph.vertexCount();
        int sum = 0;

        for (int u = 0; u < N; u++)
        {
            const auto& adjU = komsije.graph.adj(u);
            for (const auto& e : adjU)
            {
                int v = e.to;
                if (u < v) sum += e.cost;
            }
        }
        return sum;
    }

    long long computeExpectedCostCPP() const
    {
        // Expected(CPP) = whole traversal cost
        // Route has: long long totalCost() const;
        return rjes.route.totalCost();
    }

    void syncHudStats()
    {
        _oddVertices = computeOddVertices();
        _pathCostUnique = computePathCostUnique();

        if (_graphLoaded && !rjes.route.steps.empty())
            _expectedCostCPP = computeExpectedCostCPP();
        else
            _expectedCostCPP = 0;
    }

    void startFromNode(int startIdx)
    {
        if (!_graphLoaded) return;
        if (startIdx < 0 || startIdx >= (int)nodes.size()) return;

        _startNode = startIdx;

        _timer.stop();
        _animating = false;

        _rewardedEdges.clear();

        rjes = CPP::solve(komsije.graph, _startNode);
        buildStepOcc();

        _animStep = 0;
        _animT = 0.0f;
        _toniValid = false;

        // don't auto-start movement until user hits START
        reDraw();
    }

    void setAnimating(bool on)
    {
        if (on)
        {
            if (!_graphLoaded) return;
            if (rjes.route.steps.empty()) return;
            if (_animStep >= (int)rjes.route.steps.size()) return;

            _animating = true;
            if (!_timer.isRunning()) _timer.start();

            // start music
            _sfxTraversal.play();
        }
        else
        {
            _animating = false;
            _timer.stop();

            // stop music
            _sfxTraversal.stop();
        }
    }

    // HUD drawing
    void drawTopHUD(const gui::Rect& rect)
    {
        gui::Shape sh;
        gui::Rect hudRect(gui::Point(0, 0), gui::Size(rect.width(), _hudH));
        sh.createRect(hudRect);
        sh.drawFillAndWire(td::ColorID::DarkGray, td::ColorID::DarkGoldenRod);

        const int pad = 12;

        int hintH = _hudH / 2;
        gui::Rect hintR(gui::Point(0, 0), gui::Size(rect.width(), hintH));

        gui::DrawableString hint("Select a node to begin");
        hint.draw(hintR, gui::Font::ID::SystemBold, td::ColorID::DarkGoldenRod,
            td::TextAlignment::Center, td::VAlignment::Center);

        // when node is selected show play/pause button + slider + stats + wallet
        if (_startNode >= 0 && _graphLoaded && !rjes.route.steps.empty())
        {
            const int btnW = 90;
            const int btnH = 42;

            // Play/Pause
            gui::Rect btnR(gui::Point(pad, (_hudH - btnH) / 2), gui::Size(btnW, btnH));
            sh.createRoundedRect(btnR, 8);
            sh.drawFillAndWire(td::ColorID::Black, td::ColorID::DarkGoldenRod, 2);

            std::string label = _animating ? "STOP" : "START";
            gui::DrawableString sBtn(label);
            sBtn.draw(btnR, gui::Font::ID::SystemBold, td::ColorID::White,
                td::TextAlignment::Center, td::VAlignment::Center);
            _btnPlayPause = RectCordinates(btnR.top, btnR.bottom, btnR.left, btnR.right);

            // Speed label
            gui::Rect speedLblR(gui::Point(btnR.right + 18, 0), gui::Size(60, _hudH));
            gui::DrawableString sSpeed("SPEED");
            sSpeed.draw(speedLblR, gui::Font::ID::SystemBold, td::ColorID::White,
                td::TextAlignment::Center, td::VAlignment::Center);

            // Slider track
            const int trackW = 210;
            const int trackH = 10;

            int trackX = speedLblR.right + 8;
            int trackY = _hudH / 2 - trackH / 2;

            gui::Rect trackR(gui::Point(trackX, trackY), gui::Size(trackW, trackH));
            sh.createRoundedRect(trackR, 6);
            sh.drawFillAndWire(td::ColorID::Black, td::ColorID::DarkGoldenRod, 2);
            _speedTrack = RectCordinates(trackR.top, trackR.bottom, trackR.left, trackR.right);

            // Slider thumb
            const int thumbW = 18;
            const int thumbH = 26;
            int thumbX = trackR.left + (int)std::round(_speed01 * (float)(trackR.width() - thumbW));
            int thumbY = _hudH / 2 - thumbH / 2;

            gui::Rect thumbR(gui::Point(thumbX, thumbY), gui::Size(thumbW, thumbH));
            sh.createRoundedRect(thumbR, 7);
            sh.drawFillAndWire(td::ColorID::DarkGoldenRod, td::ColorID::Black, 2);
            _speedThumb = RectCordinates(thumbR.top, thumbR.bottom, thumbR.left, thumbR.right);

            // Stats in the middle area
            syncHudStats();

            std::string stats =
                "Real Path Cost: " + std::to_string((long long)_expectedCostCPP) +
                " | Sum of Edges: " + std::to_string(_pathCostUnique) +
                " | Odd Vertices: " + std::to_string(_oddVertices);

            // Wallet on right
            const int walletW = 200;
            const int walletH = 44;
            gui::Rect walletR(gui::Point(rect.width() - pad - walletW, (_hudH - walletH) / 2), gui::Size(walletW, walletH));
            sh.createRoundedRect(walletR, 10);
            sh.drawFillAndWire(td::ColorID::Black, td::ColorID::DarkGoldenRod, 2);

            gui::DrawableString wStr("Money: " + std::to_string(GameState::Wallet));
            wStr.draw(walletR, gui::Font::ID::SystemBold, td::ColorID::White,
                td::TextAlignment::Center, td::VAlignment::Center);

            int leftX = trackR.right + 14;
            int rightX = walletR.left - 10;
            if (rightX > leftX + 40)
            {
                // Stats in the BOTTOM half of HUD
                int statsTop = _hudH / 2;
                int statsH = _hudH - statsTop;

                gui::Rect statsR(gui::Point(leftX, statsTop), gui::Size(rightX - leftX, statsH));
                gui::DrawableString ss(stats);
                ss.draw(statsR, gui::Font::ID::SystemBold, td::ColorID::White,
                    td::TextAlignment::Center, td::VAlignment::Center);
            }
        }
        else
        {
            // hide interactables until node selected
            _btnPlayPause = RectCordinates(-1, -1, -1, -1);
            _speedTrack = RectCordinates(-1, -1, -1, -1);
            _speedThumb = RectCordinates(-1, -1, -1, -1);
        }
    }

    // Right panel drawing
    void drawPicker(const gui::Rect& rect)
    {
        // smaller panel placed top-right area
        const int margin = 18;
        const int panelW = rect.width() - 2 * margin;
        const int panelH = 330;
        gui::Rect panelR(gui::Point(rect.left + margin, rect.top + margin),
            gui::Size(panelW, panelH));

        gui::Shape panel;
        panel.createRoundedRect(panelR, 12);
        panel.drawFillAndWire(td::ColorID::DarkGray, td::ColorID::DarkGoldenRod, 2);

        // Title
        gui::Rect titleR = panelR;
        titleR.top += 12;
        titleR.left += 16;
        titleR.bottom = titleR.top + 30;
        gui::DrawableString chooseStr("Choose Neighborhood");
        chooseStr.draw(titleR, gui::Font::ID::SystemBold, td::ColorID::White,
            td::TextAlignment::Left, td::VAlignment::Center);

        // Items
        const int leftPad = 16;
        const int itemH = 72;
        const int gap = 14;

        int y = titleR.bottom + 14;

        for (const auto& neighborName : _neighLabels)
        {
            gui::Rect itemRect(
                gui::Point(panelR.left + leftPad, y),
                gui::Size(panelR.width() - 2 * leftPad, itemH)
            );

            bool hovered = (_hoverNeigh == neighborName);

            gui::Shape itemSh;
            itemSh.createRoundedRect(itemRect, 10);

            if (hovered)
                itemSh.drawFillAndWire(td::ColorID::DarkGoldenRod, td::ColorID::Black, 2);
            else
                itemSh.drawFillAndWire(td::ColorID::Black, td::ColorID::DarkGoldenRod, 2);

            gui::DrawableString itemStr(neighborName);
            itemStr.draw(itemRect, gui::Font::ID::SystemBold,
                hovered ? td::ColorID::Black : td::ColorID::DarkGoldenRod,
                td::TextAlignment::Center, td::VAlignment::Center);

            _neighCordinates[neighborName] = RectCordinates(itemRect.top, itemRect.bottom, itemRect.left, itemRect.right);
            y += itemH + gap;
        }
    }

    // Blackjack button bottom-left
    void drawBlackjackButton(const gui::Rect& rect)
    {
        const int pad = 18;
        const int w = 160;
        const int h = 56;

        gui::Rect r(gui::Point(rect.width() - pad - w, rect.height() - pad - h), gui::Size(w, h));

        gui::Shape s;
        s.createRoundedRect(r, 10);
        s.drawFillAndWire(td::ColorID::Firebrick, td::ColorID::DarkGoldenRod, 2);

        gui::DrawableString ds("Blackjack");
        ds.draw(r, gui::Font::ID::SystemBold, td::ColorID::White,
            td::TextAlignment::Center, td::VAlignment::Center);

        _blackjackBtn = RectCordinates(r.top, r.bottom, r.left, r.right);
    }

    // Graph drawing
    void drawSolution(const gui::Rect& graphRect)
    {
        nodes.clear();
        if (!_graphLoaded) return;

        int N = komsije.graph.vertexCount();
        if (N <= 0) return;

        int middleX = graphRect.left + graphRect.width() / 2;
        int middleY = graphRect.top + graphRect.height() / 2;

        int R = imax(120, (int)(graphRect.height() / 2) - 110);
        R = imin(R, (int)(graphRect.width() / 2) - 70);

        // layout nodes
        for (int i = 0; i < N; i++)
        {
            double ang = (2.0 * PI * (double)i) / (double)N;
            int x = middleX + (int)std::round((double)R * std::cos(ang));
            int y = middleY - (int)std::round((double)R * std::sin(ang));
            std::string lbl = (i < (int)komsije.nodes.size()) ? komsije.nodes[i] : std::to_string(i);
            nodes.emplace_back(x, y, lbl);
        }

        buildEdgeLabels(graphRect);

        const td::ColorID baseColor = td::ColorID::DarkGoldenRod;
        const td::ColorID dupColor = td::ColorID::White;
        const td::ColorID travColor = td::ColorID::Firebrick;
        const td::ColorID curColor = td::ColorID::Black;

        // base edges
        for (int u = 0; u < N; u++)
        {
            const auto& adjU = komsije.graph.adj(u);
            for (const auto& e : adjU)
            {
                int v = e.to;
                if (u >= v) continue;
                if (v < 0 || v >= N) continue;
                drawLineThick(nodes[u].x, nodes[u].y, nodes[v].x, nodes[v].y, baseColor);
            }
        }

        // duplicates plan
        for (int u = 0; u < N; u++)
        {
            const auto& adjU = komsije.graph.adj(u);
            for (const auto& e : adjU)
            {
                int v = e.to;
                if (u >= v) continue;
                if (v < 0 || v >= N) continue;

                auto it = _multiplicity.find(edgeKey(u, v));
                if (it == _multiplicity.end()) continue;

                int mult = it->second;
                for (int occ = 2; occ <= mult; occ++)
                    drawDuplicateCurveThick(u, v, occ - 1, dupColor);
            }
        }

        // traversed overlay
        int completed = imin(_animStep, (int)rjes.route.steps.size());
        for (int i = 0; i < completed; i++)
        {
            const auto& st = rjes.route.steps[i];
            int occ = (i < (int)_stepOcc.size()) ? _stepOcc[i] : 1;

            if (occ <= 1)
                drawLineThick(nodes[st.from].x, nodes[st.from].y, nodes[st.to].x, nodes[st.to].y, travColor);
            else
                drawDuplicateCurveThick(st.from, st.to, occ - 1, travColor);
        }

        // current highlight + compute Toni position
        if (_animStep >= 0 && _animStep < (int)rjes.route.steps.size())
        {
            const auto& st = rjes.route.steps[_animStep];
            int curU = st.from;
            int curV = st.to;
            int curOcc = (_animStep < (int)_stepOcc.size()) ? _stepOcc[_animStep] : 1;

            if (_animating)
            {
                if (curOcc <= 1)
                    drawLineThick(nodes[curU].x, nodes[curU].y, nodes[curV].x, nodes[curV].y, curColor);
                else
                    drawDuplicateCurveThick(curU, curV, curOcc - 1, curColor);
            }

            // update Toni last-known position both while animating AND while paused
            FPoint p = toniPosOnStep(curU, curV, _animT, curOcc);
            _toniDstLast = gui::Rect(
                gui::Point((int)(p.x - TONI_W / 2), (int)(p.y - TONI_H / 2)),
                gui::Size(TONI_W, TONI_H)
            );
            _toniValid = true;
        }

        // draw weights + leader lines
        for (const auto& lab : _edgeLabels)
        {
            int cx = (lab.rect.left + lab.rect.right) / 2;
            int cy = (lab.rect.top + lab.rect.bottom) / 2;

            drawLineThick(lab.anchor.x, lab.anchor.y, cx, cy, td::ColorID::DarkGoldenRod);

            gui::Shape box;
            box.createRect(lab.rect);
            box.drawFillAndWire(td::ColorID::Black, td::ColorID::DarkGoldenRod);

            gui::DrawableString ds(lab.text);
            ds.draw(lab.rect, gui::Font::ID::SystemBold, td::ColorID::White,
                td::TextAlignment::Center, td::VAlignment::Center);
        }

        // draw nodes
        for (int i = 0; i < (int)nodes.size(); i++)
        {
            const Node& node = nodes[i];

            if (i == _hoverNode)
            {
                gui::Circle glow(node.x, node.y, GLOW_R);
                gui::Shape g;
                g.createCircle(glow);
                g.drawWire(td::ColorID::DarkGoldenRod);
            }

            gui::Circle circle(node.x, node.y, NODE_R);
            gui::Shape s;
            s.createCircle(circle);

            if (i == _startNode || i == _hoverNode)
                s.drawFillAndWire(td::ColorID::Stone, td::ColorID::DarkGoldenRod);
            else
                s.drawFillAndWire(td::ColorID::Stone, td::ColorID::SysText);

            gui::DrawableString str(node.name);
            str.draw(circle, gui::Font::ID::SystemBold, td::ColorID::White,
                td::TextAlignment::Center, td::VAlignment::Center);
        }

        //draw Toni last
        if (_toniValid)
        {
            static gui::Image toniImg(":toni");
            toniImg.draw(_toniDstLast);
        }
    }

    void loadNeighborhoodOnly(const std::string& neigh)
    {
        auto it = _neighFiles.find(neigh);
        if (it == _neighFiles.end()) return;

        const char* resKey = it->second; // ":neighborhood1"

        std::string text = getResText(resKey).c_str();

        mu::dbgLog("CLICK neigh='%s' resKey='%s' textLen=%d\n",
            neigh.c_str(), resKey, (int)text.length());

        try
        {
            komsije = Loader::loadNeighborhoodFromString(text, resKey);
        }
        catch (const std::exception& e)
        {
            showAlert("Loader error", td::String(e.what()));
            return;
        }

        _graphLoaded = true;

        _startNode = -1;
        _hoverNode = -1;

        rjes = CPPResult{};
        _stepOcc.clear();
        _multiplicity.clear();
        _edgeLabels.clear();
        _rewardedEdges.clear();

        _animStep = 0;
        _animT = 0.0f;
        _toniValid = false;

        setAnimating(false);
        reDraw();
    }

public:
    MainView()
        : Canvas({
            gui::InputDevice::Event::PrimaryClicks,
            gui::InputDevice::Event::CursorMove
            })
        , _timer(this, 0.04f, false)
        , _sfxTraversal(":traversal")
    {
        applySpeedFromSlider();

        _timer.onTimer([this]() {
            if (!_animating) return;

            _animT += _animSpeed;

            while (_animT >= 1.0f)
            {
                _animT -= 1.0f;
                _animStep++;

                int doneStep = _animStep - 1;
                if (doneStep >= 0 && doneStep < (int)rjes.route.steps.size())
                {
                    const auto& st = rjes.route.steps[doneStep];
                    int w = getEdgeCost(st.from, st.to);

                    const int BASE_REWARD = 40;
                    const int WEIGHT_BONUS = 10;
                    const int PENALTY = 3;

                    uint64_t k = edgeKey(st.from, st.to);

                    if (_rewardedEdges.insert(k).second)
                        GameState::AddMoney(BASE_REWARD + WEIGHT_BONUS * w);

                    GameState::AddMoney(-PENALTY * w);
                }

                // finished
                if (_animStep >= (int)rjes.route.steps.size())
                {
                    _animating = false;
                    _timer.stop();
                    _sfxTraversal.stop();

                    break;
                }
            }

            reDraw();
            });
    }

    void onCursorMoved(const gui::InputDevice& dev) override
    {
        const gui::Point& pt = dev.getModelPoint();

        // hover on neighborhoods
        _hoverNeigh = "";
        if (pt.x >= _rightRect.left)
        {
            for (const auto& kv : _neighCordinates)
            {
                if (inRect(pt, kv.second))
                {
                    _hoverNeigh = kv.first;
                    break;
                }
            }
            reDraw();
            return;
        }

        // hover nodes
        _hoverNode = -1;
        if (pt.y < _hudH) { reDraw(); return; }
        if (!_graphLoaded || nodes.empty()) { reDraw(); return; }

        if (pt.x < _graphRect.left || pt.x > _graphRect.right ||
            pt.y < _graphRect.top || pt.y > _graphRect.bottom)
        {
            reDraw();
            return;
        }

        const int r2 = NODE_HIT_R * NODE_HIT_R;
        for (int i = 0; i < (int)nodes.size(); i++)
        {
            int dx = pt.x - nodes[i].x;
            int dy = pt.y - nodes[i].y;
            if (dx * dx + dy * dy <= r2)
            {
                _hoverNode = i;
                break;
            }
        }

        reDraw();
    }

    void onDraw(const gui::Rect& rect) override
    {
        const float BG_SCALE = 1.2f;

        int w = rect.width();
        int h = rect.height();

        int nw = (int)std::round(w * BG_SCALE);
        int nh = (int)std::round(h * BG_SCALE);

        int dx = (nw - w) / 2;
        int dy = (nh - h) / 2;

        gui::Rect bgRect(gui::Point(rect.left - dx, rect.top - dy), gui::Size(nw, nh));

        gui::Image bg(":background");
        bg.draw(bgRect);

        // top HUD
        drawTopHUD(rect);

        // content area below HUD
        int contentTop = _hudH;
        int contentH = rect.height() - contentTop;
        int halfW = rect.width() / 2;

        _graphRect = gui::Rect(gui::Point(0, contentTop), gui::Size(halfW, contentH));
        _rightRect = gui::Rect(gui::Point(halfW, contentTop), gui::Size(rect.width() - halfW, contentH));

        if (_graphLoaded)
            drawSolution(_graphRect);

        drawPicker(_rightRect);
        drawBlackjackButton(rect);
    }

    void onPrimaryButtonPressed(const gui::InputDevice& inputDevice) override
    {
        const gui::Point& pt = inputDevice.getModelPoint();

        // Blackjack
        if (inRect(pt, _blackjackBtn))
        {
            BlackjackWindow* window = new BlackjackWindow();
            window->open();
            return;
        }

        // HUD clicks
        if (pt.y < _hudH)
        {
            // Play/Pause toggle (only exists after node selected)
            if (inRect(pt, _btnPlayPause))
            {
                if (_animating) setAnimating(false);
                else setAnimating(true);
                reDraw();
                return;
            }

            // Slider click-to-set speed
            if (inRect(pt, _speedTrack) || inRect(pt, _speedThumb))
            {
                int trackL = _speedTrack.left;
                int trackR = _speedTrack.right;
                int thumbW = (_speedThumb.right - _speedThumb.left);

                int x = pt.x;
                if (x < trackL) x = trackL;
                if (x > trackR - thumbW) x = trackR - thumbW;

                int denom = imax(1, (trackR - trackL - thumbW));
                float t = (float)(x - trackL) / (float)denom;

                if (t < 0.0f) t = 0.0f;
                if (t > 1.0f) t = 1.0f;

                _speed01 = t;
                applySpeedFromSlider();
                reDraw();
                return;
            }

            return;
        }

        // Right panel clicks
        if (pt.x >= _rightRect.left)
        {
            for (const auto& kv : _neighCordinates)
            {
                if (inRect(pt, kv.second))
                {
                    loadNeighborhoodOnly(kv.first);
                    return;
                }
            }
            return;
        }

        // Graph clicks
        if (_graphLoaded && !nodes.empty())
        {
            if (pt.x < _graphRect.left || pt.x > _graphRect.right ||
                pt.y < _graphRect.top || pt.y > _graphRect.bottom)
                return;

            const int r2 = NODE_HIT_R * NODE_HIT_R;
            for (int i = 0; i < (int)nodes.size(); i++)
            {
                int dx = pt.x - nodes[i].x;
                int dy = pt.y - nodes[i].y;
                if (dx * dx + dy * dy <= r2)
                {
                    startFromNode(i);
                    return;
                }
            }
        }
    }
};
