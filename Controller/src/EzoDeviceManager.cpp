#include "EzoDeviceManager.h"
#include <Ezo_i2c_util.h>
#include "EzoCommandManager.h"
#include "EzoCallbacks.h"
#include <cstdlib>
#include <cstring>

EzoDeviceManager::EzoDeviceManager(TwoWire &wire)
    : _wire(wire) {}

bool EzoDeviceManager::scan(EzoCommandManager &cmdManager)
{
    clearDevices();

    bool deviceFound = false;

    for (uint8_t addr = 20; addr < 128; ++addr)
    {
        _wire.beginTransmission(addr);
        uint8_t error = _wire.endTransmission();

        if (error == 0)
        {
            deviceFound = true;
            // Create new device
            Ezo_board *device = new Ezo_board(addr, "found device", &_wire);

            Serial.println();
            // Request the device type and firmware version with the "I" command.
            cmdManager.queueCommand(device, "I", 300, [this](Ezo_board *device, const char *response, EzoCommandManager &mgr)
                                    {
                    // Response format: I?,TYPE,VERSION
                    // `EzoCommandManager` now substitutes empty responses with the sentinel "NONE".
                    if (!response || response[0] == '\0' || strcmp(response, "NONE") == 0) {
                        Serial.print("Warning: empty/none response for I command from device 0x");
                        Serial.println(device->get_address(), HEX);
                        return;
                    }

                    String responseString = String(response);
                    int typeStartIndex = responseString.indexOf(',');
                    int typeEndIndex = -1;
                    if (typeStartIndex != -1) {
                        typeEndIndex = responseString.indexOf(',', typeStartIndex + 1);
                    }

                    if (typeStartIndex == -1 || typeEndIndex == -1 || typeEndIndex <= typeStartIndex) {
                        Serial.print("Warning: unexpected I response format from device 0x");
                        Serial.println(device->get_address(), HEX);
                        return;
                    }

                    String deviceType = responseString.substring(typeStartIndex + 1, typeEndIndex);

                    // Then request the name with the "Name?" command, and add the device to the list once we have that information.
                    mgr.queueCommand(device, "Name,?", 300, [this, deviceType](Ezo_board* device, const char* nameResponse, EzoCommandManager& mgr){
                        // Response format: Name?,DEVICE_NAME
                        // If command manager supplied the "NONE" sentinel, treat as missing name
                        if (!nameResponse || nameResponse[0] == '\0' || strcmp(nameResponse, "NONE") == 0) {
                            Serial.print("Warning: empty/none Name? response from device 0x");
                            Serial.println(device->get_address(), HEX);
                            // still add device with fallback name
                            addDevice(device);
                            setDeviceNameOwned(device, deviceType + String("-unknown"));
                            return;
                        }

                        String nameResponseString = String(nameResponse);
                        int nameStart = nameResponseString.indexOf(',');
                        String deviceName;
                        if (nameStart == -1 || nameStart + 1 >= nameResponseString.length()) {
                            // fallback to using entire response (after trimming) or unknown
                            String trimmed = nameResponseString;
                            trimmed.trim();
                            if (trimmed.length() == 0) {
                                deviceName = deviceType + String("-unknown");
                            } else {
                                deviceName = deviceType + String("-") + trimmed;
                            }
                        } else {
                            deviceName = deviceType + String("-") + nameResponseString.substring(nameStart + 1);
                        }

                        addDevice(device);
                        setDeviceNameOwned(device, deviceName);

                        Serial.print("Device at 0x");
                        Serial.print(device->get_address(), HEX);
                        Serial.print("(");
                        Serial.print(device->get_address());
                        Serial.print(") with name: ");
                        Serial.println(device->get_name());
                    }); });
        }
    }

    return deviceFound;
}

/**
 * Updates the name of the device in the list at the given address.
 * This will not be remembered between scans.
 */
