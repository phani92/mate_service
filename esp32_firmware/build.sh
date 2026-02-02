#!/bin/bash
# Build and Upload Script for Mate Tracker ESP32-C3

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}========================================${NC}"
echo -e "${GREEN}  Mate Tracker ESP32-C3 Build Script   ${NC}"
echo -e "${GREEN}========================================${NC}"
echo ""

# Check if PlatformIO is installed
if ! command -v pio &> /dev/null; then
    echo -e "${RED}Error: PlatformIO CLI is not installed${NC}"
    echo "Install it with: pip install platformio"
    echo "Or: brew install platformio"
    exit 1
fi

# Navigate to firmware directory
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$SCRIPT_DIR"

case "$1" in
    build)
        echo -e "${YELLOW}Building firmware...${NC}"
        pio run
        ;;
    upload)
        echo -e "${YELLOW}Uploading firmware...${NC}"
        pio run --target upload
        ;;
    uploadfs)
        echo -e "${YELLOW}Uploading LittleFS filesystem...${NC}"
        pio run --target uploadfs
        ;;
    all)
        echo -e "${YELLOW}Building and uploading everything...${NC}"
        pio run --target uploadfs
        pio run --target upload
        ;;
    monitor)
        echo -e "${YELLOW}Opening serial monitor...${NC}"
        pio device monitor
        ;;
    clean)
        echo -e "${YELLOW}Cleaning build files...${NC}"
        pio run --target clean
        ;;
    *)
        echo "Usage: $0 {build|upload|uploadfs|all|monitor|clean}"
        echo ""
        echo "Commands:"
        echo "  build     - Compile the firmware"
        echo "  upload    - Upload firmware to ESP32-C3"
        echo "  uploadfs  - Upload LittleFS filesystem (web files)"
        echo "  all       - Build and upload both firmware and filesystem"
        echo "  monitor   - Open serial monitor"
        echo "  clean     - Clean build files"
        echo ""
        echo "Example workflow:"
        echo "  1. Edit src/config.h with your WiFi credentials"
        echo "  2. Run: $0 all"
        echo "  3. Run: $0 monitor"
        exit 1
        ;;
esac

echo ""
echo -e "${GREEN}Done!${NC}"
