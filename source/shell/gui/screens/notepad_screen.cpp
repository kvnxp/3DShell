#include "notepad_screen.h"

#include "c2d_helper.h"
#include "colours.h"
#include "config.h"
#include "touch.h"

namespace GUI {

NotepadScreen::NotepadScreen() {
    SetTopBackground(cfg.dark_theme? BLACK_BG : WHITE);
    SetBottomBackground(cfg.dark_theme? MENU_BAR_DARK : STATUS_BAR_LIGHT);

    Notepad::NewDocument();

    touch_bar_.SetSection(TouchBar::SECTION_NONE);
    touch_bar_.AddButton(TouchBar::ICON_HOME, false).Click = [this]() {
        session_.RequestClose();
    };

    /* Window close X (Windows Mobile style): closes the editor. */
    touch_bar_.AddTextButton("X", ERROR_COLOUR, true).Click = [this]() {
        session_.RequestClose();
    };
}

void NotepadScreen::OnFrame(u64 delta_time) {
    (void)delta_time;

    if (!UI::ScreenManager::Instance().IsCurrent(this))
        return;

    /* The document was closed -> leave the app. */
    if (!Notepad::IsOpen())
        UI::ScreenManager::Instance().Pop();
}

void NotepadScreen::DrawTop() {
    status_bar_.Draw();
    C2D::Rect(0, 15, 400, 25, cfg.dark_theme? MENU_BAR_DARK : MENU_BAR_LIGHT);
    Notepad::DisplayTop();
}

void NotepadScreen::DrawBottom() {
    touch_bar_.Draw();
    Notepad::DisplayBottom();
    session_.Draw();
}

void NotepadScreen::OnTouch(const touchPosition &touch, bool down) {
    if (session_.Busy()) {
        session_.Touch(touch, down);
        return;
    }

    bool in_bar = down? (touch.py < 20) : bar_touching_;

    if (in_bar) {
        touch_bar_.OnTouch(touch, down);
        bar_touching_ = down;
    }
}

void NotepadScreen::OnKeyHeld(u32 k_held) {
    k_held_ = k_held;
    Touch::Update();
}

void NotepadScreen::OnKeyDown(u32 k_down) {
    session_.KeyDown(k_down, k_held_);
}

}
