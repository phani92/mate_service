# Mate Tracker ESP32-C3 Firmware

> **📖 See the [main project README](../README.md) for an overview of all deployment options.**

This folder contains the ESP32-C3 firmware that hosts the Mate Service website as a standalone device. The ESP32 runs a web server and connects to your WiFi network — no external server required!

## Features

- **WiFi Connectivity** - Connects to your local 2.4GHz WiFi network
- **Async Web Server** - High-performance HTTP server using ESPAsyncWebServer
- **Static File Hosting** - Serves HTML/CSS/JS from LittleFS filesystem
- **RESTful API** - Complete API for consumption tracking
- **Persistent Storage** - Data survives reboots using NVS (Non-Volatile Storage)
- **mDNS Support** - Access via `http://mate-tracker.local`
- **Low Power** - ~0.5W consumption, perfect for always-on deployment

## Hardware Requirements

- ESP32-C3 development board (ESP32-C3-DevKitM-1, Seeed XIAO ESP32C3, etc.)
- USB cable for programming (data cable, not charge-only)
- WiFi network (2.4GHz only, ESP32-C3 doesn't support 5GHz)

## Quick Start

### 1. Install PlatformIO

**VS Code (Recommended):**
1. Install [VS Code](https://code.visualstudio.com/)
2. Go to Extensions → Search "PlatformIO IDE" → Install

**CLI:**
```bash
pip install platformio
```

### 2. Configure WiFi

Edit `src/config.h`:

```cpp
#define WIFI_SSID     "YourWiFiName"
#define WIFI_PASSWORD "YourWiFiPassword"
```

### 3. Build and Upload

```bash
# macOS/Linux
./build.sh all

# Windows
build.bat all

# Or using PlatformIO directly
pio run --target uploadfs   # Upload web files first
pio run --target upload     # Upload firmware
pio device monitor          # View serial output
```

### 4. Access the Website

Check the serial monitor for the IP address, then open in a browser:
```
http://192.168.x.x        (IP from serial monitor)
http://mate-tracker.local (mDNS - may not work on all networks)
```

## Project Structure

```
esp32_firmware/
├── platformio.ini          # PlatformIO configuration
├── partitions.csv          # Custom partition table
├── build.sh               # Build script (macOS/Linux)
├── build.bat              # Build script (Windows)
├── src/
│   ├── main.cpp           # Main application entry
│   ├── config.h           # Configuration (WiFi, etc.)
│   ├── wifi_manager.h     # WiFi connection handling
│   ├── web_handlers.h     # HTTP route handlers
│   └── data_storage.h     # NVS data persistence
└── data/                  # LittleFS web files
    ├── index.html         # Main webpage
    ├── style.css          # Styles
    ├── config.js          # App configuration
    └── app.js             # Frontend JavaScript
```

## API Endpoints

| Method | Endpoint | Description |
|--------|----------|-------------|
| GET | `/api/status` | System status (WiFi, memory, uptime) |
| GET | `/api/state` | Get full application state |
| POST | `/api/users` | Add new user |
| DELETE | `/api/users/{id}` | Remove user |
| POST | `/api/items` | Add new item/flavor |
| DELETE | `/api/items/{id}` | Remove item |
| PUT | `/api/items/{id}/stock` | Update item stock |
| POST | `/api/consumption` | Record consumption |
| DELETE | `/api/consumption/{id}` | Remove consumption record |
| POST | `/api/payments` | Process payment |
| POST | `/api/reset` | Reset all data |

### Example API Calls

**Add a user:**
```bash
curl -X POST http://mate-tracker.local/api/users \
  -H "Content-Type: application/json" \
  -d '{"name": "John"}'
```

**Add an item:**
```bash
curl -X POST http://mate-tracker.local/api/items \
  -H "Content-Type: application/json" \
  -d '{"name": "Mate Classic", "price": 1.00, "stock": 24}'
```

**Record consumption:**
```bash
curl -X POST http://mate-tracker.local/api/consumption \
  -H "Content-Type: application/json" \
  -d '{"userId": "123", "itemId": "456", "quantity": 2}'
```

## Configuration Options

Edit `src/config.h` to customize:

| Option | Default | Description |
|--------|---------|-------------|
| `WIFI_SSID` | (required) | Your WiFi network name |
| `WIFI_PASSWORD` | (required) | Your WiFi password |
| `WIFI_TIMEOUT_MS` | 30000 | WiFi connection timeout |
| `MDNS_HOSTNAME` | "mate-tracker" | mDNS hostname |
| `MAX_USERS` | 20 | Maximum number of users |
| `MAX_ITEMS` | 50 | Maximum number of items |
| `MAX_CONSUMPTION_RECORDS` | 500 | Maximum consumption records |
| `LED_PIN` | 8 | Status LED GPIO pin |

## Customizing the Web Interface

The web files in `data/` folder are optimized/minified versions. To customize:

1. Edit the files in `data/` folder
2. Re-upload filesystem: `pio run --target uploadfs`
3. Or modify the original files in the parent project and copy them

### Configuration in `data/config.js`

```javascript
const CONFIG = {
    appName: "Mate Service",
    appEmoji: "🧉",
    terminology: {
        item: "Flavor",
        items: "Flavors",
        unit: "bottle",
        units: "bottles"
    },
    defaults: {
        initialStock: 24,
        currency: "€"
    }
};
```

## Troubleshooting

### WiFi Connection Issues

1. Verify SSID and password are correct
2. Ensure the network is 2.4GHz (ESP32-C3 doesn't support 5GHz)
3. Check serial monitor for error messages
4. Try moving closer to the router

### mDNS Not Working

- mDNS may not work on all networks/devices
- Use the IP address instead (shown in serial monitor)
- On Windows, you may need Bonjour installed

### Upload Failed

1. Check USB cable (data cable, not charge-only)
2. Hold BOOT button while pressing RST to enter bootloader mode
3. Check correct COM port is selected

### Website Not Loading

1. Verify filesystem was uploaded (`pio run --target uploadfs`)
2. Check serial monitor for LittleFS errors
3. Ensure all files exist in `data/` folder

### Data Not Persisting

1. NVS has limited space (~15KB recommended)
2. Reset data if corrupted: POST to `/api/reset`
3. Check serial monitor for storage errors

## Memory Usage

- Flash: ~1.2MB for code + ~300KB for filesystem
- RAM: ~50KB typical usage
- NVS: ~15KB for data storage

## LED Indicators

| Pattern | Meaning |
|---------|---------|
| 2 blinks | Starting up |
| 3 quick blinks | Ready and connected |
| Single blink every 5s | Heartbeat (connected) |
| 5 slow blinks | WiFi connection error |
| 10 rapid blinks | Filesystem error |

## License

MIT License - Feel free to modify and use as needed.

## Contributing

Contributions welcome! Please:
1. Fork the repository
2. Create a feature branch
3. Submit a pull request
