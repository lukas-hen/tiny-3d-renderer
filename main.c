#include <stdlib.h>
#include <stdint.h>
#include <SDL2/SDL.h>
#include <unistd.h>
#include <math.h>

#define WINDOW_WIDTH 800
#define FPS 60
#define PI 3.1415926f

#define ALPHA(rgba) (rgba >> 0  & (uint8_t) 0xff)
#define BLUE(rgba)  (rgba >> 8  & (uint8_t) 0xff)
#define GREEN(rgba) (rgba >> 16 & (uint8_t) 0xff)
#define RED(rgba)   (rgba >> 24 & (uint8_t) 0xff)

typedef struct Vertex {
    float x;
    float y;
    float z;
} Vertex;

void clear(SDL_Renderer *renderer);
void draw_line(SDL_Renderer *renderer, Vertex *from, Vertex *to, uint32_t rgba);
void draw_pixel(SDL_Renderer *renderer, Vertex *v, uint32_t RGBA);
void draw_square(SDL_Renderer *renderer, Vertex *v, int size, uint32_t rgba);
void normal_grid_to_raster(Vertex *v);
void rotate_x(Vertex *v, float theta);
void rotate_y(Vertex *v, float theta);
void rotate_z(Vertex *v, float theta);

const Vertex Object[] = {    
    // Front face
    { .x =  0.25f, .y =  0.25f, .z = 0.25f },
    { .x =  0.25f, .y = -0.25f, .z = 0.25f },
    { .x = -0.25f, .y = -0.25f, .z = 0.25f },
    { .x = -0.25f, .y =  0.25f, .z = 0.25f },

    // Back face
    { .x =  0.25f, .y =  0.25f, .z = -0.25f },
    { .x =  0.25f, .y = -0.25f, .z = -0.25f },
    { .x = -0.25f, .y = -0.25f, .z = -0.25f },
    { .x = -0.25f, .y =  0.25f, .z = -0.25f },
};

int EdgeBuffer[][2] = {    
    { 0, 1 },
    { 1, 2 },
    { 2, 3 },
    { 3, 0 },
    { 4, 5 },
    { 5, 6 },
    { 6, 7 },
    { 7, 4 },
    { 0, 4 },
    { 1, 5 },
    { 2, 6 },
    { 3, 7 },
};

const uint32_t VertexColors[] = {    
    0x8478eaff,
    0xada5f1ff,
    0xedc967ff,
    0xdba830ff,
    0x00b8d4ff,
    0x34ebaeff,
    0xffffffff,
    0xed66d2ff,
};

int main(void) {

    SDL_Event event;
    SDL_Renderer *renderer;
    SDL_Window *window;
    float dt = 0;

    size_t n_vertices = sizeof(Object)/sizeof(Vertex);
    size_t n_edges = sizeof(EdgeBuffer)/sizeof(EdgeBuffer[0]);
    Vertex *vb = malloc(n_vertices);

    SDL_Init(SDL_INIT_VIDEO);
    SDL_CreateWindowAndRenderer(WINDOW_WIDTH, WINDOW_WIDTH, 0, &window, &renderer);

    while (1) {

        if (
            SDL_PollEvent(&event) 
            && (event.type == SDL_QUIT 
            || (event.type == SDL_KEYUP && event.key.keysym.scancode == SDL_SCANCODE_Q))
        ) break;

        clear(renderer);

        // Copy object to VBuffer before projections/rotation.
        memcpy(vb, &Object, sizeof(Object));

        for(size_t i = 0; i < n_vertices; ++i) {     

            rotate_x(&vb[i], dt);
            rotate_y(&vb[i], dt);
            rotate_z(&vb[i], dt);

            normal_grid_to_raster(&vb[i]);
            
            draw_square(renderer, &vb[i], 15, VertexColors[i]);
        }

        for(size_t j = 0; j < n_edges; ++j) {
            int v_1_idx = EdgeBuffer[j][0];
            int v_2_idx = EdgeBuffer[j][1];
            draw_line(renderer, &vb[v_1_idx], &vb[v_2_idx], VertexColors[j % (sizeof(VertexColors)/sizeof(uint32_t))]);
        }

        SDL_RenderPresent(renderer);
        
        dt += PI/(FPS*8); // One spin per 8s.

        sleep(1/FPS);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return EXIT_SUCCESS;
}


void clear(SDL_Renderer *renderer) {    
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);
} 

void draw_pixel(SDL_Renderer *renderer, Vertex *v, uint32_t rgba) {    
    SDL_SetRenderDrawColor(renderer, RED(rgba), GREEN(rgba), BLUE(rgba), ALPHA(rgba));
    SDL_RenderDrawPoint(renderer, v->x, v->y);
}

void draw_square(SDL_Renderer *renderer, Vertex *v, int size, uint32_t rgba) {  
    SDL_SetRenderDrawColor(renderer, RED(rgba), GREEN(rgba), BLUE(rgba), ALPHA(rgba));
    SDL_Rect r = {.x = (v->x - (float) size/2), .y = v->y - (float) size/2, .w = size, .h = size};
    SDL_RenderFillRect(renderer, &r);   
}

void draw_line(SDL_Renderer *renderer, Vertex *from, Vertex *to, uint32_t rgba) {
    SDL_SetRenderDrawColor(renderer, RED(rgba), GREEN(rgba), BLUE(rgba), ALPHA(rgba));
    SDL_RenderDrawLine(renderer, from->x, from->y, to->x, to->y);
}

void normal_grid_to_raster(Vertex *v) {
    
    float x = v->x;
    float y = -1.0f*v->y;
    float z = v->z;
 
    v->x = (x + 1.0f) * ((float)WINDOW_WIDTH/2); 
    v->y = (y + 1.0f) * ((float)WINDOW_WIDTH/2);
    v->z = z;
}

void rotate_y(Vertex *v, float theta) {
    float x = v->x;
    float y = v->y;
    float z = v->z;

    v->x = cosf(theta)*x + sinf(theta)*z;
    v->z = -1*sinf(theta)*x + cosf(theta)*z;
}

void rotate_z(Vertex *v, float theta) {
    float x = v->x;
    float y = v->y;
    float z = v->z;

    v->x = cosf(theta)*x - sinf(theta)*y;
    v->y = sinf(theta)*x + cosf(theta)*y;
}

void rotate_x(Vertex *v, float theta) {
    float x = v->x;
    float y = v->y;
    float z = v->z;

    v->y = cosf(theta)*y - sinf(theta)*z;
    v->z = sinf(theta)*y + cosf(theta)*z;
}