#pragma once

#include <gui/Window.h>

#include "ui/MainView.h"

class MainWindow : public gui::Window
{
    MainView _mainView;

protected:
    bool shouldClose() override
    {
        return true;
    }

public:
    MainWindow()
        : gui::Window(gui::Size(1200, 840))
    {
        setTitle(tr("appTitle"));
        setCentralView(&_mainView);
    }
};
