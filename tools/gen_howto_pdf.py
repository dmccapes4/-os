#!/usr/bin/env python3
"""
Generate HOW_TO_INSTALL_DEBUG_HARDWARE PDF with images.
SEIRAELGAR 🔧 — 2026-11-23
"""
import sys, os
sys.path.insert(0, os.path.join(os.path.dirname(__file__), "pylibs"))

from reportlab.lib.pagesizes import A4
from reportlab.lib.units import mm
from reportlab.lib import colors
from reportlab.lib.styles import getSampleStyleSheet, ParagraphStyle
from reportlab.platypus import (
    SimpleDocTemplate, Paragraph, Spacer, Image, PageBreak,
    Table, TableStyle, HRFlowable, KeepTogether
)
from reportlab.lib.enums import TA_LEFT, TA_CENTER, TA_RIGHT
from reportlab.pdfgen import canvas as pdfcanvas

ASSETS = "/home/lucifer-chains/.cursor/projects/home-lucifer-chains-dev-os/assets"
OUT = "/home/lucifer-chains/dev/-os/docs/HOW_TO_INSTALL_DEBUG_HARDWARE_20261123.pdf"

W, H = A4
MARGIN = 18 * mm

# ── styles ──────────────────────────────────────────────────────────────────
styles = getSampleStyleSheet()

def S(name, **kw):
    base = styles[name]
    return ParagraphStyle(name + "_custom_" + str(id(kw)), parent=base, **kw)

title_s   = S("Title",   fontSize=22, leading=28, textColor=colors.HexColor("#1a1a2e"), spaceAfter=4)
sub_s     = S("Normal",  fontSize=11, leading=15, textColor=colors.HexColor("#555555"), spaceAfter=2, alignment=TA_CENTER)
h1_s      = S("Heading1",fontSize=15, leading=20, textColor=colors.HexColor("#1a1a2e"), spaceBefore=12, spaceAfter=4,
               borderPad=2)
h2_s      = S("Heading2",fontSize=12, leading=16, textColor=colors.HexColor("#2c5f8a"), spaceBefore=8, spaceAfter=3)
body_s    = S("Normal",  fontSize=9,  leading=13, textColor=colors.HexColor("#222222"), spaceAfter=4)
code_s    = S("Code",    fontSize=8,  leading=11, fontName="Courier", textColor=colors.HexColor("#1a1a2e"),
               backColor=colors.HexColor("#f4f4f4"), leftIndent=8, rightIndent=8, spaceBefore=3, spaceAfter=3)
warn_s    = S("Normal",  fontSize=9,  leading=13, textColor=colors.HexColor("#8b0000"),
               backColor=colors.HexColor("#fff0f0"), leftIndent=6, rightIndent=6, spaceBefore=2, spaceAfter=2)
note_s    = S("Normal",  fontSize=9,  leading=13, textColor=colors.HexColor("#004080"),
               backColor=colors.HexColor("#f0f6ff"), leftIndent=6, rightIndent=6, spaceBefore=2, spaceAfter=2)
caption_s = S("Normal",  fontSize=8,  leading=11, textColor=colors.HexColor("#555555"), alignment=TA_CENTER, spaceAfter=8)
toc_s     = S("Normal",  fontSize=10, leading=16, textColor=colors.HexColor("#2c5f8a"))
hr_color  = colors.HexColor("#cccccc")

def fig(path, caption, width_frac=0.92):
    avail = (W - 2 * MARGIN) * width_frac
    img = Image(path, width=avail, height=avail * 0.75)
    img.hAlign = "CENTER"
    return KeepTogether([
        img,
        Paragraph(caption, caption_s),
    ])

def fig_wide(path, caption):
    avail = (W - 2 * MARGIN) * 0.98
    img = Image(path, width=avail, height=avail * 0.5625)  # 16:9
    img.hAlign = "CENTER"
    return KeepTogether([
        img,
        Paragraph(caption, caption_s),
    ])

def hr():
    return HRFlowable(width="100%", thickness=0.5, color=hr_color, spaceAfter=6, spaceBefore=6)

def code(text):
    return Paragraph(text.replace("\n", "<br/>").replace(" ", "&nbsp;"), code_s)

def warn(text):
    return Paragraph("⚠ " + text, warn_s)

def note(text):
    return Paragraph("ℹ " + text, note_s)

