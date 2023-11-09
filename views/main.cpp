#include "includes.hpp"
#include "helpers/comms.hpp"

#include <vector>
#include <memory>

enum class MainMenuOptions : uint32_t {
    Start,
    Stop,
    SetFreq,
    SetSong,
};

class MainView {
private:
    ViewDispatcher* vd;
    Submenu* menu;

    static void menu_item_callback(void* ctx, uint32_t idx) {
        MainView* mvd = (MainView*)ctx;
        MainMenuOptions o = (MainMenuOptions)idx;
        CommunicationHandler* comms = CommunicationHandler::getInstance();
        switch(o) {
        case MainMenuOptions::Start:
            comms->play();
            break;
        case MainMenuOptions::Stop:
            comms->stop();
            break;
        case MainMenuOptions::SetFreq:
            view_dispatcher_switch_to_view(
                mvd->vd, static_cast<uint32_t>(Views::SetFrequencyView));
            break;
        case MainMenuOptions::SetSong:
            view_dispatcher_switch_to_view(mvd->vd, static_cast<uint32_t>(Views::SetSongView));
            break;
        };
    }

    static uint32_t previous_callback(void* ctx) {
        UNUSED(ctx);
        return VIEW_NONE;
    }

public:
    MainView(ViewDispatcher* vd) {
        this->menu = submenu_alloc();
        this->vd = vd;

        submenu_set_header(menu, "PiFM");
        submenu_add_item(
            this->menu, "Play", (uint32_t)MainMenuOptions::Start, &this->menu_item_callback, this);
        submenu_add_item(
            this->menu, "Stop", (uint32_t)MainMenuOptions::Stop, &this->menu_item_callback, this);
        submenu_add_item(
            this->menu,
            "Set Frequency",
            (uint32_t)MainMenuOptions::SetFreq,
            &this->menu_item_callback,
            this);
        submenu_add_item(
            this->menu,
            "Set Song",
            (uint32_t)MainMenuOptions::SetSong,
            &this->menu_item_callback,
            this);
        view_set_previous_callback(this->getView(), &this->previous_callback);
    }

    ~MainView() {
        submenu_free(this->menu);
    }

    View* getView() {
        return submenu_get_view(this->menu);
    }

    MainView(const MainView& obj) = delete;
    MainView& operator=(const MainView& obj) = delete;
};