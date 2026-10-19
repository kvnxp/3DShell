#ifndef _3D_SHELL_TASK_MANAGER_PANEL_H
#define _3D_SHELL_TASK_MANAGER_PANEL_H

#include "panel_app.h"
#include <vector>
#include <string>

namespace GUI {

class TaskManagerPanel : public GUI::PanelApp {
public:
    TaskManagerPanel();

    void Draw() override;
    void OnKeyDown(u32 k_down) override;
    void OnTouch(const touchPosition &touch, bool down) override;

private:
    struct AppInfo {
        std::string id;
        std::string name;
        void *screen_ptr;
    };

    std::vector<AppInfo> apps_;
    int selection_ = 0;
    static const int item_height = 25;

    void RefreshApps();
    void SwitchToSelectedApp();
    void CloseSelectedApp();
};

}

#endif