# ── page number callback ─────────────────────────────────────────────────────
def add_page_number(canvas, doc):
    canvas.saveState()
    canvas.setFont("Helvetica", 7)
    canvas.setFillColor(colors.HexColor("#999999"))
    canvas.drawString(MARGIN, 10 * mm, "HOW TO INSTALL DEBUG HARDWARE — ὄνυχος / κύριος project")
    canvas.drawRightString(W - MARGIN, 10 * mm, f"Page {doc.page}")
    canvas.restoreState()

# ── content ──────────────────────────────────────────────────────────────────
story = []

# ── COVER ────────────────────────────────────────────────────────────────────
story += [
    Spacer(1, 30 * mm),
    Paragraph("How to Install Debug Hardware", title_s),
    Paragraph("ὄνυχος / κύριος project  ·  valseir (Pi 4) ← probes → lirvol (Pi 5)", sub_s),
    Spacer(1, 6 * mm),
    hr(),
    Spacer(1, 4 * mm),
    Paragraph("Hardware covered: RPi Debug Probe · DSD TECH SH-U09C5 · DAP-Link V2", sub_s),
    Paragraph("Filed: 2026-11-23  ·  Author: SEIRAELGAR 🔧", sub_s),
    Spacer(1, 10 * mm),
]
story.append(fig_wide(f"{ASSETS}/howto_fig1_overview.jpg",
    "Figure 1 — System topology: valseir hosts the probes via USB; probes connect to lirvol via JST-SH and DuPont wires."))
story.append(PageBreak())

# ── TOC ──────────────────────────────────────────────────────────────────────
story += [
    Paragraph("Contents", h1_s),
    hr(),
    Spacer(1, 2 * mm),
]
toc_items = [
    ("1", "What connects where", "3"),
    ("2", "RPi Debug Probe", "3"),
    ("2a", "UART connection (do this first — needed for K1)", "3"),
    ("2b", "SWD connection (requires breakout board)", "4"),
    ("2c", "Plug in to valseir", "5"),
    ("3", "DSD TECH SH-U09C5", "5"),
    ("4", "DAP-Link V2", "6"),
    ("5", "On valseir after plugging everything in", "7"),
    ("6", "Notes from the ὄνυχος spec §3 bench checks", "8"),
]
for num, text, pg in toc_items:
    story.append(Paragraph(f"<b>{num}</b>&nbsp;&nbsp;&nbsp;{text}&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;{pg}", toc_s))
story.append(PageBreak())

# ── §1 TOPOLOGY TABLE ────────────────────────────────────────────────────────
story += [
    Paragraph("1. What connects where", h1_s),
    hr(),
]
table_data = [
    [Paragraph("<b>Probe</b>", body_s), Paragraph("<b>valseir side</b>", body_s),
     Paragraph("<b>lirvol side</b>", body_s), Paragraph("<b>Purpose</b>", body_s)],
    [Paragraph("RPi Debug Probe", body_s),
     Paragraph("micro-USB → any valseir USB-A port", body_s),
     Paragraph("JST-SH 'U' → debug UART connector (between HDMI ports)<br/>JST-SH 'D' → J16 SWD connector", body_s),
     Paragraph("Serial console + SWD debug", body_s)],
    [Paragraph("DAP-Link V2", body_s),
     Paragraph("USB-C → valseir USB-A", body_s),
     Paragraph("SWD JST breakout → lirvol J16 + GPIO Pin 1 (3V3)", body_s),
     Paragraph("Alternative/primary SWD probe", body_s)],
    [Paragraph("DSD TECH SH-U09C5", body_s),
     Paragraph("USB-A → valseir USB-A", body_s),
     Paragraph("TXD/RXD/GND → GPIO 14/15 (3V3 jumper, 5V disconnected)", body_s),
     Paragraph("Spare serial; GPIO UART when RP1 is running", body_s)],
]
t = Table(table_data, colWidths=[(W - 2*MARGIN) * f for f in [0.22, 0.22, 0.34, 0.22]])
t.setStyle(TableStyle([
    ("BACKGROUND",  (0, 0), (-1, 0), colors.HexColor("#2c5f8a")),
    ("TEXTCOLOR",   (0, 0), (-1, 0), colors.white),
    ("ROWBACKGROUNDS", (0, 1), (-1, -1), [colors.HexColor("#f9f9f9"), colors.white]),
    ("GRID",        (0, 0), (-1, -1), 0.3, colors.HexColor("#cccccc")),
    ("VALIGN",      (0, 0), (-1, -1), "TOP"),
    ("LEFTPADDING", (0, 0), (-1, -1), 4),
    ("RIGHTPADDING",(0, 0), (-1, -1), 4),
    ("TOPPADDING",  (0, 0), (-1, -1), 3),
    ("BOTTOMPADDING",(0,0), (-1, -1), 3),
]))
story += [t, Spacer(1, 6*mm)]

