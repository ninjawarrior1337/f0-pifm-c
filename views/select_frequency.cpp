#include "includes.hpp"
#include "helpers/comms.hpp"
#include <memory>

class SelectFrequencyViewModel {
public:
    int current_frequency;
};

class SelectFrequencyView {
private:
    View* view;

    static void draw_callback(Canvas* canvas, void* model) {
        SelectFrequencyViewModel* vm = (SelectFrequencyViewModel*)model;
        canvas_clear(canvas);
        canvas_set_font(canvas, FontBigNumbers);
        char temp_str[32];

        snprintf(
            temp_str,
            32,
            "%d.%.1d",
            vm->current_frequency / 1000000,
            (vm->current_frequency % 1000000) / 100000);

        canvas_draw_str_aligned(canvas, 64, 32, AlignCenter, AlignCenter, temp_str);
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str_aligned(canvas, 64, 32 + 16, AlignCenter, AlignCenter, "MHz");
    }

    static bool input_callback(InputEvent* event, void* ctx) {
        SelectFrequencyView* sfv = (SelectFrequencyView*)ctx;
        SelectFrequencyViewModel* model = (SelectFrequencyViewModel*)view_get_model(sfv->view);

        if(event->key == InputKeyUp) {
            if(event->type == InputTypeRepeat) {
                model->current_frequency += 1000000;
            }

            if(event->type == InputTypeShort) {
                model->current_frequency += 100000;
            }
        }

        if(event->key == InputKeyDown) {
            if(event->type == InputTypeRepeat) {
                model->current_frequency -= 1000000;
            }

            if(event->type == InputTypeShort) {
                model->current_frequency -= 100000;
            }
        }

        view_commit_model(sfv->view, true);

        return false;
    }

    static uint32_t previous_callback(void* ctx) {
        SelectFrequencyView* sfv = (SelectFrequencyView*)ctx;
        SelectFrequencyViewModel* model = (SelectFrequencyViewModel*)view_get_model(sfv->view);

        CommunicationHandler::getInstance()->set_frequency(model->current_frequency);

        return (uint32_t)Views::MainView;
    }

public:
    SelectFrequencyView() {
        view = view_alloc();

        view_set_context(view, this);
        view_allocate_model(view, ViewModelTypeLockFree, sizeof(SelectFrequencyViewModel));

        SelectFrequencyViewModel* vm = (SelectFrequencyViewModel*)view_get_model(view);
        vm->current_frequency = 88000000;

        view_set_draw_callback(view, &draw_callback);
        view_set_input_callback(view, &input_callback);
        view_set_previous_callback(view, &previous_callback);
    }

    ~SelectFrequencyView() {
        view_free_model(view);
        view_free(view);
    }

    View* getView() {
        return view;
    }
};