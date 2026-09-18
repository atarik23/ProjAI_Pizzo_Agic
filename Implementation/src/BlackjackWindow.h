#pragma once

#include <gui/Window.h>

#include "ui/BlackjackView.h"

class BlackjackWindow : public gui::Window
{
    BlackjackView _mainView;

protected:
    bool shouldClose() override
    {
        return true;
    }

public:
    BlackjackWindow()
        : gui::Window(gui::Size(1200, 840))
    {
        setTitle(tr("blackjackTitle"));
        setCentralView(&_mainView);
    }
};
