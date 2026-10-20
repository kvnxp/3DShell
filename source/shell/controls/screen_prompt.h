#ifndef _3D_SHELL_SCREEN_PROMPT_H
#define _3D_SHELL_SCREEN_PROMPT_H

#include <memory>
#include <string>
#include <vector>

#include "c2d_helper.h"
#include "colours.h"
#include "config.h"
#include "controls/message_box.h"

namespace Prompt {

/* Themed modal prompt covering the whole bottom screen. */
inline std::unique_ptr<UI::MessageBox> Create(const std::string &title, const std::string &text,
                                              const std::vector<std::string> &buttons) {
    bool dark = cfg.dark_theme;
    UI::MessageBox::Palette palette{
        C2D_Color32(0, 0, 0, 160),
        dark ? BLACK_BG : WHITE,
        dark ? TITLE_COLOUR_DARK : TITLE_COLOUR,
        dark ? WHITE : BLACK,
        dark ? MENU_BAR_DARK : MENU_BAR_LIGHT,
        WHITE,
        BAR_COLOUR,
    };
    return std::make_unique<UI::MessageBox>(title, text, buttons,
                                            UI::Rect{0.0f, 0.0f, 320.0f, 240.0f}, palette);
}

}

#endif
