#pragma once

#include <vector>
#include <algorithm>
#include <functional>
#include <Arduino.h>
#include <Ezo_i2c.h>

class EzoCommandManager {
public:
    using EzoCallback = std::function<void(Ezo_board*, const char*, EzoCommandManager&)>;

    EzoCommandManager();

    // Queue a command to an EZO device
    void queueCommand(Ezo_board* device,
                      const char* command,
                      uint32_t delay_ms,
                      EzoCallback callback);

    // Call frequently in loop()
    void update();

    size_t size() const;

private:
    struct EzoCommand {
        Ezo_board* device;
        char command[32];
        uint32_t delay_ms;
        uint32_t start_time;
        EzoCallback callback;
        bool sent;
    };

    std::vector<EzoCommand> _queue;

    // Devices which currently have an outstanding sent command
    std::vector<Ezo_board*> _inflight_devices;

    // Process the command at `index`. Returns the device pointer that was
    // processed (or nullptr on failure). The caller is responsible for
    // removing the item from the queue and clearing in-flight markers.
    Ezo_board* processCommand(size_t index);
    bool isDeviceInFlight(Ezo_board* device) const;
    void setDeviceInFlight(Ezo_board* device);
    void clearDeviceInFlight(Ezo_board* device);
};