// 3DShell debug console (tview TUI).
// Left: kernel/network log stream (any console broadcasting on the LAN).
// Right: devices panel — click an IP to select it as the command target.
// Bottom: command line; commands go to the selected device's debug port.
//
// Usage: go run . [ip3ds]
package main

import (
	"errors"
	"fmt"
	"net"
	"os"
	"strings"
	"syscall"
	"time"

	"github.com/gdamore/tcell/v2"
	"github.com/rivo/tview"
)

const logPort = 5000 // console -> PC: log broadcasts
const cmdPort = 6000 // PC -> console: debug commands
const retryMs = 2000

var (
	targetIp    string
	seenDevices = make(map[string]struct{})

	app    *tview.Application
	logBox *tview.TextView
	list   *tview.List
	input  *tview.InputField
	status *tview.TextView
)

var root tview.Primitive

func logLine(line string) {
	stamp := time.Now().Format("15:04:05")
	fmt.Fprintf(logBox, "[cyan][%s][-] %s\n", stamp, line)
}

func refreshDevices() {
	list.Clear()
	found := false
	for ip := range seenDevices {
		found = true
		label := ip
		if ip == targetIp {
			label = fmt.Sprintf("[green]%s ◀[-]", ip)
		}
		ip := ip
		list.AddItem(label, "", 0, func() { selectDevice(ip) })
	}
	if !found {
		list.AddItem("[gray]no consoles seen[-]", "", 0, nil)
	}
}

func selectDevice(ip string) {
	targetIp = ip
	logLine(fmt.Sprintf("[magenta]target set: %s[-]", ip))
	status.SetText(statusText())
	refreshDevices()
	app.SetFocus(input)
}

func statusText() string {
	return fmt.Sprintf(
		" logs :%d  commands :%d  target %s  (f10: exit)",
		logPort, cmdPort, map[bool]string{true: targetIp, false: "<click a device to select>"}[targetIp != ""],
	)
}

// sendCommand queues the command to a goroutine so the UI thread is never
// blocked by the kernel's ARP/neighbor timeout on unreachable targets
// (observed on macOS: sendto blocks the caller for a few seconds).
// The response lands on the bound log socket and is printed by readLogs.
func sendCommand(cmd string) {
	text := strings.TrimSpace(cmd)
	if text == "" {
		return
	}
	if targetIp == "" {
		logLine("[red]no target - click an IP in the devices panel first[-]")
		return
	}
	logLine(fmt.Sprintf("[yellow][%s] $ %s[-]", targetIp, text))

	ip := targetIp
	payload := []byte(text + "\n")
	go func() {
		addr := &net.UDPAddr{IP: net.ParseIP(ip), Port: cmdPort}
		_, err := server.WriteToUDP(payload, addr)
		if err != nil {
			app.QueueUpdateDraw(func() {
				logLine(fmt.Sprintf("[red]send failed: %v[-]", err))
			})
		}
	}()
}

// --- network -----------------------------------------------------------------

var server *net.UDPConn

func bindServer() {
	ln, err := net.ListenUDP("udp4", &net.UDPAddr{Port: logPort})
	if err != nil {
		if errors.Is(err, syscall.EADDRNOTAVAIL) {
			logLine(fmt.Sprintf("[red]bind unavailable - retrying in %ds[-]", retryMs/1000))
			time.AfterFunc(retryMs, bindServer)
			return
		}
		logLine(fmt.Sprintf("[red]socket error: %v[-]", err))
		return
	}

	server = ln
	go readLogs()
	logLine(fmt.Sprintf("listening on %s:%d - click a device to command it", lnAddrHost(ln), logPort))
}

func lnAddrHost(ln *net.UDPConn) string {
	addr := ln.LocalAddr().(*net.UDPAddr)
	if addr.IP == nil {
		return "0.0.0.0"
	}
	return addr.IP.String()
}

