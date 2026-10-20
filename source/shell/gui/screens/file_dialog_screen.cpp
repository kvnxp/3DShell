#include "file_dialog_screen.h"

#include "c2d_helper.h"
#include "colours.h"
#include "config.h"
#include <screen_prompt.h>
#include "osk.h"
#include "screen_manager.h"

namespace GUI {

namespace {

constexpr float BAR_H = 20.0f;
constexpr float FOLDER_X = 4.0f;
constexpr float FOLDER_Y = 62.0f;
constexpr float FOLDER_W = 392.0f;
constexpr float FOLDER_H = 144.0f;
constexpr size_t FOLDER_ROWS = 8;
constexpr float LEFT_X = 4.0f;
constexpr float CONTENT_W = 312.0f;
constexpr float MESSAGE_Y = 130.0f;
constexpr float LABEL_TEXT_SIZE = 0.5f;
constexpr float FIELD_X = 56.0f;
constexpr float FIELD_W = 260.0f;
constexpr float FIELD_H = 22.0f;
constexpr float NAME_Y = 156.0f;
constexpr float TYPE_Y = 184.0f;
constexpr float BUTTON_Y = 214.0f;
constexpr float BUTTON_H = 24.0f;
constexpr float BUTTON_GAP = 4.0f;

std::string HeadText(const std::string &text, std::size_t max) {
    if (text.size() <= max)
        return text;
    return text.substr(0, max - 3) + "...";
}

std::string TailText(const std::string &text, std::size_t max) {
    if (text.size() <= max)
        return text;
    return "..." + text.substr(text.size() - (max - 3));
}

FolderView::Palette FolderPalette() {
    bool dark = cfg.dark_theme;
    return FolderView::Palette{
        dark? BLACK_BG : WHITE,
        dark? WHITE : BLACK,
        dark? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT,
        BAR_COLOUR,
        TITLE_COLOUR,
        dark,
    };
}

UI::ComboBox::Palette TypePalette() {
    bool dark = cfg.dark_theme;
    return UI::ComboBox::Palette{
        dark? BLACK_BG : WHITE,
        dark? WHITE : BLACK,
        dark? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT,
        dark? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT,
    };
}

std::vector<std::string> FilterLabels(const std::vector<FileDialogFilter> &filters) {
    std::vector<std::string> labels;
    for (const FileDialogFilter &filter : filters)
        labels.push_back(filter.label);
    return labels;
}

FileDialogOptions WithDefaultFilter(FileDialogOptions options) {
    if (options.filters.empty())
        options.filters.push_back(FileDialogFilter{"All files", {}});
    return options;
}

}

FileDialogScreen::FileDialogScreen(const FileDialogOptions &options)
    : options_(WithDefaultFilter(options)),
      folder_(UI::Rect{FOLDER_X, FOLDER_Y, FOLDER_W, FOLDER_H}, FOLDER_ROWS, FolderPalette()),
      type_box_(UI::Rect{FIELD_X, TYPE_Y, FIELD_W, FIELD_H}, FilterLabels(options_.filters), TypePalette(), true) {
    SetTopBackground(cfg.dark_theme? BLACK_BG : WHITE);
    SetBottomBackground(cfg.dark_theme? MENU_BAR_DARK : STATUS_BAR_LIGHT);

    if (Allows(FILE_DIALOG_SAVE))
        buttons_.push_back(BUTTON_SAVE);
    if (Allows(FILE_DIALOG_OPEN))
        buttons_.push_back(BUTTON_OPEN);
    if (Allows(FILE_DIALOG_SELECT_DIR))
        buttons_.push_back(BUTTON_SELECT);
    buttons_.push_back(BUTTON_CANCEL);

    type_box_.Changed = [this](int index) {
        filter_ = static_cast<size_t>(index);
        folder_.SetExtensions(options_.filters[filter_].extensions);
    };

    folder_.SetShowFiles(Allows(FILE_DIALOG_OPEN) || Allows(FILE_DIALOG_SAVE));
    folder_.SetExtensions(options_.filters[filter_].extensions);
    folder_.Changed = [this]() {
        name_.clear();
        message_.clear();
    };
    folder_.Failed = [this](const std::string &text) {
        Message(text);
    };
    folder_.FileChosen = [this](const std::string &name) {
        name_ = name;
        if (!Allows(FILE_DIALOG_SAVE))
            DoOpen();
    };

    std::string start = options_.start_dir.empty()? cfg.cwd : options_.start_dir;
    if (!folder_.Navigate(start))
        folder_.Navigate("/");
    name_ = options_.default_name;

    touch_bar_.SetSection(TouchBar::SECTION_NONE);
    touch_bar_.AddTextButton("X", ERROR_COLOUR, true).Click = [this]() {
        Cancel();
    };
}

void FileDialogScreen::EditName() {
    std::string text = OSK::GetText(name_, "File name");
    if (!text.empty())
        name_ = text;
    message_.clear();
}

void FileDialogScreen::RunButton(ButtonKind kind) {
    switch (kind) {
    case BUTTON_SAVE:
        DoSave();
        break;
    case BUTTON_OPEN:
        DoOpen();
        break;
    case BUTTON_SELECT:
        Finish(folder_.Cwd());
        break;
    case BUTTON_CANCEL:
        Cancel();
        break;
    }
}

void FileDialogScreen::RunPrimary() {
    for (ButtonKind kind : buttons_) {
        if (kind != BUTTON_CANCEL) {
            RunButton(kind);
            return;
        }
    }
    Cancel();
}

void FileDialogScreen::DoOpen() {
    const std::string name = name_;
    if (name.empty()) {
        Message("Choose a file to open.");
        return;
    }
    if ((name == ".") || (name == "..") || (name.find('/') != std::string::npos)) {
        Message("Invalid file name.");
        return;
    }

    std::string path = folder_.Cwd() + name;
    if (!FS::FileExists(archive, path)) {
        Message(FS::DirExists(archive, path + "/")? "Select a file, not a folder." : "File not found.");
        return;
    }

    Finish(path);
}

void FileDialogScreen::DoSave() {
    std::string name = name_;
    if (name.empty()) {
        Message("Enter a file name.");
        return;
    }
    if ((name == ".") || (name == "..") || (name.find('/') != std::string::npos)) {
        Message("Invalid file name.");
        return;
    }

    if (name.find('.') == std::string::npos)
        name += "." + DefaultExtension();
    name_ = name;

    std::string path = folder_.Cwd() + name;
    if (FS::DirExists(archive, path + "/")) {
        Message("A folder has this name.");
        return;
    }

    if (FS::FileExists(archive, path)) {
        ShowOverwritePrompt(path);
        return;
    }

    Finish(path);
}

void FileDialogScreen::Finish(const std::string &path) {
    const std::string result = path;
    auto callback = options_.callback;
    UI::ScreenManager::Instance().Pop();
    if (callback)
        callback(result);
}

void FileDialogScreen::Cancel() {
    Finish(std::string());
}

void FileDialogScreen::Message(const std::string &text) {
    message_ = text;
}

void FileDialogScreen::ShowOverwritePrompt(const std::string &path) {
    pending_path_ = path;
    std::string name = path.substr(path.find_last_of('/') + 1);
    prompt_ = Prompt::Create(
        "Replace file?",
        "\"" + HeadText(name, 30) + "\" already exists.\nDo you want to replace it?",
        std::vector<std::string>{"Yes", "No"});
    prompt_->SetFocus(1);
}

void FileDialogScreen::TakePromptResult() {
    if (!prompt_ || !prompt_->Done())
        return;

    int result = prompt_->Result();
    prompt_.reset();
    if (result == 0)
        Finish(pending_path_);
}

std::string FileDialogScreen::DefaultExtension() const {
    const std::vector<std::string> &exts = options_.filters[filter_].extensions;
    return exts.empty()? std::string("txt") : exts.front();
}

UI::Rect FileDialogScreen::NameFieldRect() const {
    return UI::Rect{FIELD_X, NAME_Y, FIELD_W, FIELD_H};
}

UI::Rect FileDialogScreen::ButtonRect(size_t index) const {
    float count = static_cast<float>(buttons_.size());
    float w = (CONTENT_W - BUTTON_GAP * (count - 1.0f)) / count;
    return UI::Rect{LEFT_X + static_cast<float>(index) * (w + BUTTON_GAP), BUTTON_Y, w, BUTTON_H};
}

void FileDialogScreen::OnFrame(u64 delta_time) {
    folder_.PumpIcons();
}

void FileDialogScreen::OnKeyDown(u32 k_down) {
    if (prompt_) {
        prompt_->OnKeyDown(k_down);
        TakePromptResult();
        return;
    }

    if (type_box_.IsOpen() && (k_down & KEY_B)) {
        type_box_.Close();
        return;
    }

    if (k_down & (KEY_UP | KEY_DOWN | KEY_A)) {
        folder_.OnKeyDown(k_down);
    } else if (k_down & KEY_B) {
        if (!folder_.GoParent())
            Cancel();
    } else if (k_down & KEY_X) {
        if (NameRowVisible())
            EditName();
    } else if (k_down & KEY_L) {
        if (NameRowVisible())
            type_box_.Step(-1);
    } else if (k_down & KEY_R) {
        if (NameRowVisible())
            type_box_.Step(1);
    } else if (k_down & KEY_START) {
        RunPrimary();
    }
}

/* Touch acts on the press edge, so a tap that opened the dialog cannot also
 * press a control. The dialog ignores touches until the finger is released once. */
void FileDialogScreen::OnTouch(const touchPosition &touch, bool down) {
    if (!touch_armed_) {
        if (!down)
            touch_armed_ = true;
        return;
    }

    if (prompt_) {
        prompt_->OnTouch(touch, down);
        TakePromptResult();
        return;
    }

    bool in_bar = down? (touch.py < BAR_H) : bar_touching_;
    if (in_bar) {
        touch_bar_.OnTouch(touch, down);
        bar_touching_ = down;
        return;
    }

    if (NameRowVisible()) {
        bool open_before = type_box_.IsOpen();
        type_box_.OnTouch(touch, down);
        if (open_before || type_box_.IsOpen())
            return;
    }

    if (down && !touching_)
        HandleTap(touch);
    touching_ = down;
}

void FileDialogScreen::HandleTap(const touchPosition &touch) {
    if (NameRowVisible() && NameFieldRect().Contains(touch)) {
        EditName();
        return;
    }

    for (size_t i = 0; i < buttons_.size(); i++) {
        if (ButtonRect(i).Contains(touch)) {
            RunButton(buttons_[i]);
            return;
        }
    }
}

void FileDialogScreen::DrawTop() {
    bool dark = cfg.dark_theme;
    status_bar_.Draw();
    C2D::Rect(0, 15, 400, 25, dark? MENU_BAR_DARK : MENU_BAR_LIGHT);
    C2D::Text(8, 19, 0.5f, WHITE, options_.title);

    C2D::Text(8, 44, 0.45f, dark? WHITE : BLACK, "Folder: " + TailText(folder_.Cwd(), 48));
    folder_.Draw();

    C2D::Text(8, 212, 0.4f, dark? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT,
              "UP/DOWN select   A open   B up folder");
    C2D::Text(8, 224, 0.4f, dark? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT,
              "X edit name   L/R file type   START confirm");
}

void FileDialogScreen::DrawBottom() {
    bool dark = cfg.dark_theme;
    u32 text_colour = dark? WHITE : BLACK;
    u32 field_colour = dark? BLACK_BG : WHITE;
    u32 border_colour = dark? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT;

    touch_bar_.Draw();

    if (!message_.empty())
        C2D::Text(LEFT_X + 4.0f, MESSAGE_Y, 0.45f, ERROR_COLOUR, message_);

    if (NameRowVisible()) {
        C2D::Text(LEFT_X + 4.0f, NAME_Y + 4.0f, LABEL_TEXT_SIZE, text_colour, "Name");
        UI::Rect field = NameFieldRect();
        C2D::Rect(field.x - 1.0f, field.y - 1.0f, field.w + 2.0f, field.h + 2.0f, border_colour);
        C2D::Rect(field.x, field.y, field.w, field.h, field_colour);
        C2D::Text(field.x + 4.0f, field.y + 4.0f, LABEL_TEXT_SIZE, text_colour, TailText(name_, 36));

        C2D::Text(LEFT_X + 4.0f, TYPE_Y + 4.0f, LABEL_TEXT_SIZE, text_colour, "Type");
    }

    for (size_t i = 0; i < buttons_.size(); i++) {
        UI::Rect r = ButtonRect(i);
        C2D::Rect(r.x, r.y, r.w, r.h, dark? MENU_BAR_DARK : MENU_BAR_LIGHT);

        const char *label = "";
        switch (buttons_[i]) {
        case BUTTON_SAVE:   label = "Save"; break;
        case BUTTON_OPEN:   label = "Open"; break;
        case BUTTON_SELECT: label = "Select"; break;
        case BUTTON_CANCEL: label = "Cancel"; break;
        }

        float tw = 0.0f;
        float th = 0.0f;
        C2D::GetTextSize(LABEL_TEXT_SIZE, &tw, &th, label);
        C2D::Text(r.x + (r.w - tw) / 2.0f, r.y + (r.h - th) / 2.0f, LABEL_TEXT_SIZE, WHITE, label);
    }

    if (NameRowVisible())
        type_box_.Draw();

    if (prompt_)
        prompt_->Draw();
}

}
