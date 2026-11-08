#ifndef TULIP_H_INCLUDED
#define TULIP_H_INCLUDED

#include <stddef.h>

#define TUI_ARRAY_DEFINE(ty, name) \
        typedef struct name { \
                ty *data; \
                size_t len, cap; \
        } name
#define tui_array_empty() {0}
#define tui_array_empty2(arr_ty) \
        (arr_ty) { \
                .data = NULL, \
                .len = 0, \
                .cap = 0, \
        }
#define tui_array_append(da, value) \
        do { \
                if ((da).len >= (da).cap) { \
                        (da).cap = (da).cap ? (da).cap * 2 : 2; \
                        (da).data = (typeof(*((da).data)) *) \
                        realloc((da).data, (da).cap * sizeof(*((da).data))); \
                } \
                (da).data[(da).len++] = (value); \
        } while (0)
#define tui_array_free(da) \
        do { \
                if ((da).data != NULL) { \
                        free((da).data); \
                } \
                (da).data = NULL; \
                (da).len = (da).cap = 0; \
        } while (0)

#define tui_array_at_s(da, i) \
        ((i) < (da).len ? (da).data[i] : (fprintf(stderr, \
                        "[array error]: index %zu is out of bounds (len = %zu)\n", \
                        (size_t)(i), (size_t)(da).len), exit(1), (da).data[0]))

#define tui_array_at(da, i) ((da).data[i])

#define tui_array_clear(da) (da).len = 0;

#define tui_array_rm_at(da, idx) \
    do {                                                     \
        for (size_t __i_ = (idx); __i_ < (da).len-1; ++__i_) \
            (da).data[__i_] = (da).data[__i_+1];             \
        (da).len--;                                          \
    } while (0)

#define tui_array_insert_at(da, idx, value) \
        do { \
                if ((idx) > (da).len) { \
                        fprintf(stderr, \
                                        "[array error]: insert index %zu is out of bounds (len = %zu)\n", \
                                        (size_t)(idx), (size_t)(da).len); \
                        exit(1); \
                } \
                if ((da).len >= (da).cap) { \
                        (da).cap = (da).cap ? (da).cap * 2 : 2; \
                        (da).data = (typeof(*((da).data)) *) \
                        realloc((da).data, (da).cap * sizeof(*((da).data))); \
                        if ((da).data == NULL) { \
                                fprintf(stderr, "[array error]: realloc failed\n"); \
                                exit(1); \
                        } \
                } \
                for (size_t __i = (da).len; __i > (idx); --__i) { \
                        (da).data[__i] = (da).data[__i - 1]; \
                } \
                (da).data[idx] = (value); \
                (da).len++; \
        } while (0)

// Describes a fake `ownership move' for visualizing ownership.
#define TUI_MOVE(x) x
#define TUI_OWNS(x) x

#define TUI_NEIGHBOR_LEFT(item) (item).neighbors[0]
#define TUI_NEIGHBOR_RIGHT(item) (item).neighbors[1]
#define TUI_NEIGHBOR_UP(item) (item).neighbors[2]
#define TUI_NEIGHBOR_DOWN(item) (item).neighbors[3]

typedef enum {
        TUI_ITEM_KIND_CANVAS = 0,
        TUI_ITEM_KIND_TEXT,
        TUI_ITEM_KIND_BOX,
} tui_item_kind;

typedef struct tui_item {
        unsigned uuid;
        tui_item_kind kind;
        // left, right, up, down
        struct tui_item *neighbors[4];
} tui_item;

TUI_ARRAY_DEFINE(tui_item *, tui_itemp_ar);

typedef struct {
        tui_item base;
        size_t term_width;
        size_t term_height;
        tui_itemp_ar children;
} tui_canvas;

typedef struct {
        tui_item base;
        char *chars;
} tui_text;

typedef struct {
        tui_item base;
} tui_box;

tui_canvas *tui_canvas_alloc(size_t term_width, size_t term_height);
tui_text *tui_text_alloc(TUI_MOVE(char *chars));
tui_box *tui_box_alloc(void);

void tui_item_free(tui_item *item);

void tui_item_bind_left(tui_item *item, TUI_MOVE(tui_item *binding));
void tui_item_bind_right(tui_item *item, TUI_MOVE(tui_item *binding));
void tui_item_bind_up(tui_item *item, TUI_MOVE(tui_item *binding));
void tui_item_bind_down(tui_item *item, TUI_MOVE(tui_item *binding));

void tui_canvas_add_child(tui_canvas *canvas, TUI_MOVE(tui_item *child));

void tui_draw(tui_canvas *canvas);

