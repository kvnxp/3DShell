#include "desktop_screen.h"

#include "app_drawer_panel.h"
#include "file_explorer_screen.h"
#include "hbloader.h"
#include "log.h"
#include "net_loader_panel.h"
#include "netloader.h"
#include "settings_panel.h"
#include "touch.h"

#include "apps.h"
#include "logger/logger.h"

namespace GUI {

static bool pending_drawer_request = false;

    static constexpr u32 DESKTOP_BG_DARK = C2D_Color32(39, 43, 58, 255);
    static constexpr u32 DESKTOP_BG_LIGHT = C2D_Color32(207, 216, 220, 255);

DesktopScreen::DesktopScreen() {
    SetTopBackground(cfg.dark_theme? DESKTOP_BG_DARK : DESKTOP_BG_LIGHT);
    SetBottomBackground(cfg.dark_theme? DESKTOP_BG_DARK : DESKTOP_BG_LIGHT);

    touch_bar_.SetHomeActive(false);

    touch_bar_.AddButton(TouchBar::ICON_HOME, false).Click = [this]() {
        if (drawer_) {
            CloseDrawer();
        } else {
            OpenDrawer();
        }
    };

    /* Window close X (Windows Mobile style): closes the drawer or the
     * active tab. */
    touch_bar_.AddTextButton("X", ERROR_COLOUR, true).Click = [this]() {
        if (drawer_) {
            CloseDrawer();
        } else if (touch_bar_.ActiveContent()) {
            touch_bar_.RemoveTab(touch_bar_.ActiveTabId());
        }
    };

    NetworkLogger::PrintfTagged("SHELL", "desktop: status bar + touch bar ready");
}

void DesktopScreen::OnFrame(u64 delta_time) {
    dying_panel_.reset();

    if (HBLoader::PendingExit())
        UI::ScreenManager::Instance().Exit();

    if (ConsumeDrawerRequest())
        OpenDrawer();

    if (drawer_)
        drawer_->OnFrame(delta_time);
    if (auto content = touch_bar_.ActiveContent()) {
        content->OnFrame(delta_time);
    }

    for (const auto &id : pending_tab_removals_) {
        if (touch_bar_.HasTab(id))
            touch_bar_.RemoveTab(id);
    }
    pending_tab_removals_.clear();
}

void DesktopScreen::DrawTop() {
    status_bar_.Draw();

    /* Placeholder desktop title until the wallpaper lands. */
    float text_width = 0.0f, text_height = 0.0f;
    C2D::GetTextSize(0.7f, &text_width, &text_height, "3DShell");
    C2D::Text(((400.0f - text_width) / 2.0f), ((240.0f - text_height) / 2.0f), 0.7f,
        cfg.dark_theme? WHITE : BLACK, "3DShell");
}

void DesktopScreen::DrawBottom() {
    touch_bar_.SetHomeActive(drawer_ != nullptr);
    touch_bar_.SetCloseUseWM(false);
    touch_bar_.SetCloseVisible(drawer_ != nullptr || !touch_bar_.Tabs().empty());
    touch_bar_.Draw();

    if (drawer_) {
        drawer_->Draw();
    } else if (auto content = touch_bar_.ActiveContent()) {
        content->Draw();
    }
}

void DesktopScreen::OnKeyHeld(u32 k_held) {
    (void)k_held;
    Touch::Update();
}

void DesktopScreen::OnTouch(const touchPosition &touch, bool down) {
    bool in_bar = down? (touch.py < 20) : bar_touching_;

    if (in_bar) {
        touch_bar_.OnTouch(touch, down);
        bar_touching_ = down;
        return;
    }

    bar_touching_ = false;

    /* The rest of the panel belongs to the open panel app; releases are
     * forwarded too so its buttons complete their click. */
    bool in_panel = down? (touch.py >= 20) : panel_touching_;

    if (in_panel) {
        if (drawer_) {
            drawer_->OnTouch(touch, down);
        } else if (auto content = touch_bar_.ActiveContent()) {
            content->OnTouch(touch, down);
        }
        panel_touching_ = down;
    }
}

void DesktopScreen::OnKeyDown(u32 k_down) {
    if (k_down & KEY_START) {
        UI::ScreenManager::Instance().Exit();
        return;
    }

    if (drawer_) {
        drawer_->OnKeyDown(k_down);
    } else if (auto content = touch_bar_.ActiveContent()) {
        content->OnKeyDown(k_down);
    }
}

void DesktopScreen::RequestDrawer() {
    pending_drawer_request = true;
}

bool DesktopScreen::ConsumeDrawerRequest() {
    bool v = pending_drawer_request;
    pending_drawer_request = false;
    return v;
}

void DesktopScreen::OpenExplorer() {
    auto explorer = std::make_unique<FileExplorerScreen>();
    if (R_SUCCEEDED(explorer->Init())) {
        CloseDrawer();
        UI::ScreenManager::Instance().Push(std::move(explorer));
    }
}

void DesktopScreen::OpenDrawer() {
    touch_bar_.SetHomeActive(true);
    auto drawer = std::make_unique<AppDrawerPanel>();

    drawer->LaunchScreen = [this](std::unique_ptr<UI::Screen> screen) {
        CloseDrawer();
        if (screen) {
            UI::ScreenManager::Instance().Push(std::move(screen));
        }
    };

    drawer->SwitchPanel = [this](Apps::Id id, std::unique_ptr<GUI::PanelApp> panel) {
        CloseDrawer();

        std::string name = "App";
        for (const auto &entry : Apps::List()) {
            if (entry.id == id) {
                name = entry.name;
                break;
            }
        }

        std::string tab_id = std::to_string(static_cast<int>(id));
        C2D_Image tab_icon = icon_dir;
        for (const auto &e : Apps::List()) {
            if (e.id == id) { tab_icon = e.icon; break; }
        }
        panel->Closed = [this, tab_id]() {
            NetLoader::Cancel();
            pending_tab_removals_.push_back(tab_id);
        };

        touch_bar_.AddTab(tab_id, name, tab_icon, std::move(panel), true);
    };

    drawer_ = std::move(drawer);
    UI::ScreenManager::Instance().WindowOpened();
    NetworkLogger::PrintfTagged("WM", "drawer open windows=%d", UI::ScreenManager::Instance().WindowCount());
}

void DesktopScreen::CloseDrawer() {
    touch_bar_.SetHomeActive(false);
    if (drawer_) {
        dying_panel_ = std::move(drawer_);
        UI::ScreenManager::Instance().WindowClosed();
        NetworkLogger::PrintfTagged("WM", "drawer closed windows=%d", UI::ScreenManager::Instance().WindowCount());
    }
}

}
