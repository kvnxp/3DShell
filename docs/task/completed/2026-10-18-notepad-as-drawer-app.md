# Task: Register Notepad as a regular app in the app drawer

- Date: 2026-10-18
- Requested by: user
- Status: completed

Decisions (confirmed by user):
- Drawer launch opens a blank untitled document and also offers an "Open" action that goes to File Explorer.
- Keep the existing File Explorer -> text file flow working.

## Tasks
- [x] Inspect how drawer apps are registered and launched (include/apps.h, app_drawer_panel)
- [x] Inspect how Notepad is currently opened and its entry points
- [x] Add `Notepad::NewDocument()` (blank untitled buffer, edit mode)
- [x] Make Save on an untitled document ask for a file name (OSK) and reject existing names
- [x] Add `GUI::NotepadScreen` (fullscreen drawer app) with HOME/X touch bar and an "Open" action to File Explorer
- [x] Register Notepad in `include/apps.h` with a drawer icon
- [x] Keep File Explorer -> text file flow unchanged
- [x] Verify build with `make`
