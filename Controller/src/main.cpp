#include "EzoDeviceManager.h"

EzoDeviceManager deviceManager(Wire);

void setup() {
  Serial.begin(115200);
  Wire.begin();

  delay(1000);

  Serial.println("Initial device scan...");
  if (!deviceManager.scan()) {
    Serial.println("No device is found.");
  }
}

void loop() {
  if (!Serial.available()) {
    return;
  }

  String line = Serial.readStringUntil('\n');
  line.trim();
  if (line.length() == 0) {
    return;
  }

  String command = line;
  command.toUpperCase();

  // ---- SCAN ----
  if (command == "SCAN") {
    Serial.println("Scanning for devices...");
    if (!deviceManager.scan()) {
      Serial.println("No device is found.");
    }
    return;
  }

  // ---- LIST ----
  if (command == "LIST") {
    const auto& devices = deviceManager.getDevices();

    if (devices.empty()) {
      Serial.println("No devices registered.");
      return;
    }

    Serial.print("Devices found: ");
    Serial.println(devices.size());

    for (size_t i = 0; i < devices.size(); ++i) {
      Ezo_board* dev = devices[i];

      Serial.print("[");
      Serial.print(i);
      Serial.print("] Address: 0x");
      Serial.print(dev->get_address(), HEX);
      Serial.print("  Name: ");
      Serial.println(dev->get_name());
    }
    return;
  }

  // ---- SETNAME <addr> <name> ----
  if (command.startsWith("SETNAME")) {
    int firstSpace = line.indexOf(' ');
    int secondSpace = line.indexOf(' ', firstSpace + 1);

    if (firstSpace < 0 || secondSpace < 0) {
      Serial.println("Usage: SETNAME <address> <name>");
      return;
    }

    String addrStr = line.substring(firstSpace + 1, secondSpace);
    String nameStr = line.substring(secondSpace + 1);

    uint8_t address = (uint8_t) strtol(addrStr.c_str(), nullptr, 0);
    deviceManager.setName(address, nameStr.c_str());
    return;
  }

  // ---- READ <addr | name> ----
  if (command.startsWith("READ")) {
    int space = line.indexOf(' ');
    if (space < 0) {
      Serial.println("Usage: READ <address | name>");
      return;
    }

    String arg = line.substring(space + 1);
    arg.trim();

    // Detect numeric (decimal or hex)
    bool isNumeric = isDigit(arg[0]) || arg.startsWith("0X");

    if (isNumeric) {
      uint8_t address = (uint8_t) strtol(arg.c_str(), nullptr, 0);
      deviceManager.read(address);
    } else {
      deviceManager.read(arg.c_str());
    }
    return;
  }

  Serial.println("Unknown command");
}
