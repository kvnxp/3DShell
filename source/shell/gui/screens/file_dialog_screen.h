#ifndef _3D_SHELL_FILE_DIALOG_SCREEN_H
#define _3D_SHELL_FILE_DIALOG_SCREEN_H

#include "gui.h"

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include <folder_view.h>
#include <status_bar.h>
#include <touch_bar.h>
#include "controls/combo_box.h"
#include "controls/message_box.h"
#include "fs.h"
#include "screen.h"
#include "textures.h"

namespace GUI {

enum FileDialogAction : u32 {
    FILE_DIALOG_OPEN = 1,
    FILE_DIALOG_SAVE = 2,
    FILE_DIALOG_SELECT_DIR = 4,
};

/* Extensions are without the leading dot. An empty list shows every file. */
struct FileDialogFilter {
    std::string label;
    std::vector<std::string> extensions;
};

struct FileDialogOptions {
    std::string title;
    /* Bit mask of FileDialogAction values. Each caller picks what it allows. */
    u32 actions = FILE_DIALOG_OPEN;
    /* Empty = current working directory. */
    std::string start_dir;
    std::string default_name;
    /* Empty = a single "All files" filter. */
    std::vector<FileDialogFilter> filters;
    /* Receives the chosen full path, or an empty string when cancelled. */
    std::function<void(const std::string &)> callback;
};

/* Reusable Open / Save / Select folder dialog. It pops itself before calling
 * the callback. */
class FileDialogScreen : public UI::Screen {
public:
    explicit FileDialogScreen(const FileDialogOptions &options);

    const char *Name() const override { return "file_dialog"; }
    const C2D_Image *Icon(bool dark_theme) const override { return dark_theme ? &dialog_dark : &dialog; }

    void DrawTop() override;
    void DrawBottom() override;
    void OnKeyDown(u32 k_down) override;
    void OnTouch(const touchPosition &touch, bool down) override;

private:
    enum ButtonKind { BUTTON_SAVE, BUTTON_OPEN, BUTTON_SELECT, BUTTON_CANCEL };

    bool Allows(FileDialogAction action) const { return (options_.actions & action) != 0; }
    bool NameRowVisible() const { return !(options_.actions == FILE_DIALOG_SELECT_DIR); }

    void EditName();
    void RunButton(ButtonKind kind);
    void RunPrimary();
    void DoOpen();
    void DoSave();
    void Finish(const std::string &path);
    void Cancel();
    void Message(const std::string &text);
    void ShowOverwritePrompt(const std::string &path);
    void TakePromptResult();
    void HandleTap(const touchPosition &touch);
    std::string DefaultExtension() const;

    UI::Rect NameFieldRect() const;
    UI::Rect ButtonRect(size_t index) const;

    FileDialogOptions options_;
    std::vector<ButtonKind> buttons_;
    std::string name_;
    std::string message_;
    std::string pending_path_;
    size_t filter_ = 0;

    bool touch_armed_ = false;
    bool touching_ = false;
    bool bar_touching_ = false;

    std::unique_ptr<UI::MessageBox> prompt_;
    FolderView folder_;
    UI::ComboBox type_box_;
    StatusBar status_bar_;
    TouchBar touch_bar_;
};

}

#endif
