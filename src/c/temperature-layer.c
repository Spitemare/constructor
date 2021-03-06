#ifndef PBL_PLATFORM_APLITE
#include "common.h"
#include <pebble-fctx/fctx.h>
#include <pebble-generic-weather/pebble-generic-weather.h>
#include "fonts.h"
#include "fctx-rect-layer.h"
#include "fctx-text-layer.h"
#include "weather.h"
#include "temperature-layer.h"

typedef struct {
    char buf[32];
    FctxRectLayer *rect_layer;
    FctxTextLayer *text_layer;
    EventHandle settings_event_handle;
    EventHandle weather_event_handle;
} Data;

static void weather_handler(GenericWeatherInfo *info, GenericWeatherStatus status, void *this) {
    log_func();
    Data *data = fctx_layer_get_data(this);
    char s[8];
    if (status == GenericWeatherStatusAvailable) {
        int unit = atoi(enamel_get_WEATHER_UNIT());
        snprintf(s, sizeof(s), "%d", unit == 1 ? info->temp_f : info->temp_c);
    } else if (status != GenericWeatherStatusPending) {
        snprintf(s, sizeof(s), "--");
    } else {
        snprintf(s, sizeof(s), "00");
    }

    snprintf(data->buf, sizeof(data->buf), "%s%s%s", enamel_get_TEMPERATURE_PREFIX(), s, enamel_get_TEMPERATURE_SUFFIX());
    layer_mark_dirty(this);
}

static void settings_handler(void *this) {
    log_func();
    Data *data = fctx_layer_get_data(this);
    FPoint offset = FPointI(enamel_get_TEMPERATURE_X(), enamel_get_TEMPERATURE_Y());
    uint32_t rotation = DEG_TO_TRIGANGLE(enamel_get_TEMPERATURE_ROTATION());
    GTextAlignment alignment = atoi(enamel_get_TEMPERATURE_ALIGNMENT());

    fctx_rect_layer_set_fill_color(data->rect_layer, enamel_get_TEMPERATURE_RECT_FILL_COLOR());
    FSize size = FSizeI(enamel_get_TEMPERATURE_RECT_SIZE_W(), enamel_get_TEMPERATURE_RECT_SIZE_H());
    fctx_rect_layer_set_size(data->rect_layer, size);
    fctx_rect_layer_set_offset(data->rect_layer, offset);
    fctx_rect_layer_set_rotation(data->rect_layer, rotation);
    fctx_rect_layer_set_border_color(data->rect_layer, enamel_get_TEMPERATURE_RECT_BORDER_COLOR());
    fctx_rect_layer_set_border_width(data->rect_layer, enamel_get_TEMPERATURE_RECT_BORDER_WIDTH());
    fctx_rect_layer_set_alignment(data->rect_layer, alignment);

    fctx_text_layer_set_alignment(data->text_layer, alignment);
    fctx_text_layer_set_em_height(data->text_layer, enamel_get_TEMPERATURE_FONT_SIZE());
    fctx_text_layer_set_fill_color(data->text_layer, enamel_get_TEMPERATURE_COLOR());
    fctx_text_layer_set_offset(data->text_layer, offset);
    fctx_text_layer_set_rotation(data->text_layer, rotation);

    weather_handler(weather_peek(), weather_status_peek(), this);
}

TemperatureLayer *temperature_layer_create(void) {
    log_func();
    TemperatureLayer *this = fctx_layer_create_with_data(sizeof(Data));
    Data *data = fctx_layer_get_data(this);

    data->rect_layer = fctx_rect_layer_create();
    fctx_layer_add_child(this, data->rect_layer);

    data->text_layer = fctx_text_layer_create();
    fctx_layer_add_child(this, data->text_layer);
    fctx_text_layer_set_anchor(data->text_layer, FTextAnchorMiddle);
    fctx_text_layer_set_font(data->text_layer, fonts_get(RESOURCE_ID_LECO_FFONT));
    fctx_text_layer_set_text(data->text_layer, data->buf);

    settings_handler(this);
    data->settings_event_handle = enamel_settings_received_subscribe(settings_handler, this);
    data->weather_event_handle = events_weather_subscribe(weather_handler, this);

    return this;
}

void temperature_layer_destroy(TemperatureLayer *this) {
    log_func();
    Data *data = fctx_layer_get_data(this);
    events_weather_unsubscribe(data->weather_event_handle);
    enamel_settings_received_unsubscribe(data->settings_event_handle);
    fctx_text_layer_destroy(data->text_layer);
    fctx_rect_layer_destroy(data->rect_layer);
    fctx_layer_destroy(this);
}
#endif // PBL_PLATFORM_APLITE
