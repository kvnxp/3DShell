#include "task_manager_panel.h"
#include "screen_manager.h"
#include "screen.h"
#include "c2d_helper.h"
#include "colours.h"
#include "config.h"
#include "touch.h"

namespace GUI {

TaskManagerPanel::TaskManagerPanel() {
    RefreshApps();
}

void TaskManagerPanel::RefreshApps() {
    apps_.clear();
    
    // Get all screens from ScreenManager except desktop (root)
    auto &sm = UI::ScreenManager::Instance();
    for (size_t i = 1; i < sm.StackSize(); ++i) {
        auto *screen = sm.GetScreenAt(i);
        if (screen && screen != sm.GetScreenAt(0)) {
            AppInfo info;
            info.id = std::to_string(i);
            info.name = screen->Name();
            info.screen_ptr = screen;
            apps_.push_back(info);
        }
    }
}

void TaskManagerPanel::Draw() {
    // Draw semi-transparent overlay
    C2D::Rect(0, 0, 320, 240, C2D_Color32(0, 0, 0, 128));
    
    // Draw panel background
    float panel_x = 20;
    float panel_y = 20;
    float panel_w = 280;
    float panel_h = 200;
    
    C2D::Rect(panel_x, panel_y, panel_w, panel_h, 
              cfg.dark_theme ? MENU_BAR_DARK : STATUS_BAR_LIGHT);
    
    // Title
    C2D::Text(panel_x + 10, panel_y + 8, 0.5f, 
              cfg.dark_theme ? WHITE : BLACK, "Task Manager");
    
    // Separator
    C2D::Rect(panel_x + 5, panel_y + 25, panel_w - 10, 1,
              cfg.dark_theme ? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);
    
    // App list
    float list_y = panel_y + 35;
    
    if (apps_.empty()) {
        C2D::Text(panel_x + 10, list_y + 10, 0.4f,
                  cfg.dark_theme ? WHITE : BLACK, "No apps running");
    } else {
        for (size_t i = 0; i < apps_.size(); ++i) {
            float item_y = list_y + (i * item_height);
            
            // Selection highlight
            if (static_cast<int>(i) == selection_) {
                C2D::Rect(panel_x + 5, item_y - 2, panel_w - 10, item_height,
                          cfg.dark_theme ? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);
            }
            
            // App name
            C2D::Text(panel_x + 10, item_y + 5, 0.45f,
                      cfg.dark_theme ? WHITE : BLACK, apps_[i].name.c_str());
        }
    }
    
    // Instructions
    C2D::Text(panel_x + 10, panel_y + panel_h - 25, 0.35f,
              cfg.dark_theme ? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT,
              "A: Switch  X: Close  B: Back");
}

void TaskManagerPanel::OnKeyDown(u32 k_down) {
    if (k_down & KEY_B) {
        Close();
        return;
    }
    
    if (k_down & KEY_UP) {
        if (selection_ > 0) {
            selection_--;
        }
    }
    
    if (k_down & KEY_DOWN) {
        if (selection_ < static_cast<int>(apps_.size()) - 1) {
            selection_++;
        }
    }
    
    if (k_down & KEY_A) {
        SwitchToSelectedApp();
        return;
    }
    
    if (k_down & KEY_X) {
        CloseSelectedApp();
        RefreshApps();
    }
}

void TaskManagerPanel::OnTouch(const touchPosition &touch, bool down) {
    if (!down) return;
    
    // Check if touch is in the list area
    float panel_x = 20;
    float panel_y = 20;
    float list_y = panel_y + 35;
    
    for (size_t i = 0; i < apps_.size(); ++i) {
        float item_y = list_y + (i * item_height);
        if (Touch::Rect(panel_x + 5, item_y - 2, panel_x + 275, item_y + item_height)) {
            selection_ = static_cast<int>(i);
            return;
        }
    }
}

void TaskManagerPanel::SwitchToSelectedApp() {
    if (selection_ >= 0 && selection_ < static_cast<int>(apps_.size())) {
        auto *screen = static_cast<UI::Screen*>(apps_[selection_].screen_ptr);
        auto &sm = UI::ScreenManager::Instance();
        
        // Find and move the screen to top of stack
        for (size_t i = 1; i < sm.StackSize(); ++i) {
            if (sm.GetScreenAt(i) == screen) {
                sm.SwitchToScreenAt(i);
                Close();
                break;
            }
        }
    }
}

void TaskManagerPanel::CloseSelectedApp() {
    if (selection_ >= 0 && selection_ < static_cast<int>(apps_.size())) {
        // Get the screen pointer and close it
        auto *screen = static_cast<UI::Screen*>(apps_[selection_].screen_ptr);
        auto &sm = UI::ScreenManager::Instance();
        
        // Find and remove the screen from stack
        for (size_t i = 1; i < sm.StackSize(); ++i) {
            if (sm.GetScreenAt(i) == screen) {
                sm.CloseScreenAt(i);
                break;
            }
        }
    }
}

}