func readLogs() {
	buf := make([]byte, 65536)
	for {
		n, rinfo, err := server.ReadFromUDP(buf)
		if err != nil {
			err := err
			app.QueueUpdateDraw(func() {
				logLine(fmt.Sprintf("[red]socket error: %v[-]", err))
			})
			return
		}
		ip := rinfo.IP.String()
		for _, line := range strings.Split(strings.TrimRight(string(buf[:n]), "\r\n"), "\n") {
			line = strings.TrimSpace(line)
			if line == "" {
				continue
			}
			line := line
			app.QueueUpdateDraw(func() {
				if _, ok := seenDevices[ip]; !ok {
					seenDevices[ip] = struct{}{}
					logLine(fmt.Sprintf("[magenta]device discovered: %s[-]", ip))
					refreshDevices()
				}
				logLine(fmt.Sprintf("[green]%s[-] %s", ip, line))
			})
		}
	}
}

// --- main --------------------------------------------------------------------

func main() {
	if len(os.Args) > 1 {
		targetIp = os.Args[1]
	}

	app = tview.NewApplication()
	logBox = tview.NewTextView().
		SetDynamicColors(true).
		SetScrollable(true).
		SetChangedFunc(func() { app.ForceDraw() })

	list = tview.NewList().
		ShowSecondaryText(false).
		SetMainTextColor(tcell.ColorWhite).
		SetSecondaryTextColor(tcell.ColorGray).
		SetSelectedStyle(tcell.StyleDefault.Background(tcell.ColorGreen).Foreground(tcell.ColorWhite).Bold(true))

	input = tview.NewInputField().
		SetFieldBackgroundColor(tcell.ColorBlack).
		SetDoneFunc(func(key tcell.Key) {
			if key != tcell.KeyEnter {
				return
			}
			sendCommand(input.GetText())
			input.SetText("")
			app.SetFocus(input)
		})

	status = tview.NewTextView().
		SetDynamicColors(false).
		SetText(statusText())

	panelsTop := tview.NewFlex().
		AddItem(logBox, 0, 1, false).
		AddItem(list, 26, 1, false)
	root = tview.NewFlex().SetDirection(tview.FlexRow).
		AddItem(panelsTop, 0, 1, false).
		AddItem(input, 3, 1, false).
		AddItem(status, 1, 1, false)

	logBox.SetBorder(true).SetTitle(" kernel log ").SetBorderColor(tcell.ColorLightSkyBlue)
	list.SetBorder(true).SetTitle(" devices ").SetBorderColor(tcell.ColorFuchsia)
	input.SetBorder(true).SetTitle(" command ").SetBorderColor(tcell.ColorYellow)
	status.SetTextColor(tcell.ColorWhite).
		SetBackgroundColor(tcell.ColorBlue)

	if targetIp != "" {
		seenDevices[targetIp] = struct{}{}
		refreshDevices()
	}

	app.
		EnableMouse(true).
		SetRoot(root, true).
		SetInputCapture(func(ev *tcell.EventKey) *tcell.EventKey {
			switch ev.Key() {
			case tcell.KeyF10, tcell.KeyCtrlQ:
				showExitDialog()
				return nil
			case tcell.KeyTab:
				switch app.GetFocus() {
				case input:
					app.SetFocus(list)
				case list:
					app.SetFocus(logBox)
				default:
					app.SetFocus(input)
				}
				return nil
			case tcell.KeyRune:
				if ev.Modifiers()&(tcell.ModCtrl|tcell.ModAlt) != 0 {
					return ev
				}
				if app.GetFocus() != input {
					app.SetFocus(input)
				}
				return ev
			}
			return ev
		}).
		SetFocus(input)

	go bindServer()
	if err := app.Run(); err != nil {
		panic(err)
	}
}

func showExitDialog() {
	modal := tview.NewModal().
		SetText("Exit the debug console?").
		AddButtons([]string{"Yes", "No"}).
		SetDoneFunc(func(_ int, label string) {
			if label == "Yes" {
				app.Stop()
				return
			}
			app.SetRoot(root, true)
			app.SetFocus(input)
		})
	app.SetRoot(modal, true).SetFocus(modal)
}