void EzoDeviceManager::setName(uint8_t address, const char *name, EzoCommandManager &cmdManager)
{
    for (Ezo_board *device : _device_list)
    {
        if (device->get_address() == address)
        {
            String oldName = device->get_name();
            int typeEndIndex = oldName.indexOf('-');
            String deviceType = oldName.substring(0, typeEndIndex);

            String nameCopy = String(name);
            String command = "Name," + nameCopy;
            cmdManager.queueCommand(device, command.c_str(), 300, [this, deviceType, nameCopy](Ezo_board *device, const char *response, EzoCommandManager &mgr)
                                    {
                String newName = deviceType + "-" + nameCopy;
                setDeviceNameOwned(device, newName);
                Serial.print("Device at 0x");
                Serial.print(device->get_address(), HEX);
                Serial.print("(");
                Serial.print(device->get_address());
                Serial.print(") renamed to: ");
                Serial.println(device->get_name());
            });
            return;
        }
    }

    Serial.print("No device found at address ");
    Serial.println(address);
}

/**
 * Sends the read command to the device in the list on the given address.
 */
void EzoDeviceManager::read(uint8_t address, EzoCommandManager &cmdManager)
{
    for (Ezo_board *device : _device_list)
    {
        if (device->get_address() == address)
        {
            // Queue a Read command in the manager
            this->sendRead(device, cmdManager);
            return;
        }
    }

    Serial.print("No device found at address ");
    Serial.println(address);
}

/**
 * Sends the read command to the device in the list with the given name.
 */
void EzoDeviceManager::read(const char *name, EzoCommandManager &cmdManager)
{
    for (Ezo_board *device : _device_list)
    {
        if (strcmp(device->get_name(), name) == 0)
        {
            this->sendRead(device, cmdManager);
            return;
        }
    }

    Serial.print("No device found with name ");
    Serial.println(name);
}

/**
 * Sends the get name command to the device in the list with the given address.
 */
void EzoDeviceManager::getName(uint8_t address, EzoCommandManager &cmdManager)
{
    for (Ezo_board *device : _device_list)
    {
        if (device->get_address() == address)
        {
            this->sendGetName(device, cmdManager);
            return;
        }
    }

    Serial.print("No device found at address ");
    Serial.println(address);
}

/**
 * Sends the get name command to the device in the list with the given internal name.
 */
void EzoDeviceManager::getName(const char *name, EzoCommandManager &cmdManager)
{
    for (Ezo_board *device : _device_list)
    {
        if (strcmp(device->get_name(), name) == 0)
        {
            this->sendGetName(device, cmdManager);
            return;
        }
    }

    Serial.print("No device found with name ");
    Serial.println(name);
}


/**
 * Sends the info command to the device in the list with the given address.
 */
void EzoDeviceManager::info(uint8_t address, EzoCommandManager &cmdManager)
{
    for (Ezo_board *device : _device_list)
    {
        if (device->get_address() == address)
        {
            this->sendInfo(device, cmdManager);
            return;
        }
    }

    Serial.print("No device found at address ");
    Serial.println(address);
}

void EzoDeviceManager::setI2CAddress(uint8_t old_address, uint8_t new_address, EzoCommandManager &cmdManager)
{
    for (Ezo_board *device : _device_list)
    {
        if (device->get_address() == old_address)
        {
            String command = "I2C," + String(new_address);
            cmdManager.queueCommand(device, command.c_str(), 1000, [this, new_address, old_address](Ezo_board *device, const char *response, EzoCommandManager &mgr){
                // For Teensy 4.x, disable USB to avoid confusing the host computer
                USB1_USBCMD = 0; 
                delay(50); // Small delay to ensure USB disconnects
                
                // Trigger system reset
                SCB_AIRCR = 0x05FA0004;
            });
        }
    }
}

/**
 * Sends the info command to the device in the list with the given internal name.
 */
