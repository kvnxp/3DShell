# Task: Notepad save flow, reusable file dialog, and unsaved-changes prompts

- Date: 2026-10-20
- Requested by: user
- Status: completed

## Tasks
- [x] Remove the FILES icon and OpenFile() from Notepad (touch bar and header)
- [x] Add Notepad::IsNewDocument() (empty filepath = blank document)
- [x] Build a reusable Yes/No confirm control in source/ui/controls/
- [x] Build a ComboBox (dropdown) control in source/ui/controls/
- [x] Build a reusable FileDialog in source/shell/ with per-caller allowed actions (Open, Save, Select directory) and file-type filters
- [x] Wire Save on a blank document to the FileDialog (Save mode)
- [x] Ask before replacing an existing file name
- [x] Ask before leaving a document with unsaved changes (HOME, X, and VIEW/EDIT B exit paths, plus the text reader)
- [x] Build with DEVKITPRO=/opt/devkitpro make -j8
- [x] Update docs/TEXT_EDITOR.md (Spanish) for the new Save and dialog behavior
- [x] Update README.md and docs/ui-components.md for the new Notepad save flow and controls
- [x] Move this file to docs/task/completed/ when every item is done
