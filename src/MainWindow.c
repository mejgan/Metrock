#include "MainWindow.h"

static Window *window;
static Layer *window_layer;
static Layer *graphics_layer;
static Animation *to_animation;
static Animation *from_animation;
static bool to_animation_playing;
static bool from_animation_playing;

static RectProps to_rect;
static RectProps from_rect;
static RectProps to_gray_rect;
static RectProps from_gray_rect;

static void update_from_animation(Animation *anim, const AnimationProgress progress) {
    from_rect.corner_mask = GCornerNone;
    from_rect.rect = GRect(124 - 124 * progress / ANIMATION_NORMALIZED_MAX,
                    22 - 22 * progress / ANIMATION_NORMALIZED_MAX,
                    12 + 132 * progress / ANIMATION_NORMALIZED_MAX,
                    56 + 112 * progress / ANIMATION_NORMALIZED_MAX);
    layer_mark_dirty(graphics_layer);
}

static void update_fromback_animation(Animation *anim, const AnimationProgress progress) {
    from_rect.corner_mask = GCornersRight;
    from_rect.rect = GRect(0 + 124 * progress / ANIMATION_NORMALIZED_MAX,
                           0 + 22 * progress / ANIMATION_NORMALIZED_MAX,
                           144 - 132 * progress / ANIMATION_NORMALIZED_MAX,
                           168 - 112 * progress / ANIMATION_NORMALIZED_MAX);
    layer_mark_dirty(graphics_layer);
}

static void update_to_animation(Animation *anim, const AnimationProgress progress) {
    to_rect.corner_mask = GCornerNone;
    to_rect.rect = GRect(124 - 124 * progress / ANIMATION_NORMALIZED_MAX,
                    104 - 104 * progress / ANIMATION_NORMALIZED_MAX,
                    12 + 132 * progress / ANIMATION_NORMALIZED_MAX,
                    56 + 112 *  progress / ANIMATION_NORMALIZED_MAX);
    layer_mark_dirty(graphics_layer);
}

static void update_toback_animation(Animation *anim, const AnimationProgress progress) {
    to_rect.corner_mask = GCornersRight;
    to_rect.rect = GRect(0 + 124 * progress / ANIMATION_NORMALIZED_MAX,
                         0 + 104 * progress / ANIMATION_NORMALIZED_MAX,
                         144 - 132 * progress / ANIMATION_NORMALIZED_MAX,
                         168 - 112 *  progress / ANIMATION_NORMALIZED_MAX);
    layer_mark_dirty(graphics_layer);
}

static AnimationImplementation to_implementation = {
        .update= update_to_animation
};

static AnimationImplementation from_implementation = {
        .update= update_from_animation
};

static AnimationImplementation toback_implementation = {
        .update= update_toback_animation
};

static AnimationImplementation fromback_implementation = {
        .update= update_fromback_animation
};

void main_window_revert_back() {
    if(to_animation_playing) {
        to_animation = animation_create();
        animation_set_implementation(to_animation, &toback_implementation);
        animation_schedule(to_animation);
    } else if(from_animation_playing) {
        from_animation = animation_create();
        animation_set_implementation(from_animation, &fromback_implementation);
        animation_schedule(from_animation);
    }
    to_animation_playing = false;
    from_animation_playing = false;
}
static void reset_animations() {
    from_rect.rect = GRect(124, 22, 12, 56);
    from_rect.corner_mask = GCornersRight;
    to_rect.rect = GRect(124, 104, 12, 56);
    to_rect.corner_mask = GCornersRight;
    from_gray_rect.rect = GRect(8, 22, 128, 56);
    from_gray_rect.corner_mask = GCornersAll;
    to_gray_rect.rect = GRect(8, 104, 128, 56);
    to_gray_rect.corner_mask = GCornersAll;
    to_animation_playing = false;
    from_animation_playing = false;
}

static void opend_station_select_callback(Animation *anim, bool finished, void *context) {
    open_station_select_window();
}

static void select_to(ClickRecognizerRef recognizer, void *context) {
    reset_animations();
    to_animation = animation_create();
    animation_set_implementation(to_animation, &to_implementation);
    animation_set_handlers(to_animation, (AnimationHandlers) {.stopped = opend_station_select_callback}, NULL);
    to_animation_playing = true;
    animation_schedule(to_animation);
    station_select_line = stations[graph_index[path_to]].line;
}

static void select_from(ClickRecognizerRef recognizer, void *context) {
    reset_animations();
    from_animation = animation_create();
    animation_set_implementation(from_animation, &from_implementation);
    animation_set_handlers(from_animation, (AnimationHandlers) {.stopped = opend_station_select_callback}, NULL);
    from_animation_playing = true;
    animation_schedule(from_animation);
    station_select_line = stations[graph_index[path_from]].line;
}

static void close_main_window(ClickRecognizerRef recognixer, void *context) {
    window_stack_remove(window, true);
    window_destroy(window);
}

static void click_config_provider(void *context) {
    window_single_click_subscribe(BUTTON_ID_UP, select_from);
    window_single_click_subscribe(BUTTON_ID_DOWN, select_to);
    window_single_click_subscribe(BUTTON_ID_BACK, close_main_window);
}

static void update_proc(Layer *this, GContext *ctx) {
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_rect(ctx, GRect(0, 0, 144, 168), 0, GCornerNone);

    if(!to_animation_playing && !from_animation_playing) {
        graphics_context_set_fill_color(ctx, GColorDarkGray);
        graphics_fill_rect(ctx, GRect(4, 4, 136, 78), 5, GCornersAll);
        graphics_fill_rect(ctx, GRect(4, 86, 136, 78), 5, GCornersAll);

        graphics_context_set_fill_color(ctx, GColorLightGray);
        graphics_fill_rect(ctx, from_gray_rect.rect, 5, from_gray_rect.corner_mask);
        graphics_fill_rect(ctx, to_gray_rect.rect, 5, to_gray_rect.corner_mask);

        graphics_context_set_text_color(ctx, GColorBlack);
        graphics_draw_text(ctx, "Отсюда:", fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD), GRect(8, 3, 132, 26),
                           GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);
        graphics_draw_text(ctx, "Сюда:", fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD), GRect(8, 85, 132, 26),
                           GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);
        graphics_draw_text(ctx, stations[graph_index[path_from]].name, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD),
                           GRect(12, 20, 95, 56), GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);
        graphics_draw_text(ctx, stations[graph_index[path_to]].name, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD),
                           GRect(12, 102, 95, 56), GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);
    }

    if(!to_animation_playing) {
        graphics_context_set_fill_color(ctx, lines[stations[graph_index[path_from]].line].color);
        graphics_fill_rect(ctx, from_rect.rect, 5, from_rect.corner_mask);
    }

    if(!from_animation_playing) {
        graphics_context_set_fill_color(ctx, lines[stations[graph_index[path_to]].line].color);
        graphics_fill_rect(ctx, to_rect.rect, 5, to_rect.corner_mask);
    }

}

static void load(Window *win) {
    path_from = 0;
    path_to = 26;
    reset_animations();
    window_layer = window_get_root_layer(window);
    GRect frame = layer_get_frame(window_layer);
    graphics_layer = layer_create(frame);
    layer_set_update_proc(graphics_layer, update_proc);
    layer_add_child(window_layer, graphics_layer);
    window_set_click_config_provider(window, click_config_provider);
}

static void unload(Window *win) {
    layer_destroy(graphics_layer);
}

void open_main_window() {
    window = window_create();
    window_set_window_handlers(window, (WindowHandlers) {
            .load = load,
            .unload = unload
    });
    window_stack_push(window, true);
}