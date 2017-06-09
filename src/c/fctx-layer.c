#include "common.h"
#include <pebble-fctx/fctx.h>
#include <@smallstoneapps/linked-list/linked-list.h>
#include "fctx-layer.h"

typedef struct {
    FctxLayerUpdateProc update_proc;
#ifdef PBL_QUICK_VIEW_ENABLED
    FPoint offset;
#endif
    void *data;
    LinkedRoot *children;
} Data;

static bool linked_list_update_proc_callback(void *this, void *context) {
    log_func();
    fctx_layer_update_proc(this, context);
    return true;
}

void fctx_layer_update_proc(FctxLayer *this, FContext *fctx) {
    log_func();
    if (layer_get_hidden(this)) return;

    Data *data = layer_get_data(this);
    if (data->update_proc) {
#ifdef PBL_QUICK_VIEW_ENABLED
        data->offset = FPointZero;
        if (enamel_get_HANDLE_PEEKS()) {
            GRect bounds = layer_get_bounds(this);
            GRect unobstructed_bounds = layer_get_unobstructed_bounds(this);
            data->offset = FPointI(0, -(bounds.size.h - unobstructed_bounds.size.h));
        }
        fctx_set_offset(fctx, data->offset);
#else
        fctx_set_offset(fctx, FPointZero);
#endif
        fctx_set_scale(fctx, FPointOne, FPointOne);
        fctx_set_rotation(fctx, 0);

        data->update_proc(this, fctx);
    }
    if (data->children) linked_list_foreach(data->children, linked_list_update_proc_callback, fctx);
}

static void update_proc(Layer *this, GContext *ctx) {
    log_func();
    Data *data = layer_get_data(this);
#ifdef PBL_COLOR
    fctx_enable_aa(false);
#endif
    FContext fctx;
    fctx_init_context(&fctx, ctx);
    logd("heap used: %d", heap_bytes_used());
    if (data->children) linked_list_foreach(data->children, linked_list_update_proc_callback, &fctx);
    logd("heap free: %d", heap_bytes_free());
    fctx_deinit_context(&fctx);
}

FctxLayer *fctx_layer_get_root_layer(Window *window) {
    log_func();
    Layer *layer = window_get_root_layer(window);
    FctxLayer *this = fctx_layer_create();
    layer_set_update_proc(this, update_proc);
    layer_add_child(layer, this);
    return this;
}

FctxLayer *fctx_layer_create() {
    log_func();
    FctxLayer *this = layer_create_with_data(GRect(0, 0, PBL_DISPLAY_WIDTH, PBL_DISPLAY_HEIGHT), sizeof(Data));
    Data *data = layer_get_data(this);
    data->update_proc = NULL;
    data->data = NULL;
    data->children = NULL;
    return this;
}

FctxLayer *fctx_layer_create_with_data(size_t data_size) {
    log_func();
    FctxLayer *this = fctx_layer_create();
    Data *data = layer_get_data(this);
    data->data = malloc(data_size);
    return this;
}

void fctx_layer_destroy(FctxLayer *this) {
    log_func();
    Data *data = layer_get_data(this);
    data->update_proc = NULL;
    if (data->children) {
        linked_list_clear(data->children);
        free(data->children);
        data->children = NULL;
    }
    if (data->data) {
        free(data->data);
        data->data = NULL;
    }
    layer_destroy(this);
}

void fctx_layer_set_update_proc(FctxLayer *this, FctxLayerUpdateProc update_proc) {
    log_func();
    Data *data = layer_get_data(this);
    data->update_proc = update_proc;
    layer_mark_dirty(this);
}

void fctx_layer_add_child(FctxLayer *this, FctxLayer *child) {
    log_func();
    Data *data = layer_get_data(this);
    if (!data->children) data->children = linked_list_create_root();
    linked_list_append(data->children, child);
    layer_add_child(this, child);
    layer_mark_dirty(this);
}

void fctx_layer_remove_child(FctxLayer *this, FctxLayer *child) {
    log_func();
    Data *data = layer_get_data(this);
    if (!data->children) return;
    int16_t index = linked_list_find(data->children, child);
    if (index != -1) {
        linked_list_remove(data->children, index);
        layer_remove_from_parent(child);
    }
}

void *fctx_layer_get_data(FctxLayer *this) {
    log_func();
    Data *data = layer_get_data(this);
    return data->data;
}

#ifdef PBL_QUICK_VIEW_ENABLED
FPoint fctx_layer_get_offset(FctxLayer *this) {
    log_func();
    Data *data = layer_get_data(this);
    return data->offset;
}
#endif

FRect fctx_layer_get_bounds(FctxLayer *this) {
    log_func();
    return g2frect(layer_get_bounds(this));
}

#ifndef PBL_PLATFORM_APLITE
FRect fctx_layer_get_unobstructed_bounds(FctxLayer *this) {
    log_func();
    return g2frect(layer_get_unobstructed_bounds(this));
}
#endif
