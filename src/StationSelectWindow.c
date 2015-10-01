#include "StationSelectWindow.h"

static Window *window;
static Layer *window_layer;
static Layer *secondary_bg;
static MenuLayer *menu_layer;

static GRect secondary_bg_rect;
static GRect textrect;
static GRect menulayerrect;

static bool line_select;
static GPoint left_arrow;
static GPoint right_arrow;
static Animation *animation;
static char *line_text;

static uint16_t menu_get_num_rows(MenuLayer *menu_layer, uint16_t section_index, void *callback_context) {
    return lines[station_select_line].stations;
}

static void menu_draw_row(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *callback_context) {
    GRect bounds = layer_get_bounds(cell_layer);
    graphics_fill_rect(ctx, bounds, 0, GCornerNone);
    graphics_draw_text(ctx,
                       stations[graph_index[lines[station_select_line].startfrom + cell_index->row]].name,
                       fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD),
                       bounds,
                       GTextOverflowModeWordWrap,
                       GTextAlignmentLeft,
                       NULL);
    //menu_cell_basic_draw(ctx, cell_layer, stations[graph_index[lines[station_select_line].startfrom + cell_index->row]].name, NULL, NULL);
}

static void update_left_animation(Animation *anim, const AnimationProgress progress) {
    if(progress <= ANIMATION_NORMALIZED_MAX / 2) {
        textrect = GRect(10 - 134 * 2 * progress / ANIMATION_NORMALIZED_MAX, 90, 124, 56);
        left_arrow = GPoint(4 - 6 * 2 * progress / ANIMATION_NORMALIZED_MAX, 86);
    } else {
        textrect = GRect(144 - 134 * 2 * (progress - ANIMATION_NORMALIZED_MAX / 2) / ANIMATION_NORMALIZED_MAX, 90, 124, 56);
        left_arrow = GPoint(-2 + 6 * 2 * (progress - ANIMATION_NORMALIZED_MAX / 2) / ANIMATION_NORMALIZED_MAX, 86);
    }
    layer_mark_dirty(secondary_bg);
}

static void update_right_animation(Animation *anim, const AnimationProgress progress) {
    if(progress <= ANIMATION_NORMALIZED_MAX / 2) {
        textrect = GRect(10 + 134 * 2 * progress / ANIMATION_NORMALIZED_MAX, 90, 124, 56);
        right_arrow = GPoint(140 + 6 * 2 * progress / ANIMATION_NORMALIZED_MAX, 86);
    } else {
        textrect = GRect(-144 + 154 * 2 * (progress - ANIMATION_NORMALIZED_MAX / 2) / ANIMATION_NORMALIZED_MAX, 90, 124, 56);
        right_arrow = GPoint(146 - 6 * 2 * (progress - ANIMATION_NORMALIZED_MAX / 2) / ANIMATION_NORMALIZED_MAX, 86);
    }
    layer_mark_dirty(secondary_bg);
}

static void update_select_animation(Animation *anim, const AnimationProgress progress) {
    textrect = GRect(10, 90 - 85 * progress / ANIMATION_NORMALIZED_MAX, 124, 56);
    secondary_bg_rect = GRect(0, 0, 144, 168 - 124 * 2 * progress / ANIMATION_NORMALIZED_MAX);
    layer_set_frame(secondary_bg, secondary_bg_rect);
}

static void show_menu(Animation *anim, bool finished, void *context) {
    menulayerrect = GRect(0, 0, 144, 168);
    layer_set_frame(menu_layer_get_layer(menu_layer), menulayerrect);
}

static AnimationImplementation left_implementation = {
        .update = update_left_animation
};

static AnimationImplementation right_implementation = {
        .update = update_right_animation
};

static AnimationImplementation select_implementation = {
        .update = update_select_animation
};

static void close_station_select_window(ClickRecognizerRef recognizer, void *context) {
    window_stack_remove(window, true);
    main_window_revert_back();
    window_destroy(window);
}

static void push_up(ClickRecognizerRef recognizer, void *context) {
    station_select_line = station_select_line >= 4 ? 0 : station_select_line + 1;
    line_text[11] = (char)('0' + station_select_line + 1);
    animation = animation_create();
    animation_set_implementation(animation, &left_implementation);
    animation_schedule(animation);
}