# ── §2 DEBUG PROBE ───────────────────────────────────────────────────────────
story += [Paragraph("2. RPi Debug Probe", h1_s), hr()]

story += [
    Paragraph("2a. UART connection (do this first — needed for K1)", h2_s),
    Paragraph(
        "The lirvol debug UART connector is the <b>3-pin JST-SH connector between the two micro-HDMI ports</b>. "
        "It outputs boot messages from firmware start, before RP1 is running. This is why it is used rather than "
        "GPIO pins 14/15, which are behind RP1 and not available early. "
        "The included JST-SH to JST-SH cable plugs straight in — the connector is keyed. "
        "This connector remains accessible at the board edge even with the NVMe/audio HAT fitted.", body_s),
    note("No config change needed on lirvol for UART. The Pi 5 outputs boot UART on this connector by default."),
    Spacer(1, 3*mm),
    fig(f"{ASSETS}/howto_fig2_uart.jpg",
        "Figure 2 — Debug Probe UART: JST-SH cable from debug connector (between HDMI ports) to probe 'U' port. "
        "Pin 3 (TX from Pi) → probe RX; Pin 1 (RX into Pi) ← probe TX; Pin 2 = GND."),
    PageBreak(),
    Paragraph("2b. SWD connection", h2_s),
    warn("J16 (SWD connector) is physically blocked by the NVMe/audio HAT on lirvol. "
         "Do not attempt to wire to J16 without removing the HAT."),
    Spacer(1, 2*mm),
    Paragraph(
        "SWD is reached through the <b>40-pin GPIO header</b>. The HAT exposes this via a passthrough "
        "header on its top surface. Connect the GPIO breakout board to the HAT passthrough. "
        "With <b>enable_jtag_gpio=1</b> in config.txt, the Pi 5 exposes SWD on the GPIO header.", body_s),
    code("GPIO breakout terminal    → RPi Debug Probe 'D' port\n"
         "GPIO 25 (SWCLK) ──────── SC   (clock, into Pi)\n"
         "GPIO 27 (SWDIO) ──────── SD   (bidirectional data)\n"
         "Any GND pin     ──────── GND\n"
         "Pin 1 (3V3)     ──────── VTREF (voltage reference)"),
    warn("GPIO 25/27 mapping is based on Pi 4 (BCM2711) precedent. Verify on Pi 5 before wiring: "
         "after adding enable_jtag_gpio=1 and rebooting, run  raspi-gpio get  on lirvol "
         "and confirm which pins are ALT4 (JTAG/SWD)."),
    warn("Add  enable_jtag_gpio=1  to lirvol's /boot/firmware/config.txt and reboot before connecting."),
    Spacer(1, 3*mm),
    fig(f"{ASSETS}/howto_fig3b_swd_gpio.jpg",
        "Figure 3 — Debug Probe SWD (revised): J16 is blocked by the HAT. "
        "GPIO breakout board on HAT passthrough provides access to GPIO 25 (SWCLK), GPIO 27 (SWDIO), "
        "GND, and Pin 1 (3V3/VTREF). Verify pin mapping on Pi 5 before wiring."),
    PageBreak(),
    Paragraph("2c. Plug in to valseir", h2_s),
    Paragraph("The probe's micro-USB goes into any of valseir's four USB-A ports. "
              "It appears as two CDC-ACM devices:", body_s),
    code("/dev/ttyACM0  — UART port (the 'U' connection to lirvol)\n"
         "/dev/ttyACM1  — SWD/debug port (used by OpenOCD)"),
    note("Numbers may be reversed. Run  dmesg | grep ttyACM  after plug-in to confirm which is which."),
    Spacer(1, 6*mm),
]

