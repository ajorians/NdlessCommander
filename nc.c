#include <os.h>
#include <SDL/SDL.h>
#include "nc.h"

SDL_Surface *screen;
nSDL_Font *font_b, *font_w;
SDL_bool done = SDL_FALSE;
char filenames[128][BUF_SIZE];
int num_files;
int file_choice = 0;
int file_scroll = 0;

void init(void) {
	if (SDL_Init(SDL_INIT_VIDEO) == -1) {
		printf("Couldn't initialize SDL: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}
	screen = SDL_SetVideoMode(320, 240, is_cx ? 16 : 8, SDL_SWSURFACE);
	if (screen == NULL) {
		printf("Couldn't set video mode: %s\n", SDL_GetError());
		SDL_Quit();
		exit(EXIT_FAILURE);
	}
	font_b = nSDL_LoadFont(NSDL_FONT_TINYTYPE, SDL_MapRGB(screen->format, 0, 0, 0), NSDL_FONTCFG_DEFAULT);
	if (font_b == NULL) {
		printf("Couldn't load font\n");
		SDL_Quit();
		exit(EXIT_FAILURE);
	}
	font_w = nSDL_LoadFont(NSDL_FONT_TINYTYPE, SDL_MapRGB(screen->format, 255, 255, 255), NSDL_FONTCFG_DEFAULT);
	if (font_w == NULL) {
		printf("Couldn't load font\n");
		nSDL_FreeFont(font_b);
		SDL_Quit();
		exit(EXIT_FAILURE);
	}
	SDL_EnableKeyRepeat(500, 200);
	SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 255, 255, 255));
}

void quit(void) {
	nSDL_FreeFont(font_b);
	nSDL_FreeFont(font_w);
	SDL_Quit();
}

void draw_panel(void) {
	char current_dir[BUF_SIZE];
	SDL_Rect rect;
	rect.x = rect.y = 0;
	rect.w = 320;
	rect.h = 24;
	DRAW_RECT();
	rect.y = 216;
	DRAW_RECT();
	rect.x = rect.y = 0;
	rect.w = 1;
	rect.h = 240;
	DRAW_RECT();
	rect.x = 160;
	DRAW_RECT();
	rect.x = 319;
	DRAW_RECT();
	rect.x = 0;
	rect.y = 239;
	rect.w = 320;
	rect.h = 1;
	DRAW_RECT();
	rect.x = 160;
	rect.y = 72;
	rect.w = 160;
	DRAW_RECT();
	nSDL_DrawString(screen, font_w, 8, 8, "Ndless Commander " VERSION);
	DRAW_STRING_BOLD(font_b, 24, 40, "Name");
	NU_Current_Dir("A:", current_dir);
	nSDL_DrawString(screen, font_w, 8, 224, current_dir);
}

void draw_filenames(void) {
	SDL_Rect rect;
	int i;
	if(num_files <= NUM_ITEMS_SHOWN)
		file_scroll = 0;
	rect.x = 24;
	rect.y = 56;
	rect.w = 136;
	rect.h = 8;
	get_filenames();
	for (i = file_scroll; i < num_files && i < file_scroll + NUM_ITEMS_SHOWN; ++i) {
		nSDL_DrawStringInRect(screen, font_b, &rect, filenames[i]);
		if (strcmp(filenames[i], ".") != 0
		 && strcmp(filenames[i], "..") != 0
		 && is_directory(filenames[i]))
			nSDL_DrawString(screen, font_b, 16, rect.y, "\\");
		rect.y += 8;
	}
	nSDL_DrawString(screen, font_b, 8, 56 + (8 * (file_choice - file_scroll)), "\x10");
	if (file_scroll > 0)
		nSDL_DrawString(screen, font_b, 8, 48, "\x1e");
	if (file_scroll + NUM_ITEMS_SHOWN < num_files)
		nSDL_DrawString(screen, font_b, 8, 200, "\x1f");
}

void draw_file_info(void) {
	struct stat file_stat;
	DRAW_STRING_BOLD(font_b, 176, 40, "File:");
	nSDL_DrawString(screen, font_b, 211, 40, filenames[file_choice]);
	DRAW_STRING_BOLD(font_b, 176, 48, "Size:");
	if (strcmp(filenames[file_choice], ".") != 0
	 && strcmp(filenames[file_choice], "..") != 0
	 && !is_directory(filenames[file_choice])) {
	 	FILE *fp;
		char buffer[BUF_SIZE] = {'\0'};
		stat(filenames[file_choice], &file_stat);
		get_readable_size(file_stat.st_size, buffer);
		nSDL_DrawString(screen, font_b, 211, 48, "%s", buffer);
		fp = fopen(filenames[file_choice], "r");
		if (fp) {
			char file_content[BUF_SIZE] = {'\0'};
			SDL_Rect rect = {164, 76, 152, 137};
			int c, i = 0;
			do {
				c = fgetc(fp);
				file_content[i] = c;
				++i;
			} while (c != EOF && i + 1 < BUF_SIZE);
			nSDL_DrawStringInRect(screen, font_b, &rect, file_content);
			fclose(fp);
		}
	} else {
		nSDL_DrawString(screen, font_b, 168, 140, "No preview available \x2");
		nSDL_DrawString(screen, font_b, 211, 48, "N/A");
	}
}

