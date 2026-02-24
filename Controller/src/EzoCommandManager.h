#pragma once

#include <vector>
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

    void processCommand(size_t index);
};