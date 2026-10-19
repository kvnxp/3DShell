#ifndef _3D_SHELL_TOUCH_BAR_H
#define _3D_SHELL_TOUCH_BAR_H

#include <3ds.h>

#include <memory>
#include <string>
#include <vector>

#include <config.h>
#include <fs.h>
#include <textures.h>

#include <container_control.h>
#include <layout.h>
#include <screen_manager.h>

#include "controls/button.h"
#include "panel_app.h"
#include "touch_button.h"
#include <c2d_helper.h>
#include <colours.h>

namespace GUI {

/* Tab button: a text button that can show an active (pressed) state. */
class TabButton : public UI::Button {
public:
    TabButton(const std::string &text, u32 background, u32 text_colour)
        : UI::Button(text, UI::Rect{}, background, text_colour) {}

    void SetActive(bool active) { active_ = active; }

    void Draw() override {
        UI::Button::Draw();
        if (active_)
            C2D::Rect(bounds_.x, bounds_.y, bounds_.w, bounds_.h, C2D_Color32(255, 255, 255, 70));
    }

private:
    bool active_ = false;
};

/* Bottom screen chrome: background bar + icon buttons laid out in two
 * groups. Screens add the buttons they need with AddButton(): the LEFT
 * group flows from the left edge, the RIGHT group is anchored to the
 * right edge. Invisible buttons are skipped by the layout, so the group
 * re-packs. Each button fires its own Click event on release. */
class TouchBar : public UI::ContainerControl {
public:
    struct Tab {
        std::string id;
        std::string label;
        std::unique_ptr<GUI::PanelApp> content;
        GUI::TouchButton *button = nullptr;
        bool right_group = false;
    };

    enum TouchBarIcon {
        ICON_HOME,
        ICON_FILES,
        ICON_NETLOADER,
        ICON_OPTIONS,
        ICON_SETTINGS,
        ICON_SD,
        ICON_NAND,
        ICON_SEARCH
    };

    enum Section {
        SECTION_NONE,
        SECTION_BROWSER,
        SECTION_ACTIONS,
        SECTION_SETTINGS
    };

    TouchBar() {
        bounds_ = UI::Rect{0.0f, 0.0f, 320.0f, 20.0f};
    }

    TouchButton &AddButton(TouchBarIcon icon, bool right_group) {
        C2D_Image normal, dark, overlay;

        switch (icon) {
            case ICON_HOME:
                normal = icon_home; dark = icon_home_dark; overlay = icon_home_overlay;
                break;
            case ICON_FILES:
                normal = icon_dir; dark = icon_dir_dark; overlay = icon_dir;
                break;
            case ICON_NETLOADER:
                normal = icon_ftp; dark = icon_ftp_dark; overlay = icon_ftp;
                break;
            case ICON_OPTIONS:
                normal = icon_options; dark = icon_options_dark; overlay = icon_options_overlay;
                break;
            case ICON_SETTINGS:
                normal = icon_settings; dark = icon_settings_dark; overlay = icon_settings_overlay;
                break;
            case ICON_SD:
                normal = icon_sd; dark = icon_sd_dark; overlay = icon_sd_overlay;
                break;
            case ICON_NAND:
                normal = icon_secure; dark = icon_secure_dark; overlay = icon_secure_overlay;
                break;
            case ICON_SEARCH:
            default:
                normal = icon_search; dark = icon_search; overlay = icon_search;
                break;
        }

        auto button = std::make_unique<TouchButton>(normal, dark, overlay);
        TouchButton &ref = *button;

        UI::FlowLayout::Item item{&ref, ref.IntrinsicWidth()};
        if (right_group)
            right_items_.push_back(item);
        else
            left_items_.push_back(item);

        Add(std::move(button));

        switch (icon) {
            case ICON_HOME:     home_ = &ref;     break;
            case ICON_OPTIONS:  options_ = &ref;  break;
            case ICON_SETTINGS: settings_ = &ref; break;
            case ICON_SD:       sd_ = &ref;      break;
            case ICON_NAND:     secure_ = &ref;   break;
            default: break;
        }

        return ref;
    }

    /* Text-based button for the bar (e.g. the window close X). Added last
     * in a group it lands rightmost. */
    UI::Button &AddTextButton(const std::string &text, u32 background, bool right_group) {
        auto button = std::make_unique<UI::Button>(text, UI::Rect{}, background, WHITE);
        UI::Button &ref = *button;

        UI::FlowLayout::Item item{&ref, BUTTON_SIZE};
        if (right_group)
            right_items_.push_back(item);
        else
            left_items_.push_back(item);

        Add(std::move(button));
        close_btn_ = &ref;
        return ref;
    }

