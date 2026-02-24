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

        // Send command if not yet sent
        if (!cmd.sent) {
            cmd.device->send_cmd(cmd.command);
            cmd.start_time = now;
            cmd.sent = true;
            i++;
            continue;
        }

        // Check if delay elapsed
        if (now - cmd.start_time >= cmd.delay_ms) {
            processCommand(i);
            // processCommand removes element → do NOT increment i
            continue;
        }

        i++;
    }
}

void EzoCommandManager::processCommand(size_t index)
{
    EzoCommand& cmd = _queue[index];

    char response[32] = {0};

    // Read response from EZO
    cmd.device->receive_cmd(response, sizeof(response));

    // Call callback
    if (cmd.callback) {
        cmd.callback(cmd.device, response, *this);
    }

    // Remove from queue
    _queue.erase(_queue.begin() + index);
}