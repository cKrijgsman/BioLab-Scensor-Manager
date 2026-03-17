#include "EzoDeviceManager.h"
#include "EzoCommandManager.h"

EzoDeviceManager deviceManager(Wire);
EzoCommandManager cmdManager;

void setup()
{
  Serial.begin(115200);
  Wire.begin();

  delay(1000);

  Serial.println("#Initial device scan...");
  if (!deviceManager.scan(cmdManager))
  {
    Serial.println("#No device is found.");
  }
}

void serialCommandHandler(String line)
{

  String command = line;
  command.toUpperCase();

  // ---- SCAN ----
  if (command == "SCAN")
  {
    Serial.println("Scanning for devices...");
    if (!deviceManager.scan(cmdManager))
    {
      Serial.println("#No device is found.");
    }
    return;
  }

  // ---- CALIBRATE <addr | name> <"clear" | "?" | calibration data> <response delay> ----
  if(command.startsWith("CALIBRATE"))
  {
    int firstSpace = line.indexOf(' ');
    int secondSpace = line.indexOf(' ', firstSpace + 1);
    int thirdSpace = line.indexOf(' ', secondSpace + 1);

    if (firstSpace < 0 || secondSpace < 0)
    {
      Serial.println("#Usage: CALIBRATE <address | name> <\"clear\" | \"?\" | calibration data> <response delay>");
      return;
    }

    String arg = line.substring(firstSpace + 1, secondSpace);
    String calibArg = line.substring(secondSpace + 1, thirdSpace);
    calibArg.trim();

    // If the calibration delay is not provided, default to 300ms
    uint32_t delayMs = 300; // default
    if (thirdSpace >= 0) {
      String delayStr = line.substring(thirdSpace + 1);
      delayStr.trim();
      if (delayStr.length() > 0) {
        delayMs = (uint32_t)strtoul(delayStr.c_str(), nullptr, 0);
      }
    }

    // Detect numeric (decimal or hex)
    bool isNumeric = isDigit(arg[0]) || arg.startsWith("0X");

    Ezo_board* device = nullptr;

    if (isNumeric)
    {
      uint8_t address = (uint8_t)strtol(arg.c_str(), nullptr, 0);
      deviceManager.getDeviceByAddress(address, device);
    }
    else
    {
      deviceManager.getDeviceByName(arg.c_str(), device);
    }

    if (device)
    {
      deviceManager.calibrate(device, calibArg.c_str(), delayMs, cmdManager);
    }
    else
    {
      Serial.println("Device not found.");
    }
    return;
  }

  // ---- LIST ----
  if (command == "LIST")
  {
    const auto &devices = deviceManager.getDevices();

    if (devices.empty())
    {
      Serial.println("#No devices registered.");
      return;
    }

    Serial.print("#Devices found: ");
    Serial.println(devices.size());

    for (size_t i = 0; i < devices.size(); ++i)
    {
      Ezo_board *dev = devices[i];

      Serial.print("DEVICE,");
      Serial.print(dev->get_address(), HEX);
      Serial.print(",");
      Serial.println(dev->get_name());
    }
    return;
  }

  // ---- NAME <addr | name> ----
  if (command.startsWith("NAME"))
  {
    int space = line.indexOf(' ');
    if (space < 0)
    {
      Serial.println("#Usage: NAME <address | name>");
      return;
    }

    String arg = line.substring(space + 1);
    arg.trim();

    // Detect numeric (decimal or hex)
    bool isNumeric = isDigit(arg[0]) || arg.startsWith("0X");

    if (isNumeric)
    {
      uint8_t address = (uint8_t)strtol(arg.c_str(), nullptr, 0);
      deviceManager.getName(address, cmdManager);
    }
    else
    {
      deviceManager.getName(arg.c_str(),cmdManager);
    }

    return;
  }

  // ---- SETNAME <addr> <name> ----
  if (command.startsWith("SETNAME"))
  {
    int firstSpace = line.indexOf(' ');
    int secondSpace = line.indexOf(' ', firstSpace + 1);

    if (firstSpace < 0 || secondSpace < 0)
    {
      Serial.println("#Usage: SETNAME <address> <name>");
      return;
    }

    String addrStr = line.substring(firstSpace + 1, secondSpace);
    String nameStr = line.substring(secondSpace + 1);

    uint8_t address = (uint8_t)strtol(addrStr.c_str(), nullptr, 0);
    deviceManager.setName(address, nameStr.c_str(), cmdManager);
    return;
  }

  // ---- INFO <addr | name> ----
  if (command.startsWith("INFO"))
  {
    int space = line.indexOf(' ');
    if (space < 0)
    {
      Serial.println("#Usage: INFO <address | name>");
      return;
    }

    String arg = line.substring(space + 1);
    arg.trim();

    // Detect numeric (decimal or hex)
    bool isNumeric = isDigit(arg[0]) || arg.startsWith("0X");

    if (isNumeric)
    {
      uint8_t address = (uint8_t)strtol(arg.c_str(), nullptr, 0);
      deviceManager.info(address, cmdManager);
    }
    else
    {
      deviceManager.info(arg.c_str(), cmdManager);
    }

    return;
  }

  // ---- READ <addr | name> ----
  if (command.startsWith("READ"))
  {
    int space = line.indexOf(' ');
    if (space < 0)
    {
      Serial.println("#Usage: READ <address | name>");
      return;
    }

    String arg = line.substring(space + 1);
    arg.trim();

    // Detect numeric (decimal or hex)
    bool isNumeric = isDigit(arg[0]) || arg.startsWith("0X");

    if (isNumeric)
    {
      uint8_t address = (uint8_t)strtol(arg.c_str(), nullptr, 0);
      deviceManager.read(address, cmdManager);
    }
    else
    {
      deviceManager.read(arg.c_str(), cmdManager);
    }
    return;
  }

  // ---- SETI2C <addr> <new_addr> ----
  if (command.startsWith("SETI2C"))
  {
      int firstSpace = line.indexOf(' ');
      int secondSpace = line.indexOf(' ', firstSpace + 1);

      if (firstSpace < 0 || secondSpace < 0)
      {
        Serial.println("#Usage: SETI2C <address> <new_address>");
        return;
      }

      String addrStr = line.substring(firstSpace + 1, secondSpace);
      String newAddrStr = line.substring(secondSpace + 1);

      uint8_t address = (uint8_t)strtol(addrStr.c_str(), nullptr, 0);
      uint8_t newAddress = (uint8_t)strtol(newAddrStr.c_str(), nullptr, 0);

      for (Ezo_board *device : deviceManager.getDevices())
      {
        if (device->get_address() == address)
        {
          deviceManager.setI2CAddress(address, newAddress, cmdManager);
          return;
        }
      }
  }

  // ---- QUEUESIZE ----
  if (command == "QUEUESIZE")
  {
    Serial.print("QUEUESIZE,");
    Serial.println(cmdManager.size());
    return;
  }

  Serial.println("#Unknown command");
}

void loop()
{
  
  // Check on EZO command updates
  cmdManager.update();

  if (!Serial.available())
  {
    return;
  }


  // Handle User input over Serial
  String line = Serial.readStringUntil('\n');
  line.trim();
  if (line.length() == 0)
  {
    return;
  }

  serialCommandHandler(line);
}