    void Draw() override {
        C2D::Rect(bounds_.x, bounds_.y, bounds_.w, bounds_.h, cfg.dark_theme? STATUS_BAR_DARK : MENU_BAR_LIGHT);

        if (home_)
            home_->SetActive(home_active_);
        if (options_)
            options_->SetActive(section_ == SECTION_ACTIONS);
        if (settings_)
            settings_->SetActive(section_ == SECTION_SETTINGS);
        if (sd_)
            sd_->SetActive(archive == sdmc_archive);
        if (secure_) {
            /* NAND access is a developer-only feature: hide the button
             * entirely instead of showing one that does nothing. */
            secure_->SetVisible(cfg.dev_options);
            secure_->SetActive(archive == nand_archive);
        }

        for (auto &t : tabs_) {
            if (t.button) {
                t.button->SetActive(t.id == active_tab_);
            }
        }

        /* Keep active tab in sync with the tab list so a stale id can never
         * keep the close button visible after every tab is gone. */
        if (tabs_.empty())
            active_tab_.clear();

        /* The close X answers to the window manager, not to each screen:
         * it exists while any window (fullscreen app or panel app) is open. */
        if (close_btn_) {
            close_btn_->SetVisible(close_visible_ || (close_use_wm_ && UI::ScreenManager::Instance().HasWindow()) || !active_tab_.empty());
        }

        Reflow();

        ContainerControl::Draw();
    }

    void SetSection(Section section) { section_ = section; }
    void SetHomeActive(bool active) { home_active_ = active; }
    void SetCloseVisible(bool visible) { close_visible_ = visible; }
    void SetCloseUseWM(bool use) { close_use_wm_ = use; }

    /* Tabs: buttons that show an associated PanelApp content below the bar. */
    Tab &AddTab(const std::string &id, const std::string &label, C2D_Image icon,
                std::unique_ptr<GUI::PanelApp> content, bool right_group) {
        for (auto &t : tabs_) {
            if (t.id == id) {
                SetActiveTab(id);
                return t;
            }
        }

        auto btn = std::make_unique<GUI::TouchButton>(icon, icon, icon);
        GUI::TouchButton *btnPtr = btn.get();

        float tab_w = btnPtr->IntrinsicWidth();
        if (tab_w < BUTTON_SIZE) tab_w = BUTTON_SIZE;

        UI::FlowLayout::Item item{btnPtr, tab_w};
        if (right_group) {
            /* Keep the system close (X) button rightmost: insert tabs
             * before it in the right group. */
            bool inserted = false;
            if (close_btn_) {
                for (size_t i = 0; i < right_items_.size(); ++i) {
                    if (right_items_[i].control == close_btn_) {
                        right_items_.insert(right_items_.begin() + i, item);
                        inserted = true;
                        break;
                    }
                }
            }
            if (!inserted) {
                right_items_.push_back(item);
            }
        } else {
            left_items_.push_back(item);
        }

        Add(std::move(btn));

        tabs_.push_back(Tab{id, label, std::move(content), btnPtr, right_group});
        Tab &tab = tabs_.back();

        btnPtr->Click = [this, id]() { SetActiveTab(id); };

        SetActiveTab(id);

        UI::ScreenManager::Instance().WindowOpened();
        return tab;
    }

    void RemoveTab(const std::string &id) {
        for (size_t i = 0; i < tabs_.size();) {
            if (tabs_[i].id == id) {
                GUI::TouchButton *btn = tabs_[i].button;

                auto removeItem = [btn](std::vector<UI::FlowLayout::Item> &v) {
                    for (size_t j = 0; j < v.size();) {
                        if (v[j].control == btn) {
                            v.erase(v.begin() + j);
                        } else {
                            ++j;
                        }
                    }
                };
                removeItem(left_items_);
                removeItem(right_items_);

                Remove(btn);

                if (active_tab_ == id) {
                    active_tab_.clear();
                    for (auto &t : tabs_) {
                        if (t.id != id) {
                            SetActiveTab(t.id);
                            break;
                        }
                    }
                }

                tabs_.erase(tabs_.begin() + i);
                UI::ScreenManager::Instance().WindowClosed();
            } else {
                ++i;
            }
        }
    }

    void SetActiveTab(const std::string &id) {
        active_tab_ = id;
        for (auto &t : tabs_) {
            if (t.button) {
                t.button->SetActive(t.id == id);
            }
        }
    }

    const std::string &ActiveTabId() const { return active_tab_; }

    GUI::PanelApp *ActiveContent() const {
        for (const auto &t : tabs_) {
            if (t.id == active_tab_) {
                return t.content.get();
            }
        }
        return nullptr;
    }

    const std::vector<Tab> &Tabs() const { return tabs_; }
    bool HasTab(const std::string &id) const {
        for (const auto &t : tabs_) if (t.id == id) return true;
        return false;
    }

private:
    void Reflow() {
        const float y = bounds_.y;
        const float height = bounds_.h;
        const float margin = 2.0f;
        const float gap = 4.0f;

        UI::FlowLayout::Flow(left_items_, y, height, bounds_.x + margin, gap, false);
        UI::FlowLayout::Flow(right_items_, y, height, bounds_.x + bounds_.w - margin, gap, true);
    }

    Section section_ = SECTION_NONE;
    bool home_active_ = false;
    bool close_visible_ = false;
    bool close_use_wm_ = true;
    UI::Button *close_btn_ = nullptr;
    TouchButton *home_ = nullptr;
    TouchButton *options_ = nullptr;
    TouchButton *settings_ = nullptr;
    TouchButton *sd_ = nullptr;
    TouchButton *secure_ = nullptr;
    std::vector<UI::FlowLayout::Item> left_items_;
    std::vector<UI::FlowLayout::Item> right_items_;
    std::vector<Tab> tabs_;
    std::string active_tab_;
    static constexpr float BUTTON_SIZE = 20.0f;
};

}

#endif
