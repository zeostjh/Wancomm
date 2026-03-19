# Quick Start Guide - VoiceChat Professional Intercom

## 🚀 5-Minute Setup

### Step 1: Build the Application
```bash
cd /Users/greglandon/Documents/code/VoiceChat
./build.sh
```

### Step 2: Start a Server (Computer #1)
```bash
./build/voicechat
```
1. Enter username: "Server"
2. Select: **"Server Mode"**
3. Port: 5000 (default)
4. Click OK
5. Click "START SERVER"
6. ✅ Server is now running

### Step 3: Connect Clients (Computer #2, #3, etc.)
```bash
./build/voicechat
```
1. Enter username: "Alice" (or any name)
2. Select: **"Client Mode"**
3. Server IP: [Server's IP address]
4. Port: 5000
5. Click OK
6. Click "CONNECT TO SERVER"
7. ✅ You're connected!

### Step 4: Start Talking!

**On any client:**
1. Select your channel (1-4)
2. **Press and HOLD the red TALK button**
3. Speak into your microphone
4. Release when done
5. Other users on the same channel will hear you!

## 🎯 Finding Your Server IP Address

### macOS/Linux:
```bash
ifconfig | grep "inet "
```
Look for something like: `192.168.1.100`

### Or use the server's address display in the app

## 💡 Common Scenarios

### Test Locally (Same Computer)
- Server IP: `127.0.0.1` or `localhost`
- Perfect for testing!

### Office Network (LAN)
- Server IP: `192.168.1.x` (local network)
- Make sure computers are on same network
- Fast and reliable!

### Over Internet
- Server IP: [Public IP of server]
- May need to forward port 5000 on router
- Higher latency

## 🎙️ Pro Tips

1. **Use Headphones** - Prevents echo/feedback
2. **Position Mic Close** - Better audio quality
3. **Monitor TX Meter** - See when you're transmitting
4. **Watch User List** - Know who's online
5. **Test Before Live** - Always do a sound check

## 🔧 Troubleshooting Quick Fixes

### "Can't hear anyone"
- ✓ Click LISTEN button to turn it ON (green)
- ✓ Check you're on the same channel
- ✓ Verify speakers/headphones work

### "No one hears me"
- ✓ Press and HOLD the TALK button
- ✓ Check microphone permissions
- ✓ Verify you're on the same channel

### "Can't connect"
- ✓ Verify server is running first
- ✓ Check server IP address is correct
- ✓ Make sure port 5000 isn't blocked

### "User list is empty (on server)"
- ✓ This is normal - server doesn't show itself
- ✓ Server only shows connected clients

## 📋 Production Checklist

Before your event:
- [ ] Server machine ready and reliable
- [ ] All users know the server IP
- [ ] Everyone has tested their audio
- [ ] Channels assigned to teams
- [ ] Usernames decided (clear names)
- [ ] Headphones for all users
- [ ] Backup plan if network fails

## 🎬 Example Production Setup

**Theater Production**

**Server:** Production Control Booth
- Username: "Server"
- Mode: Server
- Port: 5000

**Clients:**
1. **Director** (booth) → Channel 4 - DIRECTOR
2. **Stage Manager** → Channel 1 - PRODUCTION  
3. **Light Tech** → Channel 3 - TECH
4. **Sound Tech** → Channel 3 - TECH
5. **Front of House** → Channel 1 - PRODUCTION

Result: Director has private channel, Tech team shares a channel, Production team shares another!

## 📞 Quick Reference Card

```
╔══════════════════════════════════════╗
║  VOICECHAT QUICK REFERENCE          ║
╠══════════════════════════════════════╣
║  🔴 TALK → Press & Hold to speak    ║
║  🟢 LISTEN → Toggle ON/OFF          ║
║  📡 CHANNEL → Select 1-4            ║
║  👥 USERS → See who's online        ║
║  🔌 CONNECT → Join/Leave network    ║
╠══════════════════════════════════════╣
║  Indicators:                         ║
║  ● Green = Active/On                ║
║  ● Gray = Inactive/Off              ║
║  🔴 = User talking                  ║
║  🟢 = User listening                ║
╚══════════════════════════════════════╝
```

Print this and keep it near each station!

## ⚡ Emergency Recovery

### Server Crashes
1. Restart server application
2. Click "START SERVER"
3. All clients will need to reconnect

### Client Disconnects
1. Click "CONNECT TO SERVER" again
2. You'll rejoin automatically

### Audio Glitches
1. Toggle LISTEN off then on
2. If persists, disconnect and reconnect

## 🎓 Training New Users

**5-Second Tutorial:**
"Press red button to talk, release when done. Green button should be ON to hear others."

**That's it!** The interface is designed to be that simple.

## 🌟 Best Practices

### DO:
✅ Keep Listen ON during productions
✅ Hold Talk button while speaking  
✅ Use clear, short transmissions
✅ Say your name when talking in groups
✅ Monitor the user list

### DON'T:
❌ Leave Talk button pressed (background noise)
❌ Switch channels mid-production
❌ Use wireless on critical positions
❌ Skip audio testing
❌ Put computer to sleep while connected

## 📊 Expected Performance

- **Latency:** 20-50ms (excellent)
- **Quality:** Clear voice, minimal compression
- **Users:** Up to 10 simultaneous
- **Bandwidth:** ~24 kbps per talking user
- **Range:** Unlimited (if networked)

## 🆘 Support

If something doesn't work:
1. Check this guide first
2. Review README.md for detailed info
3. Check INTERFACE_GUIDE.md for UI details
4. Review BUILD.md for build issues

---

**Ready to communicate like a pro?** 🎭🎬🎤

Start your server, connect your team, and break a leg! 🌟
