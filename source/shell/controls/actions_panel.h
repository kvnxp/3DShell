#ifndef _3D_SHELL_ACTIONS_PANEL_H
#define _3D_SHELL_ACTIONS_PANEL_H

#include <functional>

#include "panel_app.h"
#include "gui.h"

namespace GUI {

class FileExplorerScreen;

class ActionsPanel : public GUI::PanelApp {
public:
    ActionsPanel(FileExplorerScreen *owner, MenuItem *item);

    std::function<void()> ListingChanged;

    void Draw() override;
    void OnKeyDown(u32 k_down) override;
    void OnKeyHeld(u32 k_held) override;
    void OnFrame(u64 delta_time) override;
    void OnTouch(const touchPosition &touch, bool down) override;

private:
    enum Action {
        NEW_FILE,
        NEW_FOLDER,
        PROPERTIES,
        RENAME,
        COPY,
        PASTE,
        MOVE,
        DELETE_ACTION,
        REFRESH,
        COUNT
    };

    void PerformAction(Action action);
    void ShowMoreOptions();
    void HideMoreOptions();

    FileExplorerScreen *owner_;
    MenuItem *item_;
    int selection_ = 0;
    bool more_options_shown_ = false;
    static const int sel_dist = 40;
};

}

#endif
