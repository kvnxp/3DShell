#ifndef _3D_SHELL_NOTEPAD_SESSION_H
#define _3D_SHELL_NOTEPAD_SESSION_H

#include <memory>

#include "file_dialog_screen.h"
#include "notepad.h"
#include "screen_manager.h"
#include "screen_prompt.h"

namespace GUI {

/* Save and leave rules shared by the Notepad screens. A blank document is
 * saved through the file dialog, and leaving with unsaved changes asks first. */
class NotepadSession {
public:
    bool Busy() const { return prompt_ != nullptr; }

    void Action(NotepadAction action) {
        if (action == NOTEPAD_ACTION_SAVE)
            Save(false);
        else if (action == NOTEPAD_ACTION_CLOSE)
            RequestClose();
    }

    void RequestClose() {
        if (!Notepad::IsModified()) {
            Notepad::Close();
            return;
        }
        prompt_ = Prompt::Create("Unsaved changes", "Save changes before closing?",
                                 {"Save", "Discard", "Cancel"});
        prompt_->SetFocus(CHOICE_SAVE);
    }

    void KeyDown(u32 k_down, u32 k_held) {
        if (prompt_) {
            prompt_->OnKeyDown(k_down);
            TakePromptResult();
            return;
        }
        Action(Notepad::Control(k_down, k_held));
    }

    void Touch(const touchPosition &touch, bool down) {
        if (!prompt_)
            return;
        prompt_->OnTouch(touch, down);
        TakePromptResult();
    }

    void Draw() {
        if (prompt_)
            prompt_->Draw();
    }

private:
    enum PromptChoice { CHOICE_SAVE = 0, CHOICE_DISCARD = 1, CHOICE_CANCEL = 2 };

    void Save(bool close_after) {
        if (Notepad::IsNewDocument()) {
            OpenSaveDialog(close_after);
            return;
        }
        if (Notepad::Save() && close_after)
            Notepad::Close();
    }

    void TakePromptResult() {
        if (!prompt_ || !prompt_->Done())
            return;

        int result = prompt_->Result();
        prompt_.reset();

        if (result == CHOICE_SAVE)
            Save(true);
        else if (result == CHOICE_DISCARD)
            Notepad::Close();
    }

    void OpenSaveDialog(bool close_after) {
        FileDialogOptions options;
        options.title = "Save As";
        options.actions = FILE_DIALOG_SAVE;
        options.default_name = Notepad::GetData().filename;
        options.filters = {
            {"All files", {}},
            {"txt", {"txt"}},
            {"log", {"log"}},
            {"json", {"json"}},
        };
        options.callback = [close_after](const std::string &path) {
            if (path.empty())
                return;
            if (Notepad::SaveTo(path) && close_after)
                Notepad::Close();
        };
        UI::ScreenManager::Instance().Push(std::make_unique<FileDialogScreen>(options));
    }

    std::unique_ptr<UI::MessageBox> prompt_;
};

}

#endif
