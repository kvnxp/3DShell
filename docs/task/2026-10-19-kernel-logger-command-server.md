# Task: Port kernel logger + debug command server

- Date: 2026-10-19
- Requested by: user
- Source: proven implementation at ../3dExplorer/src/kernel
- Status: in progress

## Tasks
- [x] Phase 1: port `NetworkSubsystem` (ref-counted socInit) to source/kernel/network.{h,cpp}
- [x] Phase 1: rewire shell `Net::Init/Exit` (net.cpp) to the shared subsystem
- [x] Phase 1: wire `NetworkLogger::Init` at boot (main.cpp) + key traces
- [x] Phase 2: debug command server (UDP 6000) based on NetLoginServer shape: Poll() from ScreenManager per frame, reply to command source
- [x] Phase 2: commands `ping`, `windows` (needs `UI::Screen::Name()`), `touch` (last touch state), `cfg` snapshot
- [x] Phase 2: logger.ts v2: both channels (5000 listen + command send to 6000)
- [x] Fix: desktop touch-bar X button now hidden when no window/panel is open (close_btn_ was never assigned in AddTextButton)
- [ ] Validate on hardware: logs flow, `windows` answers WM state (X-button debug)
