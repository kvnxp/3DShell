# Task: Remove START button app-exit behavior

- Date: 2026-10-18
- Requested by: user
- Status: completed

## Tasks
- [x] Locate every KEY_START handler that calls ScreenManager::Exit()
- [x] Remove the START-to-exit handling in each location
- [x] Verify no remaining START-to-exit path (including notepad)
- [x] Verify build
