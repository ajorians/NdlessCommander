#ifndef NC_H
#define NC_H

#include <SDL/SDL.h>

#define VERSION "0.4"

#define NUM_ITEMS_SHOWN 18

#define BUF_SIZE 512
#define BIG_BUF_SIZE 512

#define DRAW_RECT() SDL_FillRect(screen, &rect, SDL_MapRGB(screen->format, 64, 0, 0))
#define DRAW_STRING_BOLD(font, x, y, s) do { \
	nSDL_DrawString(screen, font, x, y, s); \
	nSDL_DrawString(screen, font, x + 1, y, s); \
} while (0)

void init(void);
void quit(void);
void draw_panel(void);
void draw_filenames(void);
void draw_file_info(void);
char *get_readable_size(int size, char *buffer);
int is_directory(const char *path);
void get_filenames(void);
void handle_return_key(void);
void handle_keydown(SDLKey key);

#endif