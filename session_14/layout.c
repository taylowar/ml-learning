#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <raylib.h>

typedef struct {
    float x;
    float y;
    float w;
    float h;
} V_Layout_Rect;

V_Layout_Rect nf_v_make_layout_rect(float x, float y, float w, float h) 
{
    V_Layout_Rect r = {0};
    r.x = x;
    r.y = y;
    r.w = w;
    r.h = h;
    return r;
}

typedef enum {
    V_LO_HORZ,
    V_LO_VERT,
} V_Layout_Orient;

typedef struct {
    V_Layout_Rect rect;
    V_Layout_Orient orient;
    size_t i;
    size_t count;
    float gap;
} V_Layout;

V_Layout nf_v_make_layout(V_Layout_Orient orient, V_Layout_Rect rect, size_t count, float gap) {
    V_Layout l = {0};
    l.orient = orient;
    l.rect = rect;
    l.count = count;
    l.gap = gap;
    return l;
}

void nf_v_widget(V_Layout_Rect r, Color clr)
{
    Rectangle rr = {
        ceilf(r.x), ceilf(r.y), ceilf(r.w), ceilf(r.h)
    };
    
    Vector2 mousePos = GetMousePosition();

    if (CheckCollisionPointRec(mousePos, rr)) {
        clr = ColorBrightness(clr, 0.5f);
    }

    DrawRectangleRec(rr, clr);
}

V_Layout_Rect nf_v_layout_slot(V_Layout *l, const char *file_path, int line)
{
    if (l->i >= l->count) {
        fprintf(stderr, "%s:%d: ERROR: Layout Stack Overflow\n", file_path, line);
        exit(1);
    }

    V_Layout_Rect r = {0};

    switch (l->orient) {
        case V_LO_HORZ:
            r.w = l->rect.w/l->count;
            r.h = l->rect.h;
            r.x = l->rect.x + l->i*r.w;
            r.y = l->rect.y;

            if (l->i == 0) { // first
                r.w -= l->gap/2;
            } else if (l->i >= l->count - 1) { // last
                r.x += l->gap/2;
                r.w -= l->gap/2;
            } else { // middle
                r.x += l->gap/2;
                r.w -= l->gap;
            }
            break;
        case V_LO_VERT:
            r.w = l->rect.w;
            r.h = l->rect.h/l->count;
            r.x = l->rect.x;
            r.y = l->rect.y + l->i*r.h;

            if (l->i == 0) { // first
                r.h -= l->gap/2;
            } else if (l->i >= l->count - 1) { // last
                r.y += l->gap/2;
                r.h -= l->gap/2;
            } else { // middle
                r.y += l->gap/2;
                r.h -= l->gap;
            }
            break;
        default:
            assert(0 && "Unreachable");
    }
    l->i += 1;
    return r;
}

#define DA_INIT_CAP 256
#define da_append(da, item)                                                                 \
    do {                                                                                    \
        if ((da)->count >= (da)->capacity) {                                                \
            (da)->capacity = (da)->capacity == 0 ? DA_INIT_CAP : (da)->capacity*2;          \
            (da)->items = realloc((da)->items, (da)->capacity*sizeof(*(da)->items));        \
            assert((da)->items != NULL && "Buy more RAM");                                  \
        }                                                                                   \
        (da)->items[(da)->count++] = (item);                                                \
    } while (0)                                                                           

typedef struct {
    V_Layout *items;
    size_t count;
    size_t capacity;
} V_Layout_Stack;

void nf_v_layout_stack_push(V_Layout_Stack *ls, V_Layout_Orient orient, V_Layout_Rect rect, size_t count, float gap)
{
    V_Layout l = nf_v_make_layout(orient, rect, count, gap);
    da_append(ls, l);
}

void nf_v_layout_stack_pop(V_Layout_Stack *ls)
{
    assert(ls->count > 0);
    ls->count -= 1;
}

V_Layout_Rect nf_v_layout_stack_slot(V_Layout_Stack *ls, const char *file_path, int line)
{
    assert(ls->count > 0);
    return nf_v_layout_slot(&ls->items[ls->count-1], file_path, line);
}

#define nf_v_layout_stack_slot(ls) nf_v_layout_stack_slot(ls, __FILE__, __LINE__)

int main(void)
{
    size_t factor = 120;
    size_t width = 16*factor;
    size_t height = 9*factor;

    InitWindow(width, height, "Layout");
    SetTargetFPS(60);

    V_Layout_Stack ls = {0};

    while(!WindowShouldClose()) {
        float w = GetRenderWidth();
        float h = GetRenderHeight();
        float frame = h*0.15;
        float gap = 20;
        V_Layout_Rect root_rect = nf_v_make_layout_rect(0, frame, w, h - 2*frame);

        BeginDrawing();
            ClearBackground(BLACK);
            nf_v_layout_stack_push(&ls, V_LO_HORZ, root_rect, 3, gap);
                nf_v_widget(nf_v_layout_stack_slot(&ls), BLUE);
                nf_v_widget(nf_v_layout_stack_slot(&ls), RED);
                nf_v_layout_stack_push(&ls, V_LO_VERT, nf_v_layout_stack_slot(&ls), 3, gap);
                    nf_v_layout_stack_push(&ls, V_LO_HORZ, nf_v_layout_stack_slot(&ls), 2, gap);
                        nf_v_widget(nf_v_layout_stack_slot(&ls), GREEN);
                        nf_v_widget(nf_v_layout_stack_slot(&ls), GREEN);
                    nf_v_layout_stack_pop(&ls);
                    nf_v_widget(nf_v_layout_stack_slot(&ls), YELLOW);
                    nf_v_layout_stack_push(&ls, V_LO_HORZ, nf_v_layout_stack_slot(&ls), 3, gap);
                        nf_v_widget(nf_v_layout_stack_slot(&ls), MAGENTA);
                        nf_v_widget(nf_v_layout_stack_slot(&ls), WHITE);
                        nf_v_widget(nf_v_layout_stack_slot(&ls), MAGENTA);
                    nf_v_layout_stack_pop(&ls);
                nf_v_layout_stack_pop(&ls);
            nf_v_layout_stack_pop(&ls);
        EndDrawing();
        assert(ls.count == 0);
    }

    CloseWindow();

    return 0;
}
