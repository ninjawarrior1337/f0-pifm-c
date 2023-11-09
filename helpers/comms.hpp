#pragma once

#include "includes.hpp"

#include <map>
#include <set>
#include <string>
#include <functional>
#include <memory>

enum class Command { Play, Stop, SetFreq, GetSongs, SetSong };
using RefreshEventCallback = std::function<void(std::vector<std::string>&)>;
using RefreshEventCallbackPtr = std::shared_ptr<RefreshEventCallback>;
using RefreshEventCallbackKey = std::weak_ptr<RefreshEventCallback>;

class CommunicationHandler {
private:
    inline static CommunicationHandler* instance;

    FuriThread* uartThread;
    FuriStreamBuffer* streamBuf;

    std::string receivedStringBuf;
    std::vector<std::string> songs;

    std::set<RefreshEventCallbackPtr> refreshEventHandlers;

    const std::map<Command, std::string> COMMANDS = {
        {Command::Play, "play\n"},
        {Command::Stop, "stop\n"},
        {Command::SetFreq, "set freq %d.%.1d\n"},
        {Command::GetSongs, "get songs\n"},
        {Command::SetSong, "set song %d\n"}};

    void send_command(std::string cmd);

    static int32_t uartWorker(void* ctx);
    static void uartCallback(UartIrqEvent e, uint8_t data, void* ctx);

    static void handleRx(uint8_t data, CommunicationHandler* ctx);

    CommunicationHandler();
    ~CommunicationHandler();

public:
    static const uint32_t BAUDRATE = 115200;
    static const FuriHalUartId UART = FuriHalUartIdLPUART1;

    void play();

    void stop();

    void set_frequency(int frequency);

    void refresh_songs();

    void setSong(int idx);

    static CommunicationHandler* getInstance();
    static void resetInstance();

    RefreshEventCallbackKey add_refresh_callback(RefreshEventCallback cb) {
        auto ptr = std::make_shared<RefreshEventCallback>(cb);
        refreshEventHandlers.insert(ptr);
        return ptr;
    }

    void remove_refresh_callback(RefreshEventCallbackKey key) {
        refreshEventHandlers.erase((RefreshEventCallbackPtr)key);
    }

    CommunicationHandler(const CommunicationHandler& obj) = delete;
    CommunicationHandler& operator=(const CommunicationHandler& obj) = delete;
};