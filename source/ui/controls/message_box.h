#ifndef _3D_SHELL_UI_MESSAGE_BOX_H
#define _3D_SHELL_UI_MESSAGE_BOX_H

#include <string>
#include <vector>

#include "../control.h"
#include "../c2d_helper.h"

namespace UI {

// Modal prompt drawn over its bounds. The owner routes input to it while it is
// shown, polls Done() after each input, and then reads Result().
class MessageBox : public Control {
public:
    static constexpr int RESULT_NONE = -2;
    static constexpr int RESULT_CANCEL = -1;

    struct Palette {
        u32 backdrop;
        u32 panel;
        u32 title;
        u32 text;
        u32 button;
        u32 button_text;
        u32 focus;
    };

    MessageBox(const std::string &title, const std::string &text,
               const std::vector<std::string> &buttons, const Rect &bounds, const Palette &palette)
        : title_(title), buttons_(buttons), palette_(palette) {
        bounds_ = bounds;
        lines_ = SplitLines(text);
    }

    void Draw() override {
        C2D::Rect(bounds_.x, bounds_.y, bounds_.w, bounds_.h, palette_.backdrop);

        float px = bounds_.x + (bounds_.w - PANEL_W) / 2.0f;
        float py = bounds_.y + (bounds_.h - PanelHeight()) / 2.0f;
        C2D::Rect(px, py, PANEL_W, PanelHeight(), palette_.panel);
        C2D::Rect(px, py, PANEL_W, TITLE_H, palette_.title);
        C2D::Text(px + PADDING, py + 4.0f, TEXT_SIZE, palette_.button_text, title_);

        float ty = py + TITLE_H + PADDING;
        for (const std::string &line : lines_) {
            C2D::Text(px + PADDING, ty, TEXT_SIZE, palette_.text, line);
            ty += LINE_H;
        }

        for (size_t i = 0; i < buttons_.size(); i++) {
            Rect r = ButtonRect(i);
            C2D::Rect(r.x, r.y, r.w, r.h, palette_.button);
            if (static_cast<int>(i) == focus_) {
                C2D::Rect(r.x, r.y, r.w, 2.0f, palette_.focus);
                C2D::Rect(r.x, r.y + r.h - 2.0f, r.w, 2.0f, palette_.focus);
                C2D::Rect(r.x, r.y, 2.0f, r.h, palette_.focus);
                C2D::Rect(r.x + r.w - 2.0f, r.y, 2.0f, r.h, palette_.focus);
            }

            float tw = 0.0f;
            float th = 0.0f;
            C2D::GetTextSize(TEXT_SIZE, &tw, &th, buttons_[i]);
            C2D::Text(r.x + (r.w - tw) / 2.0f, r.y + (r.h - th) / 2.0f,
                      TEXT_SIZE, palette_.button_text, buttons_[i]);
        }
    }

    void OnKeyDown(u32 k_down) override {
        if (done_ || buttons_.empty())
            return;

        if (k_down & KEY_LEFT) {
            if (focus_ > 0)
                focus_--;
        } else if (k_down & KEY_RIGHT) {
            if (focus_ < static_cast<int>(buttons_.size()) - 1)
                focus_++;
        } else if (k_down & KEY_A) {
            Finish(focus_);
        } else if (k_down & KEY_B) {
            Finish(RESULT_CANCEL);
        }
    }

    // Ignores touches until the finger has been released once, so a tap that
    // opened the box cannot also press one of its buttons.
    void OnTouch(const touchPosition &touch, bool down) override {
        if (done_)
            return;

        if (!down) {
            armed_ = true;
            touching_ = false;
            return;
        }

        if (touching_ || !armed_) {
            touching_ = true;
            return;
        }
        touching_ = true;

        for (size_t i = 0; i < buttons_.size(); i++) {
            if (ButtonRect(i).Contains(touch)) {
                focus_ = static_cast<int>(i);
                Finish(focus_);
                return;
            }
        }
    }

    bool Done() const { return done_; }
    int Result() const { return done_ ? result_ : RESULT_NONE; }

    void SetFocus(int index) {
        if (index >= 0 && index < static_cast<int>(buttons_.size()))
            focus_ = index;
    }

private:
    static constexpr float PANEL_W = 280.0f;
    static constexpr float TITLE_H = 22.0f;
    static constexpr float PADDING = 10.0f;
    static constexpr float LINE_H = 16.0f;
    static constexpr float BUTTON_H = 30.0f;
    static constexpr float BUTTON_GAP = 8.0f;
    static constexpr float TEXT_SIZE = 0.5f;

    static std::vector<std::string> SplitLines(const std::string &text) {
        std::vector<std::string> lines;
        std::string current;
        for (char c : text) {
            if (c == '\n') {
                lines.push_back(current);
                current.clear();
            } else {
                current += c;
            }
        }
        lines.push_back(current);
        return lines;
    }

    float PanelHeight() const {
        return TITLE_H + PADDING + LINE_H * static_cast<float>(lines_.size()) + PADDING
             + BUTTON_H + PADDING;
    }

    Rect ButtonRect(size_t index) const {
        float px = bounds_.x + (bounds_.w - PANEL_W) / 2.0f;
        float py = bounds_.y + (bounds_.h - PanelHeight()) / 2.0f;
        float count = static_cast<float>(buttons_.size());
        float inner = PANEL_W - PADDING * 2.0f;
        float w = (inner - BUTTON_GAP * (count - 1.0f)) / count;

        Rect r;
        r.x = px + PADDING + static_cast<float>(index) * (w + BUTTON_GAP);
        r.y = py + PanelHeight() - BUTTON_H - PADDING;
        r.w = w;
        r.h = BUTTON_H;
        return r;
    }

    void Finish(int result) {
        result_ = result;
        done_ = true;
    }

    std::string title_;
    std::vector<std::string> lines_;
    std::vector<std::string> buttons_;
    Palette palette_;
    int focus_ = 0;
    int result_ = RESULT_NONE;
    bool done_ = false;
    bool armed_ = false;
    bool touching_ = false;
};

}

#endif
