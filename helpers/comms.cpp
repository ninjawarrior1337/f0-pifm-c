#include "comms.hpp"

#include <vector>
#include <string>

enum class WorkerEvent { Stop = (1 << 0), Rx = (1 << 1) };

inline uint32_t operator|(WorkerEvent a, WorkerEvent b) {
    return static_cast<uint32_t>(static_cast<int>(a) | static_cast<int>(b));
}

inline uint32_t operator&(uint32_t a, WorkerEvent b) {
    return a & static_cast<int>(b);
}

#define WORKER_EVENT_MASK (WorkerEvent::Rx | WorkerEvent::Stop)

void CommunicationHandler::send_command(std::string cmd) {
    FURI_LOG_D(TAG, "Sending commmand %s", cmd.c_str());
    furi_hal_uart_tx(UART, (uint8_t*)cmd.c_str(), cmd.size());
}

int32_t CommunicationHandler::uartWorker(void* ctx) {
    CommunicationHandler* comms = (CommunicationHandler*)ctx;

    FURI_LOG_D(TAG, "PiFM Uart Worker Started");

    for(;;) {
        uint32_t event =
            furi_thread_flags_wait(WORKER_EVENT_MASK, FuriFlagWaitAny, FuriWaitForever);

        furi_check((event & FuriFlagError) == 0);

        if(event & WorkerEvent::Stop) {
            break;
        }

        if(event & WorkerEvent::Rx) {
            size_t length = 0;
            do {
                uint8_t data[64];

                length = furi_stream_buffer_receive(comms->streamBuf, data, sizeof(data), 0);

                if(length > 0) {
                    for(size_t i = 0; i < length; i++) {
                        handleRx(data[i], comms);
                    }
                }
            } while(length > 0);
        }
    }

    FURI_LOG_D(TAG, "PiFM Uart Worker Stopped");
    return 0;
}

void CommunicationHandler::handleRx(uint8_t data, CommunicationHandler* ctx) {
    if(data == '\n') {
        ctx->songs.push_back(ctx->receivedStringBuf);
        ctx->receivedStringBuf.clear();

        // Call all event listeners.
        for(auto f : ctx->refreshEventHandlers) {
            (*f)(ctx->songs);
        }
        return;
    }

    if(data == ',') {
        ctx->songs.push_back(ctx->receivedStringBuf);
        ctx->receivedStringBuf.clear();
        return;
    }

    ctx->receivedStringBuf.push_back((char)data);
}

void CommunicationHandler::uartCallback(UartIrqEvent e, uint8_t data, void* ctx) {
    CommunicationHandler* comms = (CommunicationHandler*)ctx;

    if(e == UartIrqEventRXNE) {
        furi_stream_buffer_send(comms->streamBuf, &data, 1, 0);
        furi_thread_flags_set(furi_thread_get_id(comms->uartThread), (uint32_t)WorkerEvent::Rx);
    }
}

void CommunicationHandler::play() {
    send_command(this->COMMANDS.at(Command::Play));
}

void CommunicationHandler::stop() {
    send_command(this->COMMANDS.at(Command::Stop));
}

void CommunicationHandler::set_frequency(int freq) {
    std::string s;
    char cmd_buf[32];
    snprintf(
        cmd_buf,
        sizeof(cmd_buf),
        COMMANDS.at(Command::SetFreq).c_str(),
        freq / 1000000,
        (freq % 1000000) / 100000);
    s = cmd_buf;
    send_command(s);
}

void CommunicationHandler::refresh_songs() {
    this->songs.clear();
    send_command(this->COMMANDS.at(Command::GetSongs));
}

void CommunicationHandler::setSong(int idx) {
    UNUSED(idx);
    std::string s;
    char cmd_buf[32];
    snprintf(cmd_buf, sizeof(cmd_buf), COMMANDS.at(Command::SetSong).c_str(), idx);
    s = cmd_buf;
    send_command(s);
}

CommunicationHandler* CommunicationHandler::getInstance() {
    if(!instance) {
        instance = new CommunicationHandler();
    }
    return instance;
}

void CommunicationHandler::resetInstance() {
    delete instance;
    instance = NULL;
}

CommunicationHandler::CommunicationHandler() {
    furi_hal_uart_init(CommunicationHandler::UART, CommunicationHandler::BAUDRATE);
    this->streamBuf = furi_stream_buffer_alloc(2048, 1);
    this->uartThread =
        furi_thread_alloc_ex("PiFMUartWorker", 1024, &CommunicationHandler::uartWorker, this);

    furi_thread_start(this->uartThread);

    furi_hal_uart_set_irq_cb(UART, &uartCallback, this);
}

CommunicationHandler::~CommunicationHandler() {
    furi_thread_flags_set(furi_thread_get_id(this->uartThread), (uint32_t)WorkerEvent::Stop);
    furi_thread_join(this->uartThread);
    furi_thread_free(this->uartThread);
    furi_stream_buffer_free(this->streamBuf);
    furi_hal_uart_deinit(CommunicationHandler::UART);
}