static void push_select(ClickRecognizerRef recognizer, void *context) {
    menu_layer_set_highlight_colors(menu_layer, lines[station_select_line].color, GColorWhite);
    menu_layer_reload_data(menu_layer);
    menu_layer_set_click_config_onto_window(menu_layer, window);
    line_select = false;
    animation = animation_create();
    animation_set_implementation(animation, &select_implementation);
    animation_set_handlers(animation, (AnimationHandlers) {.stopped= show_menu}, NULL);
    animation_schedule(animation);
}

static void push_down(ClickRecognizerRef recognizer, void *context) {
    station_select_line = station_select_line <= 0 ? 4 : station_select_line - 1;
    line_text[11] = (char)('0' + station_select_line + 1);
    animation = animation_create();
    animation_set_implementation(animation, &right_implementation);
    animation_schedule(animation);
}

static void click_config_provider(void *context) {
    window_single_repeating_click_subscribe(BUTTON_ID_UP, 300, push_up);
    window_single_click_subscribe(BUTTON_ID_SELECT, push_select);
    window_single_repeating_click_subscribe(BUTTON_ID_DOWN, 300, push_down);
    window_single_click_subscribe(BUTTON_ID_BACK, close_station_select_window);
}

static void update_proc(Layer * this, GContext *ctx) {
    graphics_context_set_fill_color(ctx, lines[station_select_line].color);
    graphics_fill_rect(ctx, layer_get_bounds(this), 0, GCornerNone);
    graphics_context_set_text_color(ctx, GColorWhite);
    if(line_select) {
        graphics_context_set_stroke_color(ctx, GColorWhite);
        graphics_context_set_stroke_width(ctx, 2);
        graphics_draw_line(ctx, GPoint(left_arrow.x + 4, left_arrow.y - 6), left_arrow);
        graphics_draw_line(ctx, left_arrow, GPoint(left_arrow.x + 4, left_arrow.y + 6));
        graphics_draw_line(ctx, GPoint(right_arrow.x - 4, right_arrow.y - 6), right_arrow);
        graphics_draw_line(ctx, right_arrow, GPoint(right_arrow.x - 4, right_arrow.y + 6));

        for(int x = 0; x < 5; x++) {
            graphics_context_set_fill_color(ctx, GColorWhite);
            if(x == station_select_line) {
                graphics_context_set_fill_color(ctx, GColorBlack);
            }
            graphics_fill_circle(ctx, GPoint(56 + x * 8, 160), 2);
        }

        graphics_draw_text(ctx,
                           line_text,
                           fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD),
                           GRect(textrect.origin.x, 50, 124, 50),
                           GTextOverflowModeWordWrap,
                           GTextAlignmentCenter,
                           NULL);
        graphics_draw_text(ctx,
                           lines[station_select_line].name,
                           fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD),
                           textrect,
                           GTextOverflowModeWordWrap,
                           GTextAlignmentCenter,
                           NULL);
    }
}

static void load(Window *win) {
    line_text = malloc(13 * sizeof(char));
    strcpy(line_text, "Линия  ");
    line_text[11] = (char)('0' + station_select_line + 1);
    line_select = true;
    left_arrow = GPoint(4, 86);
    right_arrow = GPoint(140, 86);
    textrect = GRect(10, 90, 124, 56);
    menulayerrect = GRect(144, 0, 144, 168);
    window_layer = window_get_root_layer(window);
    GRect frame = layer_get_frame(window_layer);
    secondary_bg_rect = frame;
    secondary_bg = layer_create(secondary_bg_rect);
    layer_set_update_proc(secondary_bg, update_proc);
    layer_add_child(window_layer, secondary_bg);
    menu_layer = menu_layer_create(menulayerrect);
    menu_layer_set_callbacks(menu_layer, NULL, (MenuLayerCallbacks) {
            .get_num_rows = menu_get_num_rows,
            .draw_row = menu_draw_row
    });
    layer_add_child(window_layer, menu_layer_get_layer(menu_layer));
    window_set_click_config_provider(window, click_config_provider);
}

static void unload(Window *win) {
    layer_destroy(secondary_bg);
    free(line_text);
}

void open_station_select_window() {
    window = window_create();
    window_set_window_handlers(window, (WindowHandlers) {
            .load = load,
            .unload = unload
    });
    window_stack_push(window, false);
}