void EzoDeviceManager::info(const char *name, EzoCommandManager &cmdManager)
{
    for (Ezo_board *device : _device_list)
    {
        if (strcmp(device->get_name(), name) == 0)
        {
            this->sendInfo(device, cmdManager);
            return;
        }
    }

    Serial.print("No device found with name ");
    Serial.println(name);
}

/**
 * Adds a device to the list.
 */
void EzoDeviceManager::addDevice(Ezo_board *device)
{
    _device_list.push_back(device);
    // Track owned name pointer as null initially
    _owned_names.emplace_back(device, nullptr);
}

const std::vector<Ezo_board *> &EzoDeviceManager::getDevices() const
{
    return _device_list;
}

void EzoDeviceManager::clearDevices()
{
    // Free any allocated names we own
    for (auto &p : _owned_names)
    {
        if (p.second)
        {
            free(p.second);
            p.second = nullptr;
        }
    }
    _owned_names.clear();

    for (Ezo_board *dev : _device_list)
    {
        delete dev;
    }
    _device_list.clear();
}

void EzoDeviceManager::sendGetName(Ezo_board *device, EzoCommandManager &cmdManager)
{
    cmdManager.queueCommand(device, "Name,?", 300, onEzoGetName);
}

void EzoDeviceManager::sendRead(Ezo_board *device, EzoCommandManager &cmdManager)
{
    cmdManager.queueCommand(device, "R", 600, onEzoRead);
}

void EzoDeviceManager::setDeviceNameOwned(Ezo_board *device, const String &name)
{
    // Allocate a C-string copy and assign to device via library API that stores pointer
    size_t len = name.length();
    char *buf = (char *)malloc(len + 1);
    if (!buf)
        return; // allocation failed
    memcpy(buf, name.c_str(), len + 1);

    // Free any previous owned name for this device
    for (auto &p : _owned_names)
    {
        if (p.first == device)
        {
            if (p.second)
                free(p.second);
            p.second = buf;
            device->set_name(p.second);
            return;
        }
    }

    // If device not tracked yet (unlikely), track it
    _owned_names.emplace_back(device, buf);
    device->set_name(buf);
}

void EzoDeviceManager::freeDeviceName(Ezo_board *device)
{
    for (auto it = _owned_names.begin(); it != _owned_names.end(); ++it)
    {
        if (it->first == device)
        {
            if (it->second)
                free(it->second);
            _owned_names.erase(it);
            return;
        }
    }
}

void EzoDeviceManager::sendInfo(Ezo_board *device, EzoCommandManager &cmdManager)
{
    cmdManager.queueCommand(device, "I", 300, onEzoGetInfo);
}

void EzoDeviceManager::calibrate(Ezo_board *device, const char *calibArg, uint32_t delayMs, EzoCommandManager &cmdManager)
{
    String command;
    if (strcmp(calibArg, "clear") == 0)
    {
        command = "Cal,clear";
        cmdManager.queueCommand(device, command.c_str(), delayMs, onEzoEmptyResponse);
    }
    else if (strcmp(calibArg, "?") == 0)
    {
        command = "Cal,?";
        cmdManager.queueCommand(device, command.c_str(), delayMs, onEzoCalibrate);
    }
    else
    {
        // Assume it's a calibration value
        command = "Cal," + String(calibArg);
        cmdManager.queueCommand(device, command.c_str(), delayMs, onEzoEmptyResponse);
    }

    
}

void EzoDeviceManager::getDeviceByAddress(uint8_t address, Ezo_board *&out_device)
{
    out_device = nullptr;
    for (Ezo_board *device : _device_list)
    {
        if (device->get_address() == address)
        {
            out_device = device;
            return;
        }
    }
}

void EzoDeviceManager::getDeviceByName(const char *name, Ezo_board *&out_device)
{
    out_device = nullptr;
    for (Ezo_board *device : _device_list)
    {
        if (strcmp(device->get_name(), name) == 0)
        {
            out_device = device;
            return;
        }
    }
}