#ifdef TULIP_IMPLEMENTATION
#endif

#if 1
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

static uint8_t *
__tui_alloc(size_t b)
{
        uint8_t *p;

        if (!(p = (uint8_t *)malloc(b))) {
                fprintf(stderr, "could not allocate %zu bytes\n", b);
                exit(1);
        }

        return p;
}

static void
__tui_canvas_free(tui_canvas *canvas)
{
        for (size_t i = 0; i < canvas->children.len; ++i)
                tui_item_free(canvas->children.data[i]);
        tui_array_free(canvas->children);
        free(canvas);
        canvas = NULL;
}

static void
__tui_text_free(tui_text *text)
{
        free(text->chars);
        free(text);
        text = NULL;
}

static void
__tui_box_free(tui_box *box)
{
        free(box);
        box = NULL;
}

static tui_item
__tui_base_create(tui_item_kind k)
{
        static unsigned uuid = 0;
        tui_item item;

        item.kind = k;
        item.uuid = uuid++;

        TUI_NEIGHBOR_LEFT(item) = NULL;
        TUI_NEIGHBOR_RIGHT(item) = NULL;
        TUI_NEIGHBOR_UP(item) = NULL;
        TUI_NEIGHBOR_DOWN(item) = NULL;

        return item;
}

tui_canvas *
tui_canvas_alloc(size_t term_width, size_t term_height)
{
        tui_canvas *c;

        c = (tui_canvas *)__tui_alloc(sizeof(*c));
        c->term_width = term_width;
        c->term_height = term_height;
        c->children = tui_array_empty();
        c->base = __tui_base_create(TUI_ITEM_KIND_CANVAS);

        return c;
}

tui_text *
tui_text_alloc(TUI_MOVE(char *chars))
{
        tui_text *t;

        t = (tui_text *)__tui_alloc(sizeof(*t));
        t->chars = chars;
        t->base = __tui_base_create(TUI_ITEM_KIND_TEXT);

        return t;
}

tui_box *
tui_box_alloc(void)
{
        tui_box *b;

        b = (tui_box *)__tui_alloc(sizeof(*b));
        b->base = __tui_base_create(TUI_ITEM_KIND_BOX);

        return b;
}

void
tui_item_free(tui_item *item)
{
        switch (item->kind) {
        case TUI_ITEM_KIND_CANVAS:
                __tui_canvas_free((tui_canvas *)item);
        case TUI_ITEM_KIND_TEXT:
                __tui_text_free((tui_text *)item);
        case TUI_ITEM_KIND_BOX:
                __tui_box_free((tui_box *)item);
        default:
                break;
        }

        tui_item_free(TUI_NEIGHBOR_LEFT(*item));
        tui_item_free(TUI_NEIGHBOR_RIGHT(*item));
        tui_item_free(TUI_NEIGHBOR_UP(*item));
        tui_item_free(TUI_NEIGHBOR_DOWN(*item));

        item = NULL;
}

void
tui_canvas_add_child(tui_canvas        *canvas,
                     TUI_MOVE(tui_item *child))
{
        tui_array_append(canvas->children, child);
}

void
tui_item_bind_left(tui_item          *item,
                   TUI_MOVE(tui_item *binding))
{
        tui_item *it;

        it = TUI_NEIGHBOR_LEFT(*item);

        while (it)
                it = TUI_NEIGHBOR_LEFT(*it);

        it = binding;
}

void
tui_item_bind_right(tui_item          *item,
                    TUI_MOVE(tui_item *binding))
{
        tui_item *it;

        it = TUI_NEIGHBOR_RIGHT(*item);

        while (it)
                it = TUI_NEIGHBOR_RIGHT(*it);

        it = binding;
}

void
tui_item_bind_up(tui_item          *item,
                 TUI_MOVE(tui_item *binding))
{
        tui_item *it;

        it = TUI_NEIGHBOR_UP(*item);

        while (it)
                it = TUI_NEIGHBOR_UP(*it);

        it = binding;
}

void
tui_item_bind_down(tui_item          *item,
                   TUI_MOVE(tui_item *binding))
{
        tui_item *it;

        it = TUI_NEIGHBOR_DOWN(*item);

        while (it)
                it = TUI_NEIGHBOR_DOWN(*it);

        it = binding;
}

static void
__tui_draw_item(tui_item *item)
{

}

void
tui_draw(tui_canvas *canvas)
{
        for (size_t i = 0; i < canvas->children.len; ++i) {
                __tui_draw_item(canvas->children.data[i]);
        }
}

#endif // TULIP_IMPLEMENTATION

#endif // TULIP_H_INCLUDED