char *get_readable_size(int size, char *buffer) {
    const char units[4][3] = {"B\0", "kB\0", "MB\0", "GB\0"};
    int i = 0;
    while (size >= 1024) {
        size /= 1024;
        ++i;
    }
    sprintf(buffer, "%d%s", size + 1, units[i]);
    return buffer;
}

int copy_file(const char *src, const char *dst) {
	FILE *in = fopen(src, "rb"), *out;
	int c;
	if (!in)
		return -1;
	out = fopen(dst, "wb");
	if (!out) {
		fclose(in);
		return -1;
	}
	while ((c = fgetc(in)) != EOF)
		fputc(c, out);
	fclose(in);
	fclose(out);
	return 0;
}

int is_directory(const char *path) {
	struct stat file_stat;
	stat(path, &file_stat);
	return file_stat.st_mode & S_IFDIR;
}

void get_filenames(void) {
	struct dstat file_info;
	int i = num_files = 0;
	NU_Get_First(&file_info, "*.*");
	do {
		strncpy(filenames[i++], file_info.filepath, BUF_SIZE);
		++num_files;
	} while (NU_Get_Next(&file_info) == 0);
	NU_Done(&file_info);
}

void handle_return_key(void) {
	if (is_directory(filenames[file_choice])) {
		NU_Set_Current_Dir(filenames[file_choice]);
		get_filenames();
		file_choice = file_scroll = 0;
	}
}

void handle_tab_key(void) {
	char buffer[BUF_SIZE] = {'\0'};
	nSDL_DrawString(screen, font_w, 312 - nSDL_GetStringWidth(font_w, "Copying..."), 224, "Copying...");
	SDL_Flip(screen);
	if (strcmp(filenames[file_choice], ".") != 0
	 && strcmp(filenames[file_choice], "..") != 0
	 && !is_directory(filenames[file_choice])) {
	 	SDL_Rect msg_box;
	 	int width;
	 	int wait = 1;
		sprintf(buffer, "\\documents\\%s.tns", filenames[file_choice]);
		if (copy_file(filenames[file_choice], buffer) == -1)
			sprintf(buffer, "Error copying %s", filenames[file_choice]);
		else
			sprintf(buffer, "%s copied to My Documents", filenames[file_choice]);
		width = nSDL_GetStringWidth(font_w, buffer);
		msg_box.x = 139 - (width / 2);
		msg_box.y = 101;
		msg_box.w = width + 42;
		msg_box.h = 30;
		SDL_FillRect(screen, &msg_box, SDL_MapRGB(screen->format, 128, 0, 0));
		++msg_box.x;
		++msg_box.y;
		msg_box.w -= 2;
		msg_box.h -= 2;
		SDL_FillRect(screen, &msg_box, SDL_MapRGB(screen->format, 255, 255, 255));
		nSDL_DrawString(screen, font_b, 160 - (width / 2), msg_box.y + 10, buffer);
		SDL_Flip(screen);
		while (wait) {
			SDL_Event event;
			SDL_WaitEvent(&event);
			if (event.type == SDL_KEYDOWN)
				wait = 0;
		}
	}
}

void handle_keydown(SDLKey key) {
	switch (key) {
		case SDLK_UP:
			--file_choice;
			if (file_choice < 0) {
				file_choice = num_files - 1;
				file_scroll = num_files - NUM_ITEMS_SHOWN;
			} else if (file_choice - file_scroll < 0)
				--file_scroll;
			break;
		case SDLK_DOWN:
			++file_choice;
			if (file_choice >= num_files)
				file_choice = file_scroll = 0;
			else if (file_choice - file_scroll >= NUM_ITEMS_SHOWN)
				++file_scroll;
			break;
		case SDLK_RETURN:
			handle_return_key();
			break;
		case SDLK_TAB:
			handle_tab_key();
			break;
		case SDLK_ESCAPE:
			done = SDL_TRUE;
			break;
		default:
			break;
	}
}

int main(void) {
	init();
	while (!done) {
		SDL_Event event;
		SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 255, 255, 255));
		draw_panel();
		draw_filenames();
		draw_file_info();
		SDL_Flip(screen);
		SDL_WaitEvent(&event);
		if (event.type == SDL_KEYDOWN)
			handle_keydown(event.key.keysym.sym);
	}
	quit();
	return EXIT_SUCCESS;
}