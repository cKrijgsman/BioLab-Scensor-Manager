#include "EzoCommandManager.h"

EzoCommandManager::EzoCommandManager() {}

void EzoCommandManager::queueCommand(Ezo_board* device,
                                     const char* command,
                                     uint32_t delay_ms,
                                     EzoCallback callback)
{
    EzoCommand cmd;
    cmd.device = device;
    strncpy(cmd.command, command, sizeof(cmd.command) - 1);
    cmd.delay_ms = delay_ms;
    cmd.start_time = 0;
    cmd.callback = callback;
    cmd.sent = false;

    Serial.print("#Queueing command for device 0x");
    Serial.print(device->get_address(), HEX);
    Serial.print(": \"");
    Serial.print(command);
    Serial.print("\" with delay ");
    Serial.print(delay_ms);
    Serial.println(" ms");

    _queue.push_back(cmd);
}

size_t EzoCommandManager::size() const {
    return _queue.size();
}

void EzoCommandManager::update()
{
    uint32_t now = millis();

    for (size_t i = 0; i < _queue.size(); )
    {
        EzoCommand& cmd = _queue[i];

        // Send command if not yet sent. Only send if device has no other
        // outstanding in-flight command.
        if (!cmd.sent) {
            if (isDeviceInFlight(cmd.device)) {
                // Device busy; skip this entry for now
                i++;
                continue;
            }

            cmd.device->send_cmd(cmd.command);
            cmd.start_time = now;
            cmd.sent = true;
            setDeviceInFlight(cmd.device);
            i++;
            continue;
        }

        // Check if delay elapsed
        if (now - cmd.start_time >= cmd.delay_ms) {
            Ezo_board* processedDevice = processCommand(i);
            if (processedDevice) {
                // Clear in-flight marker for this device before removing so next queued
                // command for the same device can be sent in subsequent update() calls.
                clearDeviceInFlight(processedDevice);
            }

            // Remove from queue
            _queue.erase(_queue.begin() + i);
            // Continue without incrementing i; next element shifted into i
            continue;
        }

        i++;
    }
}

Ezo_board* EzoCommandManager::processCommand(size_t index)
{
    if (index >= _queue.size()) return nullptr;

    // Copy command locally so callbacks can safely mutate the queue.
    EzoCommand cmd = _queue[index];

    char response[32] = {0};

    // Read response from EZO
    auto err = cmd.device->receive_cmd(response, sizeof(response));
    (void)err; // silence unused-variable when not inspecting error here

    // If response is empty, replace it with the sentinel "NONE" and still invoke callback
    if (!response || response[0] == '\0') {
        strncpy(response, "NONE", sizeof(response) - 1);
        response[sizeof(response) - 1] = '\0';
    }

    // Call callback with actual or sentinel response
    if (cmd.callback) {
        cmd.callback(cmd.device, response, *this);
    }

    // Return the processed device so the caller can clear inflight and erase
    return cmd.device;
}

bool EzoCommandManager::isDeviceInFlight(Ezo_board* device) const {
    for (Ezo_board* d : _inflight_devices) {
        if (d == device) return true;
    }
    return false;
}

void EzoCommandManager::setDeviceInFlight(Ezo_board* device) {
    if (!isDeviceInFlight(device)) {
        _inflight_devices.push_back(device);
    }
}

void EzoCommandManager::clearDeviceInFlight(Ezo_board* device) {
    for (size_t i = 0; i < _inflight_devices.size(); ++i) {
        if (_inflight_devices[i] == device) {
            _inflight_devices.erase(_inflight_devices.begin() + i);
            return;
        }
    }
}