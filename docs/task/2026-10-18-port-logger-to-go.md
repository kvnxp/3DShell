# Task: Port logger.ts debug console to Go

- Date: 2026-10-18
- Requested by: user
- Status: completed

## Tasks
- [x] Set up Go module under `logger/` (tview v0.42 / tcell)
- [x] Port UDP log listener (port 5000) with bind-retry logic
- [x] Port TUI layout: log box, devices panel, command input, status bar
- [x] Port device discovery/selection (TAB+Enter, and mouse click via tview click = Selected)
- [x] Port command sending (UDP to port 6000, responses land on log socket)
- [x] Verify build and run parity (E2E: fake console over UDP + reply + exit flow)

## Notes
- Verified end-to-end with automated fake console: bind, discovery, log render, target selection, command send/receive, response logging, exit modal.
- A real LAN device (192.168.0.104) was captured during testing and handled correctly.
- 2 real bugs found and fixed during verification: tview QueueUpdateDraw deadlock (app.Draw inside SetChangedFunc -> Use ForceDraw) and global key capture swallowing Tab.
- `logger.ts` kept as reference (untracked); delete when the Go version is confirmed in real use.

## Run
```
cd logger && go run . [ip3ds]
```
