#include "includes.hpp"
#include "helpers/comms.hpp"
#include "views.hpp"

#include <memory>

class SelectSongView {
private:
    Submenu* submenu;
    ViewDispatcher* vd;
    RefreshEventCallbackKey cb;

    static void onEnter(void* ctx) {
        UNUSED(ctx);

        CommunicationHandler::getInstance()->refresh_songs();
    }

    static void onSelect(void* ctx, uint32_t idx) {
        SelectSongView* ssv = (SelectSongView*)ctx;
        CommunicationHandler::getInstance()->setSong(idx);
        view_dispatcher_switch_to_view(ssv->vd, static_cast<uint32_t>(Views::MainView));
    }

    static uint32_t onPrevious(void* ctx) {
        UNUSED(ctx);
        return static_cast<uint32_t>(Views::MainView);
    }

public:
    SelectSongView(ViewDispatcher* vd) {
        this->vd = vd;
        submenu = submenu_alloc();

        submenu_set_header(submenu, "Select Song");

        view_set_enter_callback(getView(), &onEnter);
        view_set_previous_callback(getView(), &onPrevious);

        cb = CommunicationHandler::getInstance()->add_refresh_callback(
            [this](std::vector<std::string> songs) {
                submenu_reset(submenu);
                for(uint32_t i = 0; i < songs.size(); i++) {
                    submenu_add_item(this->submenu, songs[i].c_str(), i, &this->onSelect, this);
                }
            });
    }

    ~SelectSongView() {
        CommunicationHandler::getInstance()->remove_refresh_callback(cb);
        submenu_free(submenu);
    }

    View* getView() {
        return submenu_get_view(submenu);
    }
};