# VoiceChat Interface Guide

## Login Dialog

When you first launch VoiceChat, you'll see a login dialog:

```
┌─────────────────────────────────────┐
│   VoiceChat Intercom                │
│                                     │
│   Username:  [_____________]        │
│   Mode:      [▼ Client Mode ]       │
│   Server IP: [127.0.0.1____]        │
│   Port:      [5000_________]        │
│                                     │
│              [ OK ] [Cancel]        │
└─────────────────────────────────────┘
```

## Main Interface (Client Mode)

```
╔═══════════════════════════════════════╗
║   CLEARCOM INTERCOM                   ║
║   JohnDoe (Client)                    ║
╠═══════════════════════════════════════╣
║                                       ║
║  CHANNEL                              ║
║  ┌─────────────────────────────────┐ ║
║  │ 🔴 CHANNEL 1 - PRODUCTION     ▼│ ║
║  └─────────────────────────────────┘ ║
║                                       ║
║  CONTROLS                             ║
║  ┌─────────────────────────────────┐ ║
║  │ ●  ┌──────────────────────────┐│ ║
║  │    │       TALK               ││ ║  <- Red button
║  │    │  (Press & Hold)          ││ ║     (large)
║  │    └──────────────────────────┘│ ║
║  └─────────────────────────────────┘ ║
║  ┌─────────────────────────────────┐ ║
║  │ ● ┌───────────────────────────┐│ ║
║  │   │    LISTEN: ON             ││ ║  <- Green button
║  │   └───────────────────────────┘│ ║     (toggle)
║  └─────────────────────────────────┘ ║
║                                       ║
║  AUDIO LEVELS                         ║
║  TX ▓▓▓▓▓▓▓▓▓▓▓▓░░░░░░░           ║
║  RX ▓▓▓▓▓░░░░░░░░░░░░░░           ║
║                                       ║
║  CONNECTED USERS                      ║
║  3 users online                       ║
║  ┌─────────────────────────────────┐ ║
║  │ 🔴 Alice (talking)              │ ║
║  │ 🟢 Bob                          │ ║
║  │ 🟢 Charlie                      │ ║
║  └─────────────────────────────────┘ ║
║                                       ║
║  ┌─────────────────────────────────┐ ║
║  │   DISCONNECT                    │ ║
║  └─────────────────────────────────┘ ║
╠═══════════════════════════════════════╣
║ ● ONLINE                              ║
║ TX: 1234 | RX: 5678 | Loss: 0        ║
╚═══════════════════════════════════════╝
```

## Color Scheme

### Dark Professional Theme
- **Background:** Almost black (#0a0a0a, #121212)
- **Accent Color:** Orange (#ff6b35) - Headers, highlights
- **Text:** Light gray (#e0e0e0)

### Button Colors
- **Talk Button:**
  - Default: Dark red (#8b0000)
  - Pressed: Bright red (#ff0000)
  - Border: Darker red (#4a0000)
  
- **Listen Button:**
  - Off: Dark green (#004d00)
  - On: Bright green (#00cc00)
  - Border: Darker green (#003300)

- **Connect Button:** Orange (#ff6b35)

### LED Indicators
- **Off:** Gray (#2a2a2a)
- **Active:** Bright green (#00ff00)
- **Border:** #1a1a1a / #00cc00

### Audio Meters
- **TX Meter:** Green → Yellow → Red gradient
- **RX Meter:** Blue → Cyan gradient
- **Background:** Dark gray (#1a1a1a)

## Interface States

### Not Connected
```
TALK button: Disabled (dark gray)
LISTEN button: Disabled (dark gray)
LED indicators: Off (gray)
Status: "● OFFLINE"
Users list: Empty
Connect button: "CONNECT TO NETWORK"
```

### Connected & Idle
```
TALK button: Enabled (dark red)
LISTEN button: Enabled & ON (bright green)
LED indicators: Listen ON (green), Talk OFF (gray)
Status: "● ONLINE"
Users list: Shows connected users
Connect button: "DISCONNECT"
```

### Talking
```
TALK button: Pressed (bright red)
Talk LED: Bright green
TX meter: Active (60-100%)
Audio is being transmitted
```

### Listening
```
LISTEN button: Checked (bright green)
Listen LED: Bright green
RX meter: Active when receiving audio
Audio is being played
```

### Not Listening
```
LISTEN button: Unchecked (dark green)
Listen LED: Off (gray)
RX meter: Inactive (0%)
No audio is played
```

## User List Icons

- 🔴 **Red dot** - User is currently talking
- 🟢 **Green dot** - User is listening (idle)
- ⚪ **White dot** - User is not listening

## Tips for Professional Use

1. **Keep Listen ON** - Stay connected to your team
2. **Press Talk firmly** - Hold button while speaking
3. **Monitor levels** - Check TX meter when talking
4. **Watch user list** - See who's on your channel
5. **Choose right channel** - Each production group has its own
6. **Server stability** - Run server on reliable machine
7. **Network quality** - Use wired connections when possible
8. **Audio check** - Test before live production

## Typical Production Setup

### Server
- Run on production control machine
- Reliable network connection
- No audio devices needed (just routing)

### Clients
- Director: Channel 4
- Production crew: Channel 1
- Stage crew: Channel 2
- Technical crew: Channel 3

### Example Workflow
1. Server starts at beginning of day
2. All crew members connect with their names
3. Each department selects their channel
4. Press Talk to communicate within channel
5. Toggle Listen off for privacy when needed
6. Monitor who's talking via LED indicators

## Interface Dimensions

- **Window Size:** 480 x 720 pixels (fixed)
- **Talk Button:** 80px height
- **Listen Button:** 70px height
- **Channel Selector:** 50px height
- **Connect Button:** 50px height
- **Audio Meters:** 25px height
- **LED Indicators:** 20x20px

## Font Usage

- **Title:** 16pt, Bold, Letter-spaced
- **Section Labels:** 11pt, Bold, Orange
- **Buttons:** 14-24pt, Bold
- **Status Text:** 10-12pt
- **User List:** 12pt

This creates a professional, clear, easy-to-use interface suitable for production environments!
