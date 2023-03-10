#include <stdio.h>
#include <SDL2/SDL.h>

int WIDTH  = 768;
int HEIGHT = 512;

const double move = 0.05;
const double zoom = 0.9;

typedef std::pair<double, double> complex_T;

complex_T f(complex_T z, complex_T seed) {
        return {
                z.first * z.first - z.second * z.second + seed.first,
                2 * z.first * z.second + seed.second
        };
}

double randunif() {
        return (double) rand() / RAND_MAX;
}

double escape(
        complex_T z,
        complex_T seed,
        complex_T (*f)(complex_T, complex_T),
        int maxiter,
        double radius
) {
        for (int i = 0; i < maxiter; i++) {
                auto [x, y] = f(z, seed);
                if (x * x + y * y > radius * radius)
                        return i + 1 - std::log(std::log(std::sqrt(x*x + y*y))) / std::log(2.0);
                z.first = x;
                z.second = y;
        }
        return maxiter;
}

double stability(
        complex_T z,
        complex_T seed,
        complex_T (*f)(complex_T, complex_T),
        int maxiter,
        double radius = 4
) {
        return escape(z, seed, f, maxiter, radius) / maxiter;
}

void render(
        uint32_t *buffer,
        complex_T centre,
        double dx,
        complex_T seed,
        complex_T (*f)(complex_T, complex_T),
        int maxiter,
        int samples
) {
        double x_min = centre.first - dx / 2;
        double x_max = centre.first + dx / 2;
        double y_min = centre.second - dx * HEIGHT / (WIDTH * 2);
        double y_max = centre.second + dx * HEIGHT / (WIDTH * 2);

        int r, g, b;
        double x, y;
        double s;

        for (int i = 0; i < WIDTH; i++) {
                for (int j = 0; j < HEIGHT; j++) {
                        s = 0;
                        for (int sample = 0; sample < samples; sample ++) {
                                x = x_min + (x_max - x_min) * (i + randunif()) / WIDTH;
                                y = y_min + (y_max - y_min) * (j + randunif()) / HEIGHT;
                                s += stability({x, y}, seed, f, maxiter);
                        }
                        s /= samples;
                        r = (int) (255 * (1 - s));
                        g = r;
                        b = r;
                        buffer[j * WIDTH + i] = 0xFF000000 | (r << 16) | (b << 8) | g;
                }
        }
}


int main(int argc, const char *argv[]) {

        complex_T seed = {-0.8, 0.156};
        complex_T centre = {0, 0};
        double dx = 4;
        int maxiter = 32;
        int samples = 4;

        if (argc >= 2) {
                WIDTH = atoi(argv[1]);
                HEIGHT = WIDTH;
        }
        if (argc >= 3) {
                HEIGHT = atoi(argv[2]);
        }


        SDL_Window *window = SDL_CreateWindow(
                "Julia",
                SDL_WINDOWPOS_UNDEFINED,
                SDL_WINDOWPOS_UNDEFINED,
                WIDTH,
                HEIGHT,
                SDL_WINDOW_SHOWN
        );

        SDL_Renderer *renderer = SDL_CreateRenderer(
                window,
                -1,
                SDL_RENDERER_ACCELERATED
        );

        SDL_Texture *texture = SDL_CreateTexture(
                renderer,
                SDL_PIXELFORMAT_ARGB8888,
                SDL_TEXTUREACCESS_STREAMING,
                WIDTH,
                HEIGHT
        );

        SDL_Rect texture_rect = {0, 0, WIDTH, HEIGHT};

        uint32_t *buffer = new uint32_t[WIDTH * HEIGHT];

        render(buffer, centre, dx, seed, f, maxiter, samples);
        SDL_UpdateTexture(texture, NULL, buffer, WIDTH * sizeof(uint32_t));
        SDL_RenderCopy(renderer, texture, NULL, &texture_rect);
        SDL_RenderPresent(renderer);

        SDL_Event event;
        bool quit = false;

        while(!quit && SDL_WaitEvent(&event)) {
                if(event.type == SDL_QUIT) {
                        quit = true;
                } else if (event.type == SDL_MOUSEBUTTONDOWN) {
                        int i, j;
                        SDL_GetMouseState(&i, &j);
                        double x_min = centre.first - dx / 2;
                        double y_min = centre.second - dx * HEIGHT / (WIDTH * 2);
                        double x = x_min + dx * i / WIDTH;
                        double y = y_min + dx * j / WIDTH;

                        switch(event.button.button) {
                                case SDL_BUTTON_RIGHT:
                                        seed = {x, y};
                                        break;
                                case SDL_BUTTON_LEFT:
                                        centre = {x, y};
                                        break;
                        }

                        printf("Seed %.6f + %.6fi\n", seed.first, seed.second);
                        render(buffer, centre, dx, seed, f, maxiter, samples);
                        SDL_UpdateTexture(texture, NULL, buffer, WIDTH * sizeof(uint32_t));
                        SDL_RenderCopy(renderer, texture, NULL, &texture_rect);
                        SDL_RenderPresent(renderer);
                } else if (event.type == SDL_KEYDOWN) {
                        switch(event.key.keysym.scancode) {
                                case SDL_SCANCODE_Q:
                                        quit = true;
                                        break;
                                case SDL_SCANCODE_0:
                                        centre = {0, 0};
                                        dx = 4;
                                        break;
                                case SDL_SCANCODE_MINUS:
                                        dx /= zoom;
                                        break;
                                case SDL_SCANCODE_EQUALS:
                                        dx *= zoom;
                                        break;
                                case SDL_SCANCODE_LEFT:
                                        centre.first -= dx * move;
                                        break;
                                case SDL_SCANCODE_RIGHT:
                                        centre.first += dx * move;
                                        break;
                                case SDL_SCANCODE_UP:
                                        centre.second -= dx * move;
                                        break;
                                case SDL_SCANCODE_DOWN:
                                        centre.second += dx * move;
                                        break;
                                        break;
                                case SDL_SCANCODE_COMMA:
                                        maxiter /= 2;
                                        break;
                                case SDL_SCANCODE_PERIOD:
                                        maxiter *= 2;
                                        break;
                                case SDL_SCANCODE_LEFTBRACKET:
                                        samples /= 2;
                                        break;
                                case SDL_SCANCODE_RIGHTBRACKET:
                                        samples *= 2;
                                        break;
                        }
                        maxiter = (maxiter == 0)? 1 : maxiter;
                        samples = (samples == 0)? 1 : samples;
                        printf("Maxiter %d, Samples %d\n", maxiter, samples);
                        render(buffer, centre, dx, seed, f, maxiter, samples);
                        SDL_UpdateTexture(texture, NULL, buffer, WIDTH * sizeof(uint32_t));
                        SDL_RenderCopy(renderer, texture, NULL, &texture_rect);
                        SDL_RenderPresent(renderer);
                }
        }

        SDL_DestroyTexture(texture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);

        SDL_Quit();

        return 0;
}