# ── §3 DSD TECH ──────────────────────────────────────────────────────────────
story += [
    Paragraph("3. DSD TECH SH-U09C5", h1_s),
    hr(),
    warn("DO NOT connect the 5V wire (red) to lirvol. The Pi 5 GPIO is 3.3V. 5V will damage it."),
    Spacer(1, 2*mm),
    Paragraph(
        "The board has a <b>voltage selector jumper</b> on the PCB. Set it to <b>3V3</b> before "
        "connecting to any Pi. The DuPont wires are labelled on the board:", body_s),
    code("TXD (green) ─── Pin 10 / GPIO 15 (Pi RX)\n"
         "RXD (white) ─── Pin 8  / GPIO 14 (Pi TX)\n"
         "GND (black) ─── Pin 6  or any GND pin\n"
         "5V  (red)   ─── NOT CONNECTED"),
    note("GPIO 14/15 on the Pi 5 are behind the RP1 chip. They are only usable once the kernel "
         "has RP1 running — not for boot messages. Use the RPi Debug Probe 'U' port for early serial."),
    Spacer(1, 3*mm),
    fig(f"{ASSETS}/howto_fig4_dsd_tech.jpg",
        "Figure 4 — DSD TECH SH-U09C5: voltage jumper set to 3V3; 5V wire left disconnected; "
        "TX/RX cross when wiring to Pi GPIO 14/15."),
    Spacer(1, 2*mm),
    Paragraph("Plug the USB-A end into valseir. It appears as <b>/dev/ttyUSB0</b>.", body_s),
    PageBreak(),
]

# ── §4 DAP-LINK V2 ───────────────────────────────────────────────────────────
story += [
    Paragraph("4. DAP-Link V2", h1_s),
    hr(),
    Paragraph(
        "This is the black board with USB-C, two white JST breakouts, and a yellow 20-pin IDC connector. "
        "The two JST breakouts are labelled on the board:", body_s),
    code("COM (left):  3.3V / RX / TX / GND  — serial port\n"
         "SWD (right): GND  / CLK / DIO / 3.3V — SWD debug"),
    Paragraph("For SWD to lirvol J16:", body_s),
    code("DAP-Link SWD JST   lirvol J16\n"
         "3.3V ───────────── GPIO header Pin 1 (VTREF)\n"
         "CLK  ───────────── J16 Pin 1: SWCLK\n"
         "GND  ───────────── J16 Pin 2: GND\n"
         "DIO  ───────────── J16 Pin 3: SWDIO"),
    note("The 20-pin IDC connector follows the standard ARM JTAG pinout. "
         "For SWD via IDC: use pins 1 (VTREF), 4/6/8 (GND), 7 (TMS/SWDIO), 9 (TCK/SWCLK). "
         "Only needed if a 20-pin IDC target connector is available."),
    Spacer(1, 3*mm),
    fig(f"{ASSETS}/howto_fig5_daplink.jpg",
        "Figure 5 — DAP-Link V2: SWD JST breakout wired to lirvol J16 via breakout board; "
        "3V3 also runs to GPIO Pin 1 as VTREF."),
    Spacer(1, 2*mm),
    Paragraph("Connect via USB-A↔USB-C cable to any valseir USB port. It appears as a USB HID "
              "device (addressed directly by OpenOCD) and optionally /dev/ttyACM* for the COM port.", body_s),
    PageBreak(),
]

# ── §5 ON VALSEIR ────────────────────────────────────────────────────────────
story += [
    Paragraph("5. On valseir after plugging everything in", h1_s),
    hr(),
    Paragraph("<b>Check group membership</b>", h2_s),
    code("groups kali"),
    Paragraph("Must include <b>dialout</b> (for /dev/ttyUSB* and /dev/ttyACM*) and "
              "<b>plugdev</b> (for HID-based probes like the Debug Probe and DAP-Link V2).", body_s),
    Paragraph("If either is missing:", body_s),
    code("sudo usermod -aG dialout,plugdev kali\nexec su -l kali"),
    Spacer(1, 4*mm),
    Paragraph("<b>Verify devices appeared</b>", h2_s),
    code("dmesg | tail -20\nls /dev/ttyACM* /dev/ttyUSB* 2>/dev/null"),
    Paragraph("Expected after plugging in all three:", body_s),
    code("/dev/ttyACM0   RPi Debug Probe UART\n"
         "/dev/ttyACM1   RPi Debug Probe SWD/debug\n"
         "/dev/ttyUSB0   DSD TECH SH-U09C5"),
    note("DAP-Link V2 HID device does not appear in /dev/tty*. OpenOCD addresses it directly."),
    Spacer(1, 4*mm),
    Paragraph("<b>Quick serial test (UART to lirvol)</b>", h2_s),
    Paragraph("Once lirvol is powered on with the UART cable connected:", body_s),
    code("minicom -D /dev/ttyACM0 -b 115200"),
    Paragraph("Boot messages from lirvol should appear. Ctrl-A X to exit minicom.", body_s),
    Spacer(1, 4*mm),
    Paragraph("<b>Quick OpenOCD test (SWD to lirvol)</b>", h2_s),
    warn("Requires enable_jtag_gpio=1 in lirvol's config.txt first (see §2b)."),
    code("# Using the RPi Debug Probe:\nopenocd -f interface/cmsis-dap.cfg -f target/rpi5.cfg\n\n"
         "# Using the DAP-Link V2 (same command — both are CMSIS-DAP):\nopenocd -f interface/cmsis-dap.cfg -f target/rpi5.cfg"),
    Paragraph("A successful connection prints the target CPU ID and halts. Ctrl-C to exit.", body_s),
    PageBreak(),
]

