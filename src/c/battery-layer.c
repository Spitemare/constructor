#include "common.h"
#include <pebble-events/pebble-events.h>
#include <pebble-fctx/fctx.h>
#include "fonts.h"
#include "fctx-rect-layer.h"
#include "fctx-text-layer.h"
#include "battery-layer.h"

typedef struct {
    char buf[PBL_IF_LOW_MEM_ELSE(8, 32)];
#ifndef PBL_PLATFORM_APLITE
    FctxRectLayer *rect_layer;
#endif
    FctxTextLayer *text_layer;
    EventHandle battery_state_event_handle;
    EventHandle settings_event_handle;
} Data;

static void battery_state_handler(BatteryChargeState state, void *this) {
    log_func();
    Data *data = fctx_layer_get_data(this);
#ifdef PBL_PLATFORM_APLITE
    snprintf(data->buf, sizeof(data->buf), "%d%%", state.charge_percent);
#else
    snprintf(data->buf, sizeof(data->buf), "%s%d%s", enamel_get_BATTERY_PREFIX(), state.charge_percent, enamel_get_BATTERY_SUFFIX());
#endif
    layer_mark_dirty(this);
}

static void settings_handler(void *this) {
    log_func();
    Data *data = fctx_layer_get_data(this);
    FPoint offset = FPointI(enamel_get_BATTERY_X(), enamel_get_BATTERY_Y());
    uint32_t rotation = DEG_TO_TRIGANGLE(enamel_get_BATTERY_ROTATION());
    GTextAlignment alignment = atoi(enamel_get_BATTERY_ALIGNMENT());

#ifndef PBL_PLATFORM_APLITE
    fctx_rect_layer_set_fill_color(data->rect_layer, enamel_get_BATTERY_RECT_FILL_COLOR());
    FSize size = FSizeI(enamel_get_BATTERY_RECT_SIZE_W(), enamel_get_BATTERY_RECT_SIZE_H());
    fctx_rect_layer_set_size(data->rect_layer, size);
    fctx_rect_layer_set_offset(data->rect_layer, offset);
    fctx_rect_layer_set_rotation(data->rect_layer, rotation);
    fctx_rect_layer_set_border_color(data->rect_layer, enamel_get_BATTERY_RECT_BORDER_COLOR());
    fctx_rect_layer_set_border_width(data->rect_layer, enamel_get_BATTERY_RECT_BORDER_WIDTH());
    fctx_rect_layer_set_alignment(data->rect_layer, alignment);
#endif

    fctx_text_layer_set_alignment(data->text_layer, alignment);
    fctx_text_layer_set_em_height(data->text_layer, enamel_get_BATTERY_FONT_SIZE());
    fctx_text_layer_set_fill_color(data->text_layer, enamel_get_BATTERY_COLOR());
    fctx_text_layer_set_offset(data->text_layer, offset);
    fctx_text_layer_set_rotation(data->text_layer, rotation);

    battery_state_handler(battery_state_service_peek(), this);
}

BatteryLayer *battery_layer_create(void) {
    log_func();
    BatteryLayer *this = fctx_layer_create_with_data(sizeof(Data));
    Data *data = fctx_layer_get_data(this);

#ifndef PBL_PLATFORM_APLITE
    data->rect_layer = fctx_rect_layer_create();
    fctx_layer_add_child(this, data->rect_layer);
#endif

    data->text_layer = fctx_text_layer_create();
    fctx_layer_add_child(this, data->text_layer);
    fctx_text_layer_set_anchor(data->text_layer, FTextAnchorMiddle);
    fctx_text_layer_set_font(data->text_layer, fonts_get(RESOURCE_ID_LECO_FFONT));
    fctx_text_layer_set_text(data->text_layer, data->buf);

    battery_state_handler(battery_state_service_peek(), this);
    data->battery_state_event_handle = events_battery_state_service_subscribe_context(battery_state_handler, this);

    settings_handler(this);
    data->settings_event_handle = enamel_settings_received_subscribe(settings_handler, this);

    return this;
}

void battery_layer_destroy(BatteryLayer *this) {
    log_func();
    Data *data = fctx_layer_get_data(this);
    enamel_settings_received_unsubscribe(data->settings_event_handle);
    events_battery_state_service_unsubscribe(data->battery_state_event_handle);
    fctx_text_layer_destroy(data->text_layer);
#ifndef PBL_PLATFORM_APLITE
    fctx_rect_layer_destroy(data->rect_layer);
#endif
    fctx_layer_destroy(this);
}
