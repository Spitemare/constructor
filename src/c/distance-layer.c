#ifdef PBL_HEALTH
#include "common.h"
#include <pebble-events/pebble-events.h>
#include <pebble-fctx/fctx.h>
#include "fonts.h"
#include "fctx-layer.h"
#include "fctx-rect-layer.h"
#include "fctx-text-layer.h"
#include "distance-layer.h"

typedef struct {
    char buf[32];
    FctxRectLayer *rect_layer;
    FctxTextLayer *text_layer;
    EventHandle health_event_handle;
    EventHandle settings_event_handle;
} Data;

static void health_handler(HealthEventType event, void *this) {
    log_func();
    if (event == HealthEventSignificantUpdate) {
        health_handler(HealthEventMovementUpdate, this);
    } else if (event == HealthEventMovementUpdate) {
        Data *data = fctx_layer_get_data(this);
        time_t start = time_start_of_today();
        time_t end = time(NULL);
        char s[8];
        HealthServiceAccessibilityMask mask = health_service_metric_accessible(HealthMetricWalkedDistanceMeters, start, end);
        if (mask & HealthServiceAccessibilityMaskAvailable) {
            HealthValue sum = health_service_sum_today(HealthMetricWalkedDistanceMeters);
            MeasurementSystem system = health_service_get_measurement_system_for_display(HealthMetricWalkedDistanceMeters);
            if (system != MeasurementSystemImperial) {
                if (sum < 100) {
                    snprintf(s, sizeof(s), "%ldm", sum);
                } else if (sum < 1000) {
                    sum /= 100;
                    snprintf(s, sizeof(s), ".%ldkm", sum);
                } else {
                    sum /= 1000;
                    snprintf(s, sizeof(s), "%ldkm", sum);
                }
            } else {
                int tenths = sum * 10 / 1609 % 10;
                int whole = sum / 1609;
                if (whole < 10) {
                    snprintf(s, sizeof(s), "%d.%dmi", whole, tenths);
                } else {
                    snprintf(s, sizeof(s), "%dmi", whole);
                }
            }
        } else {
            s[0] = '\0';
        }
        snprintf(data->buf, sizeof(data->buf), "%s%s%s", enamel_get_DISTANCE_PREFIX(), s, enamel_get_DISTANCE_SUFFIX());
        layer_mark_dirty(this);
    }
}

static void settings_handler(void *this) {
    log_func();
    Data *data = fctx_layer_get_data(this);
    FPoint offset = FPointI(enamel_get_DISTANCE_X(), enamel_get_DISTANCE_Y());
    uint32_t rotation = DEG_TO_TRIGANGLE(enamel_get_DISTANCE_ROTATION());
    GTextAlignment alignment = atoi(enamel_get_DISTANCE_ALIGNMENT());

    fctx_rect_layer_set_fill_color(data->rect_layer, enamel_get_DISTANCE_RECT_FILL_COLOR());
    FSize size = FSizeI(enamel_get_DISTANCE_RECT_SIZE_W(), enamel_get_DISTANCE_RECT_SIZE_H());
    fctx_rect_layer_set_size(data->rect_layer, size);
    fctx_rect_layer_set_offset(data->rect_layer, offset);
    fctx_rect_layer_set_rotation(data->rect_layer, rotation);
    fctx_rect_layer_set_border_color(data->rect_layer, enamel_get_DISTANCE_RECT_BORDER_COLOR());
    fctx_rect_layer_set_border_width(data->rect_layer, enamel_get_DISTANCE_RECT_BORDER_WIDTH());
    fctx_rect_layer_set_alignment(data->rect_layer, alignment);

    fctx_text_layer_set_alignment(data->text_layer, alignment);
    fctx_text_layer_set_em_height(data->text_layer, enamel_get_DISTANCE_FONT_SIZE());
    fctx_text_layer_set_fill_color(data->text_layer, enamel_get_DISTANCE_COLOR());
    fctx_text_layer_set_offset(data->text_layer, offset);
    fctx_text_layer_set_rotation(data->text_layer, rotation);

    health_handler(HealthEventSignificantUpdate, this);
}

DistanceLayer *distance_layer_create(void) {
    log_func();
    DistanceLayer *this = fctx_layer_create_with_data(sizeof(Data));
    Data *data = fctx_layer_get_data(this);

    data->rect_layer = fctx_rect_layer_create();
    fctx_layer_add_child(this, data->rect_layer);

    data->text_layer = fctx_text_layer_create();
    fctx_layer_add_child(this, data->text_layer);
    fctx_text_layer_set_anchor(data->text_layer, FTextAnchorMiddle);
    fctx_text_layer_set_font(data->text_layer, fonts_get(RESOURCE_ID_LECO_FFONT));
    fctx_text_layer_set_text(data->text_layer, data->buf);

    health_handler(HealthEventSignificantUpdate, this);
    data->health_event_handle = events_health_service_events_subscribe(health_handler, this);

    settings_handler(this);
    data->settings_event_handle = enamel_settings_received_subscribe(settings_handler, this);

    return this;
}

void distance_layer_destroy(DistanceLayer *this) {
    log_func();
    Data *data = fctx_layer_get_data(this);
    enamel_settings_received_unsubscribe(data->settings_event_handle);
    events_health_service_events_unsubscribe(data->health_event_handle);
    fctx_text_layer_destroy(data->text_layer);
    fctx_rect_layer_destroy(data->rect_layer);
    fctx_layer_destroy(this);
}
#endif // PBL_HEALTH