# ── §6 BENCH CHECKS ──────────────────────────────────────────────────────────
story += [
    Paragraph("6. Notes from the ὄνυχος spec §3 bench checks", h1_s),
    hr(),
    Paragraph("The spec listed four things to verify before relying on them. Status:", body_s),
    Spacer(1, 2*mm),
]
bench = [
    [Paragraph("<b>Check</b>", body_s), Paragraph("<b>Status</b>", body_s)],
    [Paragraph("UART direction: GPIO 14 is lirvol TX, GPIO 15 is lirvol RX; wires cross.", body_s),
     Paragraph("✓ Confirmed by RPi spec RP-008189-DS-1. Debug connector Pin 3 is TX (out from Pi), "
                "Pin 1 is RX (into Pi). Cross when wiring.", body_s)],
    [Paragraph("UART on lirvol: if the audio HAT covers the GPIO header, use the 3-pin debug "
                "connector between the HDMI ports.", body_s),
     Paragraph("✓ Debug connector is the right path regardless — wired to processor directly, "
                "available before RP1 runs. Use it.", body_s)],
    [Paragraph("Debug access to lirvol: OpenOCD needs SWD/JTAG; which pins the Pi 5 exposes "
                "was not established.", body_s),
     Paragraph("✓ Now established: J16 (3-pin JST-SH between HDMI ports). SWCLK on pin 1, "
                "GND on pin 2, SWDIO on pin 3. Needs enable_jtag_gpio=1 in config.txt.", body_s)],
    [Paragraph("br0: whether the bridge has a second member so lirvol's external traffic passes "
                "through valseir.", body_s),
     Paragraph("— Still undecided. Not a debug hardware question.", body_s)],
]
bt = Table(bench, colWidths=[(W - 2*MARGIN) * f for f in [0.42, 0.58]])
bt.setStyle(TableStyle([
    ("BACKGROUND",   (0, 0), (-1, 0), colors.HexColor("#2c5f8a")),
    ("TEXTCOLOR",    (0, 0), (-1, 0), colors.white),
    ("ROWBACKGROUNDS",(0, 1), (-1, -1), [colors.HexColor("#f9f9f9"), colors.white]),
    ("GRID",         (0, 0), (-1, -1), 0.3, colors.HexColor("#cccccc")),
    ("VALIGN",       (0, 0), (-1, -1), "TOP"),
    ("LEFTPADDING",  (0, 0), (-1, -1), 5),
    ("RIGHTPADDING", (0, 0), (-1, -1), 5),
    ("TOPPADDING",   (0, 0), (-1, -1), 4),
    ("BOTTOMPADDING",(0, 0), (-1, -1), 4),
]))
story += [bt, Spacer(1, 6*mm),
    Paragraph("Sources: Raspberry Pi 3-pin Debug Connector Specification (RP-008189-DS-1), "
              "Arm DS KA005936, SEGGER Knowledge Base (Raspberry Pi 5), arm.kali.org.", caption_s),
]

# ── BUILD ─────────────────────────────────────────────────────────────────────
doc = SimpleDocTemplate(
    OUT,
    pagesize=A4,
    leftMargin=MARGIN,
    rightMargin=MARGIN,
    topMargin=MARGIN,
    bottomMargin=18 * mm,
    title="How to Install Debug Hardware",
    author="SEIRAELGAR",
    subject="ὄνυχος / κύριος project — valseir / lirvol debug wiring",
)
doc.build(story, onFirstPage=add_page_number, onLaterPages=add_page_number)
print(f"PDF written to: {OUT}")
