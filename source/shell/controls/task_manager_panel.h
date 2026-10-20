#ifndef _3D_SHELL_TASK_MANAGER_PANEL_H
#define _3D_SHELL_TASK_MANAGER_PANEL_H

#include "panel_app.h"
#include <citro2d.h>
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
        const C2D_Image *icon;
    };

    std::vector<AppInfo> apps_;
    int selection_ = 0;
    static const int item_height = 25;
    static constexpr float icon_size = 16.0f;

    void RefreshApps();
    void SwitchToSelectedApp();
    void CloseSelectedApp();
};

}

#endif
