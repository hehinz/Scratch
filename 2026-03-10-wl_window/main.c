#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <wayland-client.h>
#include "xdg-shell-client-protocol.h"
#include "xdg-decoration-client-protocol.h"

struct app_state {
    struct wl_shm *shm;
    struct wl_compositor *compositor;
    struct xdg_wm_base *xdg_wm_base;
    struct wl_surface *surface;
    struct wl_buffer *buffer; // The listener needs this!
    struct zxdg_decoration_manager_v1 *deco_manager;
    int configured;
    int width;
    int height;
};


static void xdg_surface_configure(void *data, struct xdg_surface *xdg_surface, uint32_t serial) {
    struct app_state *state = data; // Retrieve our struct
    xdg_surface_ack_configure(xdg_surface, serial);

    if (!state->configured) {
        // Now the listener has access to the buffer and surface
        wl_surface_attach(state->surface, state->buffer, 0, 0);
        wl_surface_damage(state->surface, 0, 0, 400, 300);
        wl_surface_commit(state->surface);
        state->configured = 1;
    }
}

static const struct xdg_surface_listener xdg_surface_listener = { xdg_surface_configure };

static void xdg_toplevel_configure(void *data, struct xdg_toplevel *toplevel, int32_t w, int32_t h, struct wl_array *states) {}
static void xdg_toplevel_close(void *data, struct xdg_toplevel *toplevel) { exit(0); }
static const struct xdg_toplevel_listener xdg_toplevel_listener = { xdg_toplevel_configure, xdg_toplevel_close };

static void wm_base_ping(void *data, struct xdg_wm_base *wm_base, uint32_t serial) {
    xdg_wm_base_pong(wm_base, serial);
}
static const struct xdg_wm_base_listener wm_base_listener = { wm_base_ping };

static void registry_handle_global(void *data, struct wl_registry *reg, uint32_t id, const char *intf, uint32_t ver) {
    struct app_state *state = data;
    if (strcmp(intf, "wl_compositor") == 0) {
        state->compositor = wl_registry_bind(reg, id, &wl_compositor_interface, 1);
    } else if (strcmp(intf, "wl_shm") == 0) {
        state->shm = wl_registry_bind(reg, id, &wl_shm_interface, 1);
    } else if (strcmp(intf, "xdg_wm_base") == 0) {
        state->xdg_wm_base = wl_registry_bind(reg, id, &xdg_wm_base_interface, 1);
        xdg_wm_base_add_listener(state->xdg_wm_base, &wm_base_listener, NULL);
    } if (strcmp(intf, "zxdg_decoration_manager_v1") == 0) {
        state->deco_manager = wl_registry_bind(reg, id, &zxdg_decoration_manager_v1_interface, 1);
    }
}
static const struct wl_registry_listener registry_listener = { registry_handle_global, NULL };

int main() {
    struct app_state state = { .width = 400, .height = 300, .configured = 0 };

    struct wl_display *display = wl_display_connect(NULL);
    if (!display) return 1;

    struct wl_registry *registry = wl_display_get_registry(display);
    wl_registry_add_listener(registry, &registry_listener, &state);
    wl_display_roundtrip(display);

    // 1. Prepare Shared Memory
    int stride = state.width * 4;
    int size = stride * state.height;
    int fd = memfd_create("shm_buffer", MFD_CLOEXEC);
    ftruncate(fd, size);
    uint32_t *pixels = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    // 2. Draw Bitmap (Orange Gradient)
    for (int y = 0; y < state.height; y++) {
        for (int x = 0; x < state.width; x++) {
            pixels[y * state.width + x] = (0xFF << 24) | (255 << 16) | (x % 255 << 8) | 0;
        }
    }

    // 3. Create Wayland Buffer
    struct wl_shm_pool *pool = wl_shm_create_pool(state.shm, fd, size);
    state.buffer = wl_shm_pool_create_buffer(pool, 0, state.width, state.height, stride, WL_SHM_FORMAT_XRGB8888);
    wl_shm_pool_destroy(pool);
    close(fd);

    // 4. Create Surface and Shell Objects
    state.surface = wl_compositor_create_surface(state.compositor);
    struct xdg_surface *xdg_surf = xdg_wm_base_get_xdg_surface(state.xdg_wm_base, state.surface);
    xdg_surface_add_listener(xdg_surf, &xdg_surface_listener, &state);

    struct xdg_toplevel *toplevel = xdg_surface_get_toplevel(xdg_surf);
    xdg_toplevel_add_listener(toplevel, &xdg_toplevel_listener, &state);
    xdg_toplevel_set_title(toplevel, "Wayland Bitmap");

    if (state.deco_manager) {
        struct zxdg_toplevel_decoration_v1 *deco =
            zxdg_decoration_manager_v1_get_toplevel_decoration(state.deco_manager, toplevel);

        // Request the server to draw decorations
        zxdg_toplevel_decoration_v1_set_mode(deco, ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE);
    }

    // 5. Initial Commit (Triggers the Configure Event)
    wl_surface_commit(state.surface);

    printf("Starting event loop...\n");
    while (wl_display_dispatch(display) != -1) {
        // Main loop
    }

    return 0;
}
