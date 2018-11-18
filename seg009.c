#include "common.h"
#include <fcntl.h>
#include <stdlib.h>
#include <time.h>

// Most functions in this file are different from those in the original game.

void sdlperror(const char* header) {
	const char* error = SDL_GetError();
	printf("%s: %s\n",header,error);
	//quit(1);
}

dat_type* dat_chain_ptr = NULL;

int last_key;

// seg009:000D
int __pascal far read_key() {
	// stub
	int key = last_key;
	last_key = 0;
	return key;
}

// seg009:019A
void __pascal far clear_kbd_buf() {
	// stub
	last_key = 0;
}

// seg009:040A
word __pascal far prandom(word max) {
	if (!seed_was_init) {
		// init from current time
		random_seed = time(NULL);
		seed_was_init = 1;
	}
	random_seed = random_seed * 214013 + 2531011;
	return (random_seed >> 16) % (max + 1);
}

// seg009:0467
int __pascal far round_xpos_to_byte(int xpos,int round_direction) {
	// stub
	return xpos;
}

// seg009:0C7A
void __pascal far quit(int exit_code) {
	restore_stuff();
	exit(exit_code);
}

// seg009:0C90
void __pascal far restore_stuff() {
	SDL_Quit();
}

// seg009:0E33
int __pascal far key_test_quit() {
	word key;
	key = read_key();
	if (key == 0x11) { // ctrl-q
		quit(0);
	}
	return key;
}

// seg009:0E54
const char* __pascal far check_param(const char *param) {
	// stub
	short arg_index;
	for (arg_index = 1; arg_index < g_argc; ++arg_index) {
		if (/*strnicmp*/strncasecmp(g_argv[arg_index], param, strlen(param)) == 0) {
			return g_argv[arg_index];
		}
	}
	return 0;
}

// seg009:0EDF
int __pascal far pop_wait(int timer_index,int time) {
	//wait_time[timer_index] = time;
	start_timer(timer_index, time);
	return do_wait(timer_index);
}

// seg009:0F58
dat_type *__pascal open_dat(const char *filename,int drive) {
	FILE* fp = fopen(filename, "rb");
	dat_header_type dat_header;
	dat_table_type* dat_table = NULL;
	if (fp != NULL) {
		fread(&dat_header, 6, 1, fp);
		dat_table = (dat_table_type*) malloc(dat_header.table_size);
		fseek(fp, dat_header.table_offset, SEEK_SET);
		fread(dat_table, dat_header.table_size, 1, fp);
	}
	dat_type* pointer = (dat_type*) malloc(sizeof(dat_type));
	memset(pointer, 0, sizeof(dat_type));
	strncpy(pointer->filename, filename, sizeof(pointer->filename));
	pointer->next_dat = dat_chain_ptr;
	if (fp != NULL) {
		pointer->handle = fp;
		pointer->dat_table = dat_table;
	}
	dat_chain_ptr = pointer;
	// stub
	return pointer;
}

// seg009:101B
//int __pascal far file_exists(const char near *filename) ;

// seg009:A172
//int __pascal far load_from_opendats_to_area(int resource,void far *area,int length) {
//}

// seg009:9CAC
void __pascal far set_loaded_palette(dat_pal_type far *palette_ptr) {
	int si, di, current_row;
	for (si = di = current_row = 0; si < 16; ++si, di += 0x10) {
		if (palette_ptr->row_bits & (1 << si)) {
			set_pal_arr(di, 16, palette_ptr->vga + current_row*0x10, 1);
			++current_row;
		}
	}
}

// data:3356
word chtab_palette_bits = 1;

// seg009:104E
chtab_type* __pascal load_sprites_from_file(int resource,int palette_bits/*,byte* pack,int shift*/, int quit_on_error) {
#if 0
	chtab_type* chtab_ptr;
	dat_shpl_type area;
	word curr_image_index;
	dat_pal_type* pal_ptr;
	word count;
	void* xlat_buffer;
	word var_2;
	has_palette_bits = 1;
	load_from_opendats_to_area(resource, &area, 0);
	pal_ptr = &area.palette;
	if (graphics_mode == gmMcgaVga) {
		if (palette_bits == 0) {
			palette_bits = add_palette_bits(area1_ptr->n_colors);
			if (palette_bits == 0) {
				quit(1);
			}
		} else {
			chtab_palette_bits |= palette_bits;
			has_palette_bits = 0;
		}
		area1_ptr->row_bits = palette_bits;
	}
	count = area.n_images;
	if (graphics_mode != gmCga && graphics_mode != gmHgaHerc) {
		shift = 0;
	}
	count <<= shift;
	chtab_ptr = malloc_near(sizeof(chtab_type) + count * sizeof(image_type* far));
	xlat_buffer = malloc_near(0x200);
	process_palette(xlat_buffer, area1_ptr);
	if (graphics_mode == gmMcgaVga) {
		chtab_ptr->chtab_palette_bits = palette_bits;
		chtab_ptr->has_palette_bits = has_palette_bits;
	}
	chtab_ptr->n_images = count;
	for (curr_image_index = 0; curr_image_index < area.n_images; ++curr_image_index) {
		chtab_ptr->pointers[curr_image_index] = load_image(
			resource + curr_image_index + 1, xlat_buffer, shift,
			chtab_ptr->pointers[curr_image_index + area.n_images],
			(int)pack == -1 ? 1 : (int)pack == 0 ? 0 : pack[curr_image_index]
		);
	}
	set_loaded_palette(area1_ptr);
	free_near(xlat_buffer);
	return chtab_ptr;
#endif // 0

	int i;
	int n_images = 0;
	//int has_palette_bits = 1;
	chtab_type* chtab = NULL;
	dat_shpl_type* shpl = (dat_shpl_type*) load_from_opendats_alloc(resource, "pal", NULL, NULL);
	if (shpl == NULL) {
		printf("Can't load sprites from resource %d.\n", resource);
		if (quit_on_error) quit(1);
		return NULL;
	}
	
	dat_pal_type* pal_ptr = &shpl->palette;
	if (graphics_mode == gmMcgaVga) {
		if (palette_bits == 0) {
			/*
			palette_bits = add_palette_bits(pal_ptr->n_colors);
			if (palette_bits == 0) {
				quit(1);
			}
			*/
		} else {
			chtab_palette_bits |= palette_bits;
			//has_palette_bits = 0;
		}
		pal_ptr->row_bits = palette_bits;
	}
	
	n_images = shpl->n_images;
	size_t alloc_size = sizeof(chtab_type) + sizeof(void far *) * n_images;
	chtab = (chtab_type*) malloc(alloc_size);
	memset(chtab, 0, alloc_size);
	chtab->n_images = n_images;
	for (i = 1; i <= n_images; i++) {
		SDL_Surface* image = load_image(resource + i, pal_ptr);
//		if (image == NULL) printf(" failed");
		if (image != NULL) {
			
			if (SDL_SetAlpha(image, 0, 0) != 0) {
				sdlperror("SDL_SetAlpha");
				quit(1);
			}
			
			/*
			if (SDL_SetColorKey(image, SDL_SRCCOLORKEY, 0) != 0) {
				sdlperror("SDL_SetColorKey");
				quit(1);
			}
			*/
		}
//		printf("\n");
		chtab->pointers[i-1] = image;
	}
	set_loaded_palette(pal_ptr);
	return chtab;
}

// seg009:11A8
void __pascal far free_chtab(chtab_type *chtab_ptr) {
	image_type far* curr_image;
	word id;
	word n_images;
	if (graphics_mode == gmMcgaVga && chtab_ptr->has_palette_bits) {
		chtab_palette_bits &= ~ chtab_ptr->chtab_palette_bits;
	}
	n_images = chtab_ptr->n_images;
	for (id = 0; id < n_images; ++id) {
		curr_image = chtab_ptr->pointers[id];
		if (curr_image) {
			/*free_far*/SDL_FreeSurface(curr_image);
		}
	}
	free_near(chtab_ptr);
}

// seg009:8CE6
void __pascal far unrle_lr(byte far *destination,const byte far *source,int length) {
	const byte* si = source;
	byte* di = destination;
	short bp = length;
	while (bp) {
		sbyte al = *(si++);
		sbyte cl = al;
		if (cl >= 0) {
			++cl;
			do {
				*(di++) = *(si++);
				--bp;
			} while (--cl);
		} else {
			al = *(si++);
			cl = -cl;
			do {
				*(di++) = al;
				--bp;
			} while (--cl);
		}
	}
}

// seg009:8D1C
void __pascal far unrle_ud(byte far *destination,const byte far *source,int size,int width,int height) {
	short bx = height;
	const byte* si = source;
	byte* di = destination;
	short dx = size;
	--size;
	--width;
	while (dx) {
		sbyte al = *(si++);
		sbyte cl = al;
		if (cl >= 0) {
			++cl;
			do {
				*(di++) = *(si++);
				di += width;
				if (--bx == 0) {
					di -= size;
					bx = height;
				}
				--dx;
			} while (--cl);
		} else {
			al = *(si++);
			cl = -cl;
			do {
				*(di++) = al;
				di += width;
				if (--bx == 0) {
					di -= size;
					bx = height;
				}
				--dx;
			} while (--cl);
		}
	}
}

// seg009:90FA
byte far* __pascal far unlz_lr(byte far *dest,const byte far *source,int length) {
	byte* window = (byte*) malloc_near(0x400);
	if (window == NULL) return NULL;
	memset(window, 0, 0x400);
	byte* window_pos = window + 0x400 - 0x42; // bx
	short remaining = length; // cx
	byte* window_end = window + 0x400; // dx
	const byte* si = source;
	byte* di = dest;
	word mask = 0;
	do {
		mask >>= 1;
		if ((mask & 0xFF00) == 0) {
			mask = *(si++) | 0xFF00;
		}
		if (mask & 1) {
			*(window_pos++) = *(di++) = *(si++);
			if (window_pos >= window_end) window_pos = window;
			--remaining;
		} else {
			word ax = *(si++);
			ax = (ax << 8) | *(si++);
			byte* copy_source = window + (ax & 0x3FF);
			byte copy_length = (ax >> 10) + 3;
			do {
				*(window_pos++) = *(di++) = *(copy_source++);
				if (copy_source >= window_end) copy_source = window;
				if (window_pos >= window_end) window_pos = window;
			} while (--remaining && --copy_length);
		}
	} while (remaining);
//	end:
	free(window);
	return dest;
}

// seg009:91AD
byte far* __pascal far unlz_ud(byte far *dest,const byte far *source,int length,int stride,int height) {
	byte* window = (byte*) malloc_near(0x400);
	if (window == NULL) return NULL;
	memset(window, 0, 0x400);
	byte* window_pos = window + 0x400 - 0x42; // bx
	short remaining = height; // cx
	byte* window_end = window + 0x400; // dx
	const byte* si = source;
	byte* di = dest;
	word mask = 0;
	short var_6 = length - 1;
	do {
		mask >>= 1;
		if ((mask & 0xFF00) == 0) {
			mask = *(si++) | 0xFF00;
		}
		if (mask & 1) {
			*(window_pos++) = *di = *(si++);
			di += stride;
			if (--remaining == 0) {
				di -= var_6;
				remaining = height;
			}
			if (window_pos >= window_end) window_pos = window;
			--length;
		} else {
			word ax = *(si++);
			ax = (ax << 8) | *(si++);
			byte* copy_source = window + (ax & 0x3FF);
			byte copy_length = (ax >> 10) + 3;
			do {
				*(window_pos++) = *di = *(copy_source++);
				di += stride;
				if (--remaining == 0) {
						di -= var_6;
						remaining = height;
				}
				if (copy_source >= window_end) copy_source = window;
				if (window_pos >= window_end) window_pos = window;
			} while (--length && --copy_length);
		}
	} while (length);
//	end:
	free(window);
	return dest;
}

// seg009:938E
void __pascal far decompr_img(byte far *dest,const image_data_type far *source,int decomp_size,int cmeth, int stride) {
	switch (cmeth) {
		case 0: // RAW
			memcpy_far(dest, &source->data, decomp_size);
		break;
		case 1: // RLE left-to-right
			unrle_lr(dest, source->data, decomp_size);
		break;
		case 2: // RLE up-to-down
			unrle_ud(dest, source->data, decomp_size, stride, source->height);
		break;
		case 3: // LZ left-to-right
			unlz_lr(dest, source->data, decomp_size);
		break;
		case 4: // LZ up-to-down
			unlz_ud(dest, source->data, decomp_size, stride, source->height);
		break;
	}
}

int calc_stride(image_data_type* image_data) {
	int width = image_data->width;
	int flags = image_data->flags;
	int depth = ((flags >> 12) & 3) + 1;
	return (depth * width + 7) / 8;
}

byte* conv_to_8bpp(byte* in_data, int width, int height, int stride, int depth) {
	byte* out_data = (byte*) malloc(width * height);
	int y,x,bx,b;
	int px_per_byte = 8 / depth;
	int mask = (1 << depth) - 1;
	for (y = 0; y < height; ++y) {
		byte* in_p = in_data + y*stride;
		byte* out_p = out_data + y*width;
		for (x = bx = 0; bx < stride; ++bx) {
			byte v = *in_p;
			int sh = 8;
			for (b = 0; b < px_per_byte && x < width; ++b, ++x) {
				sh -= depth;
				*out_p = (v >> sh) & mask;
				++out_p;
			}
			++in_p;
		}
	}
	return out_data;
}

image_type* decode_image(image_data_type* image_data, dat_pal_type* palette) {
	int height = image_data->height;
	if (height == 0) return NULL;
	int width = image_data->width;
	int flags = image_data->flags;
	int depth = ((flags >> 12) & 3) + 1;
	int cmeth = (flags >> 8) & 0x0F;
	int stride = calc_stride(image_data);
	int dest_size = stride * height;
	byte* dest = (byte*) malloc(dest_size);
	memset(dest, 0, dest_size);
	decompr_img(dest, image_data, dest_size, cmeth, stride);
	byte* image_8bpp = conv_to_8bpp(dest, width, height, stride, depth);
	free(dest); dest = NULL;
	image_type* image = SDL_CreateRGBSurface(0, width, height, 8, 0, 0, 0, 0);
	if (image == NULL) {
		sdlperror("SDL_CreateRGBSurface");
		quit(1);
	}
	if (SDL_LockSurface(image) != 0) {
		sdlperror("SDL_LockSurface");
	}
	int y;
	for (y = 0; y < height; ++y) {
		// fill image with data
		memcpy((byte*)image->pixels + y*image->pitch, image_8bpp + y*width, width);
	}
	SDL_UnlockSurface(image);
	/*
	FILE* fp = fopen("decode_image.raw", "wb");
	fwrite(image_8bpp,width*height,1,fp);
	fclose(fp);
	if (SDL_SaveBMP(image, "decode_image.bmp") != 0) { // debug
		sdlperror("SDL_SaveBMP");
	}
//	*/
	free(image_8bpp); image_8bpp = NULL;
	SDL_Color colors[16];
	int i;
	for (i = 0; i < 16; ++i) {
		colors[i].r = palette->vga[i].r << 2;
		colors[i].g = palette->vga[i].g << 2;
		colors[i].b = palette->vga[i].b << 2;
	}
	SDL_SetColors(image, colors, 0, 16);
	return image;
}

// seg009:121A
image_type* far __pascal far load_image(int resource_id, dat_pal_type* palette /*void* xlat_tbl,int use_global_xlat,void* target,int pack*/) {
	// stub
	data_location result;
	int size;
	void* image_data = load_from_opendats_alloc(resource_id, "png", &result, &size);
	image_type* image = NULL;
	switch (result) {
		case data_none:
			return NULL;
		break;
		case data_DAT: { // DAT
			image = decode_image((image_data_type*) image_data, palette);
			//free(image_data);
		} break;
		case data_directory: { // directory
			SDL_RWops* rw = SDL_RWFromConstMem(image_data, size);
			if (rw == NULL) {
				sdlperror("SDL_RWFromConstMem");
				return NULL;
			}
			image = IMG_LoadPNG_RW(rw);
			if (SDL_RWclose(rw) != 0) {
				sdlperror("SDL_RWclose");
			}
			//free(image_data);
		} break;
	}
	if (image_data != NULL) free(image_data);
	if (image != NULL) {
		if (SDL_SetColorKey(image, SDL_SRCCOLORKEY, 0) != 0) {
			sdlperror("SDL_SetColorKey");
			quit(1);
		}
//		printf("bpp = %d\n", image->format->BitsPerPixel);
		if (SDL_SetAlpha(image, 0, 0) != 0) {
			sdlperror("SDL_SetAlpha");
			quit(1);
		}
	}
	return image;
}

// seg009:13C4
void __pascal far draw_image_transp(image_type far *image,image_type far *mask,int xpos,int ypos) {
	if (graphics_mode == gmMcgaVga) {
		draw_image_transp_vga(image, xpos, ypos);
	} else {
		// ...
	}
}

// seg009:157E
int __pascal far set_joy_mode() {
	// stub
	is_joyst_mode = 0;
	is_keyboard_mode = !is_joyst_mode;
	return 0;
}

// seg009:178B
surface_type far *__pascal make_offscreen_buffer(const rect_type far *rect) {
	// stub
#ifndef USE_ALPHA
	//return SDL_CreateRGBSurface(0, rect->right, rect->bottom, 24, 0xFF, 0xFF<<8, 0xFF<<16, 0);
	// Bit order matches onscreen buffer, good for fading.
	return SDL_CreateRGBSurface(0, rect->right, rect->bottom, 24, 0xFF<<16, 0xFF<<8, 0xFF<<0, 0);
#else
	return SDL_CreateRGBSurface(0, rect->right, rect->bottom, 32, 0xFF, 0xFF<<8, 0xFF<<16, 0xFF<<24);
#endif
	//return surface;
}

// seg009:17BD
void __pascal far free_surface(surface_type *surface) {
	SDL_FreeSurface(surface);
}

// seg009:17EA
void __pascal far free_peel(peel_type *peel_ptr) {
	//method_8_free(peel_ptr->peel);
	SDL_FreeSurface(peel_ptr->peel);
	//free_near(peel_ptr);
}

const rgb_type vga_palette[] = {
{0x00, 0x00, 0x00},
{0x00, 0x00, 0x2A},
{0x00, 0x2A, 0x00},
{0x00, 0x2A, 0x2A},
{0x2A, 0x00, 0x00},
{0x2A, 0x00, 0x2A},
{0x2A, 0x15, 0x00},
{0x2A, 0x2A, 0x2A},
{0x15, 0x15, 0x15},
{0x15, 0x15, 0x3F},
{0x15, 0x3F, 0x15},
{0x15, 0x3F, 0x3F},
{0x3F, 0x15, 0x15},
{0x3F, 0x15, 0x3F},
{0x3F, 0x3F, 0x15},
{0x3F, 0x3F, 0x3F},
};

// seg009:182F
void __pascal far set_hc_pal() {
	// stub
	if (graphics_mode == gmMcgaVga) {
		set_pal_arr(0, 16, vga_palette, 1);
	} else {
		// ...
	}
}

// seg009:2446
void __pascal far flip_not_ega(byte far *memory,int bottom,int stride) {
	byte* row_buffer = (byte*) malloc(stride);
	byte* top_ptr;
	byte* bottom_ptr;
	short cx = bottom;
	bottom_ptr = top_ptr = memory;
	bottom_ptr += (bottom - 1) * stride;
	cx = bottom >> 1;
	do {
		memcpy(row_buffer, top_ptr, stride);
		memcpy(top_ptr, bottom_ptr, stride);
		memcpy(bottom_ptr, row_buffer, stride);
		top_ptr += stride;
		bottom_ptr -= stride;
		--cx;
	} while (cx);
	free(row_buffer);
}

// seg009:19B1
void __pascal far flip_screen(surface_type far *surface) {
	// stub
	if (graphics_mode != gmEga) {
		if (SDL_LockSurface(surface) != 0) {
			sdlperror("SDL_LockSurface");
			quit(1);
		}
		flip_not_ega((byte*) surface->pixels, surface->h, surface->pitch);
		SDL_UnlockSurface(surface);
	} else {
		// ...
	}
}

#ifndef USE_FADE
// seg009:19EF
void __pascal far fade_in_2(surface_type near *source_surface,int which_rows) {
	// stub
	method_1_blit_rect(onscreen_surface_, source_surface, &screen_rect, &screen_rect, 0);
	SDL_UpdateRect(onscreen_surface_, 0, 0, 0, 0); // debug
}

// seg009:1CC9
void __pascal far fade_out_2(int rows) {
	// stub
}
#endif // USE_FADE

// seg009:2288
void __pascal far draw_image_transp_vga(image_type far *image,int xpos,int ypos) {
	// stub
	method_6_blit_img_to_scr(image, xpos, ypos, 0x10);
}

#ifdef USE_TEXT

font_type hc_font = {0x01,0xFF, 7,2,1,1, NULL};
textstate_type textstate = {0,0,0,15,&hc_font};

/*const*/ byte hc_font_data[] = {
0x20,0x83,0x07,0x00,0x02,0x00,0x01,0x00,0x01,0x00,0xD2,0x00,0xD8,0x00,0xE5,0x00,
0xEE,0x00,0xFA,0x00,0x07,0x01,0x14,0x01,0x21,0x01,0x2A,0x01,0x37,0x01,0x44,0x01,
0x50,0x01,0x5C,0x01,0x6A,0x01,0x74,0x01,0x81,0x01,0x8E,0x01,0x9B,0x01,0xA8,0x01,
0xB5,0x01,0xC2,0x01,0xCF,0x01,0xDC,0x01,0xE9,0x01,0xF6,0x01,0x03,0x02,0x10,0x02,
0x1C,0x02,0x2A,0x02,0x37,0x02,0x42,0x02,0x4F,0x02,0x5C,0x02,0x69,0x02,0x76,0x02,
0x83,0x02,0x90,0x02,0x9D,0x02,0xAA,0x02,0xB7,0x02,0xC4,0x02,0xD1,0x02,0xDE,0x02,
0xEB,0x02,0xF8,0x02,0x05,0x03,0x12,0x03,0x1F,0x03,0x2C,0x03,0x39,0x03,0x46,0x03,
0x53,0x03,0x60,0x03,0x6D,0x03,0x7A,0x03,0x87,0x03,0x94,0x03,0xA1,0x03,0xAE,0x03,
0xBB,0x03,0xC8,0x03,0xD5,0x03,0xE2,0x03,0xEB,0x03,0xF9,0x03,0x02,0x04,0x0F,0x04,
0x1C,0x04,0x29,0x04,0x36,0x04,0x43,0x04,0x50,0x04,0x5F,0x04,0x6C,0x04,0x79,0x04,
0x88,0x04,0x95,0x04,0xA2,0x04,0xAF,0x04,0xBC,0x04,0xC9,0x04,0xD8,0x04,0xE7,0x04,
0xF4,0x04,0x01,0x05,0x0E,0x05,0x1B,0x05,0x28,0x05,0x35,0x05,0x42,0x05,0x51,0x05,
0x5E,0x05,0x6B,0x05,0x78,0x05,0x85,0x05,0x8D,0x05,0x9A,0x05,0xA7,0x05,0xBB,0x05,
0xD9,0x05,0x00,0x00,0x03,0x00,0x00,0x00,0x07,0x00,0x02,0x00,0x01,0x00,0xC0,0xC0,
0xC0,0xC0,0xC0,0x00,0xC0,0x03,0x00,0x05,0x00,0x01,0x00,0xD8,0xD8,0xD8,0x06,0x00,
0x07,0x00,0x01,0x00,0x00,0x6C,0xFE,0x6C,0xFE,0x6C,0x07,0x00,0x07,0x00,0x01,0x00,
0x10,0x7C,0xD0,0x7C,0x16,0x7C,0x10,0x07,0x00,0x08,0x00,0x01,0x00,0xC3,0xC6,0x0C,
0x18,0x30,0x63,0xC3,0x07,0x00,0x08,0x00,0x01,0x00,0x38,0x6C,0x38,0x7A,0xCC,0xCE,
0x7B,0x03,0x00,0x03,0x00,0x01,0x00,0x60,0x60,0xC0,0x07,0x00,0x04,0x00,0x01,0x00,
0x30,0x60,0xC0,0xC0,0xC0,0x60,0x30,0x07,0x00,0x04,0x00,0x01,0x00,0xC0,0x60,0x30,
0x30,0x30,0x60,0xC0,0x06,0x00,0x07,0x00,0x01,0x00,0x00,0x6C,0x38,0xFE,0x38,0x6C,
0x06,0x00,0x06,0x00,0x01,0x00,0x00,0x30,0x30,0xFC,0x30,0x30,0x08,0x00,0x03,0x00,
0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x60,0x60,0xC0,0x04,0x00,0x04,0x00,0x01,0x00,
0x00,0x00,0x00,0xF0,0x07,0x00,0x02,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0xC0,
0xC0,0x07,0x00,0x08,0x00,0x01,0x00,0x03,0x06,0x0C,0x18,0x30,0x60,0xC0,0x07,0x00,
0x06,0x00,0x01,0x00,0x78,0xCC,0xCC,0xCC,0xCC,0xCC,0x78,0x07,0x00,0x06,0x00,0x01,
0x00,0x30,0x70,0xF0,0x30,0x30,0x30,0xFC,0x07,0x00,0x06,0x00,0x01,0x00,0x78,0xCC,
0x0C,0x18,0x30,0x60,0xFC,0x07,0x00,0x06,0x00,0x01,0x00,0x78,0xCC,0x0C,0x18,0x0C,
0xCC,0x78,0x07,0x00,0x07,0x00,0x01,0x00,0x1C,0x3C,0x6C,0xCC,0xFE,0x0C,0x0C,0x07,
0x00,0x06,0x00,0x01,0x00,0xF8,0xC0,0xC0,0xF8,0x0C,0x0C,0xF8,0x07,0x00,0x06,0x00,
0x01,0x00,0x78,0xC0,0xC0,0xF8,0xCC,0xCC,0x78,0x07,0x00,0x06,0x00,0x01,0x00,0xFC,
0x0C,0x18,0x30,0x30,0x30,0x30,0x07,0x00,0x06,0x00,0x01,0x00,0x78,0xCC,0xCC,0x78,
0xCC,0xCC,0x78,0x07,0x00,0x06,0x00,0x01,0x00,0x78,0xCC,0xCC,0x7C,0x0C,0xCC,0x78,
0x06,0x00,0x02,0x00,0x01,0x00,0x00,0xC0,0xC0,0x00,0xC0,0xC0,0x08,0x00,0x03,0x00,
0x01,0x00,0x00,0x60,0x60,0x00,0x00,0x60,0x60,0xC0,0x07,0x00,0x05,0x00,0x01,0x00,
0x18,0x30,0x60,0xC0,0x60,0x30,0x18,0x05,0x00,0x04,0x00,0x01,0x00,0x00,0x00,0xF0,
0x00,0xF0,0x07,0x00,0x05,0x00,0x01,0x00,0xC0,0x60,0x30,0x18,0x30,0x60,0xC0,0x07,
0x00,0x06,0x00,0x01,0x00,0x78,0xCC,0x0C,0x18,0x30,0x00,0x30,0x07,0x00,0x06,0x00,
0x01,0x00,0x78,0xCC,0xDC,0xDC,0xD8,0xC0,0x78,0x07,0x00,0x06,0x00,0x01,0x00,0x78,
0xCC,0xCC,0xFC,0xCC,0xCC,0xCC,0x07,0x00,0x06,0x00,0x01,0x00,0xF8,0xCC,0xCC,0xF8,
0xCC,0xCC,0xF8,0x07,0x00,0x06,0x00,0x01,0x00,0x78,0xCC,0xC0,0xC0,0xC0,0xCC,0x78,
0x07,0x00,0x06,0x00,0x01,0x00,0xF8,0xCC,0xCC,0xCC,0xCC,0xCC,0xF8,0x07,0x00,0x05,
0x00,0x01,0x00,0xF8,0xC0,0xC0,0xF0,0xC0,0xC0,0xF8,0x07,0x00,0x05,0x00,0x01,0x00,
0xF8,0xC0,0xC0,0xF0,0xC0,0xC0,0xC0,0x07,0x00,0x06,0x00,0x01,0x00,0x78,0xCC,0xC0,
0xDC,0xCC,0xCC,0x78,0x07,0x00,0x06,0x00,0x01,0x00,0xCC,0xCC,0xCC,0xFC,0xCC,0xCC,
0xCC,0x07,0x00,0x04,0x00,0x01,0x00,0xF0,0x60,0x60,0x60,0x60,0x60,0xF0,0x07,0x00,
0x06,0x00,0x01,0x00,0x0C,0x0C,0x0C,0x0C,0x0C,0xCC,0x78,0x07,0x00,0x07,0x00,0x01,
0x00,0xC6,0xCC,0xD8,0xF0,0xD8,0xCC,0xC6,0x07,0x00,0x05,0x00,0x01,0x00,0xC0,0xC0,
0xC0,0xC0,0xC0,0xC0,0xF8,0x07,0x00,0x08,0x00,0x01,0x00,0xC3,0xE7,0xFF,0xDB,0xC3,
0xC3,0xC3,0x07,0x00,0x06,0x00,0x01,0x00,0xCC,0xCC,0xEC,0xFC,0xDC,0xCC,0xCC,0x07,
0x00,0x06,0x00,0x01,0x00,0x78,0xCC,0xCC,0xCC,0xCC,0xCC,0x78,0x07,0x00,0x06,0x00,
0x01,0x00,0xF8,0xCC,0xCC,0xF8,0xC0,0xC0,0xC0,0x07,0x00,0x06,0x00,0x01,0x00,0x78,
0xCC,0xCC,0xCC,0xCC,0xD8,0x6C,0x07,0x00,0x06,0x00,0x01,0x00,0xF8,0xCC,0xCC,0xF8,
0xD8,0xCC,0xCC,0x07,0x00,0x06,0x00,0x01,0x00,0x78,0xCC,0xC0,0x78,0x0C,0xCC,0x78,
0x07,0x00,0x06,0x00,0x01,0x00,0xFC,0x30,0x30,0x30,0x30,0x30,0x30,0x07,0x00,0x06,
0x00,0x01,0x00,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0x7C,0x07,0x00,0x06,0x00,0x01,0x00,
0xCC,0xCC,0xCC,0xCC,0xCC,0x78,0x30,0x07,0x00,0x08,0x00,0x01,0x00,0xC3,0xC3,0xC3,
0xDB,0xFF,0xE7,0xC3,0x07,0x00,0x06,0x00,0x01,0x00,0xCC,0xCC,0x78,0x30,0x78,0xCC,
0xCC,0x07,0x00,0x06,0x00,0x01,0x00,0xCC,0xCC,0xCC,0x78,0x30,0x30,0x30,0x07,0x00,
0x08,0x00,0x01,0x00,0xFF,0x06,0x0C,0x18,0x30,0x60,0xFF,0x07,0x00,0x04,0x00,0x01,
0x00,0xF0,0xC0,0xC0,0xC0,0xC0,0xC0,0xF0,0x07,0x00,0x08,0x00,0x01,0x00,0xC0,0x60,
0x30,0x18,0x0C,0x06,0x03,0x07,0x00,0x04,0x00,0x01,0x00,0xF0,0x30,0x30,0x30,0x30,
0x30,0xF0,0x03,0x00,0x06,0x00,0x01,0x00,0x30,0x78,0xCC,0x08,0x00,0x06,0x00,0x01,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFC,0x03,0x00,0x04,0x00,0x01,0x00,0xC0,
0x60,0x30,0x07,0x00,0x06,0x00,0x01,0x00,0x00,0x00,0x78,0x0C,0x7C,0xCC,0x7C,0x07,
0x00,0x06,0x00,0x01,0x00,0xC0,0xC0,0xF8,0xCC,0xCC,0xCC,0xF8,0x07,0x00,0x06,0x00,
0x01,0x00,0x00,0x00,0x78,0xCC,0xC0,0xCC,0x78,0x07,0x00,0x06,0x00,0x01,0x00,0x0C,
0x0C,0x7C,0xCC,0xCC,0xCC,0x7C,0x07,0x00,0x06,0x00,0x01,0x00,0x00,0x00,0x78,0xCC,
0xFC,0xC0,0x7C,0x07,0x00,0x05,0x00,0x01,0x00,0x38,0x60,0xF8,0x60,0x60,0x60,0x60,
0x09,0x00,0x06,0x00,0x01,0x00,0x00,0x00,0x78,0xCC,0xCC,0xCC,0x7C,0x0C,0x78,0x07,
0x00,0x06,0x00,0x01,0x00,0xC0,0xC0,0xF8,0xCC,0xCC,0xCC,0xCC,0x07,0x00,0x02,0x00,
0x01,0x00,0xC0,0x00,0xC0,0xC0,0xC0,0xC0,0xC0,0x09,0x00,0x04,0x00,0x01,0x00,0x30,
0x00,0x30,0x30,0x30,0x30,0x30,0x30,0xE0,0x07,0x00,0x06,0x00,0x01,0x00,0xC0,0xC0,
0xCC,0xD8,0xF0,0xD8,0xCC,0x07,0x00,0x02,0x00,0x01,0x00,0xC0,0xC0,0xC0,0xC0,0xC0,
0xC0,0xC0,0x07,0x00,0x08,0x00,0x01,0x00,0x00,0x00,0xFE,0xDB,0xDB,0xDB,0xDB,0x07,
0x00,0x06,0x00,0x01,0x00,0x00,0x00,0xF8,0xCC,0xCC,0xCC,0xCC,0x07,0x00,0x06,0x00,
0x01,0x00,0x00,0x00,0x78,0xCC,0xCC,0xCC,0x78,0x09,0x00,0x06,0x00,0x01,0x00,0x00,
0x00,0xF8,0xCC,0xCC,0xCC,0xF8,0xC0,0xC0,0x09,0x00,0x06,0x00,0x01,0x00,0x00,0x00,
0x78,0xCC,0xCC,0xCC,0x7C,0x0C,0x0C,0x07,0x00,0x06,0x00,0x01,0x00,0x00,0x00,0x78,
0xCC,0xC0,0xC0,0xC0,0x07,0x00,0x06,0x00,0x01,0x00,0x00,0x00,0x78,0xC0,0x78,0x0C,
0xF8,0x07,0x00,0x05,0x00,0x01,0x00,0x60,0x60,0xF8,0x60,0x60,0x60,0x38,0x07,0x00,
0x06,0x00,0x01,0x00,0x00,0x00,0xCC,0xCC,0xCC,0xCC,0x7C,0x07,0x00,0x06,0x00,0x01,
0x00,0x00,0x00,0xCC,0xCC,0xCC,0x78,0x30,0x07,0x00,0x08,0x00,0x01,0x00,0x00,0x00,
0xC3,0xC3,0xDB,0xFF,0x66,0x07,0x00,0x06,0x00,0x01,0x00,0x00,0x00,0xCC,0x78,0x30,
0x78,0xCC,0x09,0x00,0x06,0x00,0x01,0x00,0x00,0x00,0xCC,0xCC,0xCC,0xCC,0x7C,0x0C,
0x78,0x07,0x00,0x06,0x00,0x01,0x00,0x00,0x00,0xFC,0x18,0x30,0x60,0xFC,0x07,0x00,
0x04,0x00,0x01,0x00,0x30,0x60,0x60,0xC0,0x60,0x60,0x30,0x07,0x00,0x02,0x00,0x01,
0x00,0xC0,0xC0,0xC0,0x00,0xC0,0xC0,0xC0,0x07,0x00,0x04,0x00,0x01,0x00,0xC0,0x60,
0x60,0x30,0x60,0x60,0xC0,0x02,0x00,0x07,0x00,0x01,0x00,0x76,0xDC,0x07,0x00,0x07,
0x00,0x01,0x00,0x00,0x00,0x70,0xC4,0xCC,0x8C,0x38,0x07,0x00,0x07,0x00,0x01,0x00,
0x00,0x06,0x0C,0xD8,0xF0,0xE0,0xC0,0x08,0x00,0x10,0x00,0x02,0x00,0x7F,0xFE,0xCD,
0xC7,0xB5,0xEF,0xB5,0xEF,0x85,0xEF,0xB5,0xEF,0xB4,0x6F,0x08,0x00,0x13,0x00,0x03,
0x00,0x7F,0xFF,0xC0,0xCC,0x46,0xE0,0xB6,0xDA,0xE0,0xBE,0xDA,0xE0,0xBE,0xC6,0xE0,
0xB6,0xDA,0xE0,0xCE,0xDA,0x20,0x7F,0xFF,0xC0,0x08,0x00,0x11,0x00,0x03,0x00,0x7F,
0xFF,0x00,0xC6,0x73,0x80,0xDD,0xAD,0x80,0xCE,0xEF,0x80,0xDF,0x6F,0x80,0xDD,0xAD,
0x80,0xC6,0x73,0x80,0x7F,0xFF,0x00
};

font_type load_font_from_data(/*const*/ rawfont_type* data) {
	font_type font;
	font.first_char = data->first_char;
	font.last_char = data->last_char;
	font.height_above_baseline = data->height_above_baseline;
	font.height_below_baseline = data->height_below_baseline;
	font.space_between_lines = data->space_between_lines;
	font.space_between_chars = data->space_between_chars;
	int n_chars = font.last_char - font.first_char + 1;
	chtab_type* chtab = malloc(sizeof(chtab_type) + sizeof(image_type* far) * n_chars);
	int chr,index;
	// Make a dummy palette for decode_image().
	dat_pal_type dat_pal;
	memset(&dat_pal, 0, sizeof(dat_pal));
	dat_pal.vga[1].r = dat_pal.vga[1].g = dat_pal.vga[1].b = 0x3F; // white
	for (index = 0, chr = data->first_char; chr <= data->last_char; ++index, ++chr) {
		/*const*/ image_data_type* image = (/*const*/ image_data_type*)((/*const*/ byte*)data + data->offsets[index]);
		//image->flags=0;
		if (image->height == 0) image->height = 1; // HACK: decode_image() returns NULL if height==0.
		chtab->pointers[index] = decode_image(image, &dat_pal);
	}
	font.chtab = chtab;
	return font;
}

void load_font() {
	// Try to load font from a file.
	dat_type* dathandle = open_dat("font", 0);
	hc_font.chtab = load_sprites_from_file(1000, 1<<1/*, 0, 0*/,0);
	close_dat(dathandle);
	if (hc_font.chtab == NULL) {
		// Use built-in font.
		hc_font = load_font_from_data((/*const*/ rawfont_type*)hc_font_data);
	}
}

// seg009:35C5
int __pascal far get_char_width(byte character) {
	font_type* font = textstate.ptr_font;
	int ax = 0;
	if (character <= font->last_char && character >= font->first_char) {
		ax += font->chtab->pointers[character - font->first_char]->w; //char_ptrs[character - font->first_char]->width;
		if (ax) ax += font->space_between_chars;
	}
	return ax;
}

// seg009:3E99
int __pascal far measure_text_line(const char far *text,int length,int break_width,int x_align) {
	short curr_line_width;
	short curr_line_length;
	int di = 0;
	curr_line_length = 0;
	curr_line_width = 0;
	const char* si = text;
	while (di < length) {
		curr_line_width += get_char_width(*si);
		if (curr_line_width <= break_width) {
			++di;
			char al = *(si++);
			if (al == '\r') {
				return di;
			}
			if (al == '-' ||
				(x_align <= 0 && (al == ' ' || *si == ' ')) ||
				(*si == ' ' && al == ' ')
			) {
				// May break here
				curr_line_length = di;
			}
		} else {
			if (curr_line_length == 0) {
				// If the first word is wider than the rect then break it.
				return di;
			} else {
				// Otherwise break at the last space.
				return curr_line_length;
			}
		}
	}
	return di;
}

// seg009:403F
int __pascal far get_line_width(const char far *text,int length) {
	int di = 0;
	const char* si = text;
	while (--length >= 0) {
		di += get_char_width(*(si++));
	}
	return di;
}

// seg009:3706
int __pascal far draw_text_character(byte character) {
	font_type* font = textstate.ptr_font;
	int ax = 0;
	if (character <= font->last_char && character >= font->first_char) {
		image_type* image = font->chtab->pointers[character - font->first_char]; //char_ptrs[character - font->first_char];
		method_3_blit_mono(image, textstate.current_x, textstate.current_y - font->height_above_baseline, textstate.textblit, textstate.textcolor);
		ax = font->space_between_chars + image->w;
	}
	textstate.current_x += ax;
	return ax;
}

// seg009:377F
int __pascal far draw_text_line(const char far *text,int length) {
	//hide_cursor();
	int di = 0;
	const char* si = text;
	while (--length >= 0) {
		di += draw_text_character(*(si++));
	}
	//show_cursor();
	return di;
}

// seg009:3755
int __pascal far draw_cstring(const char far *string) {
	//hide_cursor();
	int di = 0;
	const char* si = string;
	while (*si) {
		di += draw_text_character(*(si++));
	}
	//show_cursor();
	return di;
}

// seg009:3F01
const rect_type far *__pascal draw_text(const rect_type far *rect_ptr,int x_align_or_break,int y_align,const char far *text_ptr,int length) {
	short rect_top;
	short rect_height;
	short rect_width;
	//textinfo_type var_C;
	short num_lines;
	short font_line_distance;
	//hide_cursor();
	//get_textinfo(&var_C);
	set_clip_rect(rect_ptr);
	rect_width = rect_ptr->right - rect_ptr->left;
	rect_top = rect_ptr->top;
	rect_height = rect_ptr->bottom - rect_ptr->top;
	num_lines = 0;
	int di = length;
	const char* si = text_ptr;
	static const int max_lines = 100;
	const char* line_starts[max_lines];
	int line_lengths[max_lines];
	do {
		int ax = measure_text_line(si, di, rect_width, x_align_or_break);
		if (ax == 0) break;
		if (num_lines >= max_lines) {
			//... ERROR!
			printf("draw_text(): Too many lines!\n");
			quit(1);
		}
		line_starts[num_lines] = si;
		line_lengths[num_lines] = ax;
		++num_lines;
		si += ax;
		di -= ax;
	} while(di);
	font_type* font = textstate.ptr_font;
	font_line_distance = font->height_above_baseline + font->height_below_baseline + font->space_between_lines;
	int text_height = font_line_distance * num_lines - font->space_between_lines;
	int text_top = rect_top;
	if (y_align >= 0) {
		if (y_align <= 0) {
			// middle
			text_top += rect_height/2 - text_height/2;
		} else {
			// bottom
			text_top += rect_height - text_height;
		}
	}
	textstate.current_y = text_top + font->height_above_baseline;
	int i;
	for (i = 0; i < num_lines; ++i) {
		const char* bx = line_starts[i];
		int dx = line_lengths[i];
		if (x_align_or_break < 0 &&
			*bx == ' ' &&
			i != 0 &&
			*(bx-1) != '\r'
		) {
			// Skip over space if it's not at the beginning of a line.
			++bx;
			--dx;
			if (dx != 0 &&
				*bx == ' ' &&
				*(bx-2) == '.'
			) {
				// Skip over second space after point.
				++bx;
				--dx;
			}
		}
		int ax = get_line_width(bx,dx);
		int text_left = rect_ptr->left;
		if (x_align_or_break >= 0) {
			if (x_align_or_break <= 0) {
				// center
				text_left += rect_width/2 - ax/2;
			} else {
				// right
				text_left += rect_width - ax;
			}
		}
		textstate.current_x = text_left;
		draw_text_line(bx,dx);
		textstate.current_y += font_line_distance;
	}
	reset_clip_rect();
	//set_textinfo(...);
	//show_cursor();
	return rect_ptr;
}

// seg009:3E4F
void __pascal far show_text(const rect_type far *rect_ptr,int x_align_or_break,int y_align,const char far *text) {
	// stub
	//printf("show_text: %s\n",text);
	draw_text(rect_ptr, x_align_or_break, y_align, text, strlen(text));
}

// seg009:04FF
void __pascal far show_text_with_color(const rect_type far *rect_ptr,int x_align,int y_align, const char far *text,int color) {
	short saved_textcolor;
	saved_textcolor = textstate.textcolor;
	textstate.textcolor = color;
	show_text(rect_ptr, x_align, y_align, text);
	textstate.textcolor = saved_textcolor;
}

// seg009:3A91
void __pascal far set_curr_pos(int xpos,int ypos) {
	textstate.current_x = xpos;
	textstate.current_y = ypos;
}

// seg009:0838
int __pascal far showmessage(char far *text,int arg_4,void far *arg_0) {
#if 0
	word key;
	rect_type rect;
	font_type* saved_font_ptr;
	surface_type* old_target;
	old_target = current_target_surface;
	current_target_surface = onscreen_surface_;
	method_1_blit_rect(word_1F942->0x14, onscreen_surface_, &word_1F942->0x0A, &word_1F942->0x0A, 0);
	sub_D16E(word_1F942);
	saved_font_ptr = current_target_surface->ptr_font;
	current_target_surface->ptr_font = ptr_font;
	shrink2_rect(&rect, word_1F942->0x02, 2, 1);
	show_text_with_color(&rect, 0, 0, text, 15);
	current_target_surface->ptr_font = saved_font_ptr;
	clear_kbd_buf();
	do {
		key = key_test_quit();
	} while(key == 0);
	sub_DF99(word_1F942->0x14);
	current_target_surface = old_target;
	return key;
#else // 0
	return 0;
#endif // 0
}

// seg009:0C44
void __pascal far show_dialog(const char *text) {
	char string[256];
	snprintf(string, sizeof(string), "%s\r\rPress any key to continue.", text);
	//showmessage(string, 1, &key_test_quit);
}

// seg009:0791
int __pascal far get_text_center_y(const rect_type far *rect) {
	const font_type far* font;
	short var_6;
	font = &hc_font;//current_target_surface->ptr_font;
	var_6 = rect->bottom - font->height_above_baseline - font->height_below_baseline - rect->top;
	return ((var_6 - var_6 % 2) >> 1) + font->height_above_baseline + var_6 % 2 + rect->top;
}

// seg009:3E77
int __pascal far get_cstring_width(const char far *text) {
	int di = 0;
	const char* si = text;
	char al;
	while (0 != (al = *(si++))) {
		di += get_char_width(al);
	}
	return di;
}

// seg009:0767
void __pascal far draw_text_cursor(int xpos,int ypos,int color) {
	set_curr_pos(xpos, ypos);
	/*current_target_surface->*/textstate.textcolor = color;
	draw_text_character('_');
	//restore_curr_color();
	textstate.textcolor = 15;
}

// seg009:053C
int __pascal far input_str(const rect_type far *rect,char *buffer,int max_length,const char *initial,int has_initial,int arg_4,int color,int bgcolor) {
	short length;
	word key;
	short var_6;
	short current_xpos;
	short ypos;
	short init_length;
	length = 0;
	var_6 = 0;
	draw_rect(rect, bgcolor);
	init_length = strlen(initial);
	if (has_initial) {
		strcpy(buffer, initial);
		length = init_length;
	}
	current_xpos = rect->left + arg_4;
	ypos = get_text_center_y(rect);
	set_curr_pos(current_xpos, ypos);
	/*current_target_surface->*/textstate.textcolor = color;
	draw_cstring(initial);
	//restore_curr_pos?();
	current_xpos += get_cstring_width(initial) + (init_length != 0) * arg_4;
	do {
		key = 0;
		do {
			if (var_6) {
				draw_text_cursor(current_xpos, ypos, color);
			} else {
				draw_text_cursor(current_xpos, ypos, bgcolor);
			}
			var_6 = !var_6;
			start_timer(0, 6);
			if (key) {
				if (var_6) {
					draw_text_cursor(current_xpos, ypos, color);
					var_6 = !var_6;
				}
				if (key == 0x0D) { // enter
					buffer[length] = 0;
					return length;
				} else break;
			}
//			while (!timer_stopped[0] && (key = key_test_quit()) == 0) idle();
			while (!has_timer_stopped(0) && (key = key_test_quit()) == 0) idle();
		} while (1);
		if (key == 0x1B) { // esc
			draw_rect(rect, bgcolor);
			buffer[0] = 0;
			return -1;
		}
		if (length != 0 && (key == 8 || key == 0x5300)) { // backspace, delete
			--length;
			draw_text_cursor(current_xpos, ypos, bgcolor);
			current_xpos -= get_char_width(buffer[length]);
			set_curr_pos(current_xpos, ypos);
			/*current_target_surface->*/textstate.textcolor = bgcolor;
			draw_text_character(buffer[length]);
			//restore_curr_pos?();
			draw_text_cursor(current_xpos, ypos, color);
		} else if (key >= 0x20 && key <= 0x7E && length < max_length) {
			// Would the new character make the cursor go past the right side of the rect?
			if (get_char_width('_') + get_char_width(key) + current_xpos < rect->right) {
				draw_text_cursor(current_xpos, ypos, bgcolor);
				set_curr_pos(current_xpos, ypos);
				/*current_target_surface->*/textstate.textcolor = color;
				current_xpos += draw_text_character(buffer[length++] = key);
			}
		}
	} while(1);
}

#else // USE_TEXT

// seg009:3706
int __pascal far draw_text_character(byte character) {
	// stub
	printf("draw_text_character: %c\n",character);
	return 0;
}

// seg009:3E4F
void __pascal far show_text(const rect_type far *rect_ptr,int x_align_or_break,int y_align,const char far *text) {
	// stub
	printf("show_text: %s\n",text);
}

// seg009:04FF
void __pascal far show_text_with_color(const rect_type far *rect_ptr,int x_align,int y_align,char far *text,int color) {
	//short saved_textcolor;
	//saved_textcolor = textstate.textcolor;
	//textstate.textcolor = color;
	show_text(rect_ptr, x_align, y_align, text);
	//textstate.textcolor = saved_textcolor;
}

// seg009:3A91
void __pascal far set_curr_pos(int xpos,int ypos) {
	// stub
}

// seg009:0C44
void __pascal far show_dialog(char *text) {
	// stub
	puts(text);
}

// seg009:053C
int __pascal far input_str(const rect_type far *rect,char *buffer,int max_length,const char *initial,int has_initial,int arg_4,int color,int bgcolor) {
	// stub
	strncpy(buffer, "dummy input text", max_length);
	return strlen(buffer);
}

#endif // USE_TEXT

// seg009:37E8
void __pascal far draw_rect(const rect_type far *rect,int color) {
	method_5_rect(rect, blitters_0_no_transp, color);
}

// seg009:3985
surface_type far *__pascal rect_sthg(surface_type *surface,const rect_type far *rect) {
	// stub
	return surface;
}

// seg009:39CE
rect_type far *__pascal shrink2_rect(rect_type far *target_rect,const rect_type far *source_rect,int delta_x,int delta_y) {
	target_rect->top    = source_rect->top    + delta_y;
	target_rect->left   = source_rect->left   + delta_x;
	target_rect->bottom = source_rect->bottom - delta_y;
	target_rect->right  = source_rect->right  - delta_x;
	return target_rect;
}

// seg009:3BBA
void __pascal far restore_peel(peel_type peel_ptr) {
	//printf("restoring peel at (x=%d, y=%d)\n", peel_ptr.rect.left, peel_ptr.rect.top); // debug
	method_6_blit_img_to_scr(peel_ptr.peel, peel_ptr.rect.left, peel_ptr.rect.top, /*0x10*/0);
	free_peel(&peel_ptr);
	//SDL_FreeSurface(peel_ptr.peel);
}

// seg009:3BE9
peel_type __pascal far read_peel_from_screen(const rect_type far *rect) {
	// stub
	peel_type result;
	memset(&result, 0, sizeof(result));
	result.rect = *rect;
#ifndef USE_ALPHA
	SDL_Surface* peel_surface = SDL_CreateRGBSurface(0, rect->right - rect->left, rect->bottom - rect->top, 24, 0xFF, 0xFF<<8, 0xFF<<16, 0);
#else
	SDL_Surface* peel_surface = SDL_CreateRGBSurface(0, rect->right - rect->left, rect->bottom - rect->top, 32, 0xFF, 0xFF<<8, 0xFF<<16, 0xFF<<24);
#endif
	if (peel_surface == NULL) {
		sdlperror("SDL_CreateRGBSurface");
		quit(1);
	}
	result.peel = peel_surface;
	rect_type target_rect = {0, 0, rect->right - rect->left, rect->bottom - rect->top};
	method_1_blit_rect(result.peel, /*surface*/current_target_surface, &target_rect, rect, 0);
	return result;
}

// seg009:3D95
int __pascal far intersect_rect(rect_type far *output,const rect_type far *input1,const rect_type far *input2) {
	short left = MAX(input1->left, input2->left);
	short right = MIN(input1->right, input2->right);
	if (left < right) {
		output->left = left;
		output->right = right;
		short top = MAX(input1->top, input2->top);
		short bottom = MIN(input1->bottom, input2->bottom);
		if (top < bottom) {
			output->top = top;
			output->bottom = bottom;
			return 1;
		}
	}
	memset(output, 0, sizeof(rect_type));
	return 0;
}

// seg009:4063
rect_type far * __pascal far union_rect(rect_type far *output,const rect_type far *input1,const rect_type far *input2) {
	short top = MIN(input1->top, input2->top);
	short left = MIN(input1->left, input2->left);
	short bottom = MAX(input1->bottom, input2->bottom);
	short right = MAX(input1->right, input2->right);
	output->top = top;
	output->left = left;
	output->bottom = bottom;
	output->right = right;
	return output;
}

const int userevent_SOUND = 'SOUN';
const int userevent_TIMER = 'TIME';

SDL_TimerID sound_timer = NULL;
short speaker_playing = 0;
short digi_playing = 0;
short midi_playing = 0;

void __pascal far speaker_sound_stop() {
	// stub
	speaker_playing = 0;
	if (sound_timer != NULL) {
		if (!SDL_RemoveTimer(sound_timer)) {
			sdlperror("SDL_RemoveTimer in speaker_sound_stop");
			//quit(1);
		}
		sound_timer = NULL;
	}
}

// The current buffer, holds the resampled sound data.
byte* digi_buffer = NULL;
// The current position in digi_buffer.
byte* digi_remaining_pos = NULL;
// The remaining length.
size_t digi_remaining_length = 0;

// The properties of the audio device.
SDL_AudioSpec* digi_audiospec = NULL;
// The desired samplerate. Everything will be resampled to this.
const int digi_samplerate = 22050;

void stop_digi() {
#ifndef USE_MIXER
	SDL_PauseAudio(1);
	if (!digi_playing) return;
	SDL_LockAudio();
	digi_playing = 0;
	/*
//	if (SDL_GetAudioStatus() == SDL_AUDIO_PLAYING) {
		SDL_PauseAudio(1);
		SDL_CloseAudio();
//	}
	if (digi_audiospec != NULL) {
		free(digi_audiospec);
		digi_audiospec = NULL;
	}
	*/
	if (digi_buffer != NULL) {
		free(digi_buffer);
		digi_buffer = NULL;
	}
	digi_remaining_length = 0;
	digi_remaining_pos = NULL;
	SDL_UnlockAudio();
#else
	Mix_HaltChannel(-1);
	digi_playing = 0;
#endif
}

// seg009:7214
void __pascal far stop_sounds() {
	// stub
	stop_digi();
	// stop_midi();
	speaker_sound_stop();
}

Uint32 speaker_callback(Uint32 interval, void *param) {
	SDL_Event event;
	memset(&event, 0, sizeof(event));
	event.type = SDL_USEREVENT;
	event.user.code = userevent_SOUND;
	event.user.data1 = param;
	if (!SDL_RemoveTimer(sound_timer)) {
		sdlperror("SDL_RemoveTimer in speaker_callback");
		//quit(1);
	}
	sound_timer = NULL;
	speaker_playing = 0;
	// First remove the timer, then allow the other thread to continue.
	SDL_PushEvent(&event);
	return 0;
}

// seg009:7640
void __pascal far play_speaker_sound(sound_buffer_type far *buffer) {
	// stub
	//speaker_sound_stop();
	stop_sounds();
	int length = 0;
	int index;
	for (index = 0; buffer->speaker.notes[index].frequency != 0x12; ++index) {
		length += buffer->speaker.notes[index].length;
	}
	int time_ms = length*1000 / buffer->speaker.tempo;
	//printf("length = %d ms\n", time_ms);
	sound_timer = SDL_AddTimer(time_ms, speaker_callback, NULL);
	if (sound_timer == NULL) {
		sdlperror("SDL_AddTimer");
		quit(1);
	}
	speaker_playing = 1;
}

#ifndef USE_MIXER
void digi_callback(void *userdata, Uint8 *stream, int len) {
	// Don't go over the end of either the input or the output buffer.
	size_t copy_len = MIN(len, digi_remaining_length);
	//printf("digi_callback(): copy_len = %d\n", copy_len);
	//printf("digi_callback(): len = %d\n", len);
	if (is_sound_on) {
		// Copy the next part of the input of the output.
		memcpy(stream, digi_remaining_pos, copy_len);
		// In case the sound does not fill the buffer: fill the rest of the buffer with silence.
		memset(stream + copy_len, digi_audiospec->silence, len - copy_len);
	} else {
		// If sound is off: Mute the sound but keep track of where we are.
		memset(stream, digi_audiospec->silence, len);
	}
	// If the sound ended, push an event.
	if (digi_playing && digi_remaining_length == 0) {
		//printf("digi_callback(): sound ended\n");
		SDL_Event event;
		memset(&event, 0, sizeof(event));
		event.type = SDL_USEREVENT;
		event.user.code = userevent_SOUND;
		digi_playing = 0;
		SDL_PushEvent(&event);
	}
	// Advance the pointer.
	digi_remaining_length -= copy_len;
	digi_remaining_pos += copy_len;
}
#endif

#ifdef USE_MIXER
void channel_finished(int channel) {
	digi_playing = 0;
	//printf("Finished channel %d\n", channel);
	SDL_Event event;
	memset(&event, 0, sizeof(event));
	event.type = SDL_USEREVENT;
	event.user.code = userevent_SOUND;
	SDL_PushEvent(&event);
}
#endif

int digi_unavailable = 0;
void init_digi() {
	if (digi_unavailable) return;
	if (digi_audiospec != NULL) return;
	// Open the audio device. Called once.
	//printf("init_digi(): called\n");
	SDL_AudioSpec *desired;
	desired = (SDL_AudioSpec *)malloc(sizeof(SDL_AudioSpec));
	memset(desired, 0, sizeof(SDL_AudioSpec));
	desired->freq = digi_samplerate; //buffer->digi.sample_rate;
	desired->format = AUDIO_S16;
	desired->channels = 1;
	//desired->samples = buffer->digi.sample_count;
	desired->samples = /*4096*/ /*512*/ 256;
#ifndef USE_MIXER
	desired->callback = digi_callback;
	desired->userdata = NULL;
	if (SDL_OpenAudio(desired, NULL) != 0) {
		sdlperror("SDL_OpenAudio");
		//quit(1);
		digi_unavailable = 1;
		return;
	}
	//SDL_PauseAudio(0);
#else
	if (Mix_OpenAudio(desired->freq, desired->format, desired->channels, desired->samples) != 0) {
		sdlperror("Mix_OpenAudio");
		digi_unavailable = 1;
		return;
	}
	Mix_AllocateChannels(1);
	Mix_ChannelFinished(channel_finished);
#endif
	digi_audiospec = desired;
}

#ifdef USE_MIXER
const int sound_channel = 0;
const int max_sound_id = 58;
char** sound_names = NULL;

void load_sound_names() {
	if (sound_names != NULL) return;
	FILE* fp = fopen("data/music/names.txt","rt");
	if (fp==NULL) return;
	sound_names = (char**) calloc(sizeof(char*) * max_sound_id, 1);
	while (!feof(fp)) {
		int number;
		char name[256];
		fscanf(fp, "%d=%255s\n", &number, /*sizeof(name)-1,*/ name);
		//if (feof(fp)) break;
		printf("sound_names[%d] = %s\n",number,name);
		if (number>=0 && number<max_sound_id) {
			sound_names[number] = malloc(strlen(name)+1);
			strcpy(sound_names[number], name);
		}
	}
	fclose(fp);
}
#endif

sound_buffer_type* load_sound(int index) {
	sound_buffer_type* result = NULL;
#ifdef USE_MIXER
	//printf("load_sound(%d)\n", index);
	init_digi();
	if (!digi_unavailable && result == NULL && index>=0 && index<max_sound_id) {
		//printf("Trying to load from music folder\n");
		load_sound_names();
		if (sound_names != NULL && sound_names[index] != NULL) {
			//printf("Loading from music folder\n");
			const char* exts[]={"mp3","ogg","flac","wav"};
			int i;
			for (i = 0; i < COUNT(exts); ++i) {
				//printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
				char filename[256];
				const char* ext=exts[i];
				snprintf(filename, sizeof(filename), "data/music/%s.%s", sound_names[index], ext);
				//printf("Trying to load %s\n", filename);
				Mix_Chunk* chunk = Mix_LoadWAV(filename);
				if (chunk == NULL) {
					sdlperror("Mix_LoadWAV");
                    printf("error loading sound file %s\n", filename); 
					continue;
				}
				//printf("Loaded sound from %s\n", filename);
				result = malloc(sizeof(sound_buffer_type));
				result->type = sound_chunk;
				result->chunk = chunk;
				break;
			}
		} else {
			//printf("sound_names = %p\n", sound_names);
			//printf("sound_names[%d] = %p\n", index, sound_names[index]);
		}
	}
#endif
	if (result == NULL) {
		//printf("Trying to load from DAT\n");
		result = (sound_buffer_type*) load_from_opendats_alloc(index + 10000, "bin", NULL, NULL);
	}
	return result;
}

#ifdef USE_MIXER
void __pascal far play_chunk_sound(sound_buffer_type far *buffer) {
	//if (!is_sound_on) return;
	init_digi();
	if (digi_unavailable) return;
	stop_sounds();
	//printf("playing chunk sound %p\n", buffer);
	if (Mix_PlayChannel(sound_channel, buffer->chunk, 0) == -1) {
		sdlperror("Mix_PlayChannel");
	}
	digi_playing = 1;
}

Uint32 fourcc(char* string) {
	return *(Uint32*)string;
}
#endif

// seg009:74F0
void __pascal far play_digi_sound(sound_buffer_type far *buffer) {
	//if (!is_sound_on) return;
	init_digi();
	if (digi_unavailable) return;
	//stop_digi();
	stop_sounds();
	//printf("play_digi_sound(): called\n");
	if (buffer->digi.sample_size != 8) return;
#ifndef USE_MIXER	
	SDL_AudioCVT cvt;
	memset(&cvt, 0, sizeof(cvt));
	int result = SDL_BuildAudioCVT(&cvt,
		AUDIO_S16, 1, buffer->digi.sample_rate,
		digi_audiospec->format, digi_audiospec->channels, digi_audiospec->freq
	);
	// The case of result == 0 is undocumented, but it may occur.
	if (result != 1 && result != 0) {
		sdlperror("SDL_BuildAudioCVT");
		printf("(returned %d)\n", result);
		quit(1);
	}
	int dlen = buffer->digi.sample_count; // if format is AUDIO_U8
	cvt.buf = (Uint8*) malloc(dlen * cvt.len_mult);
	memcpy(cvt.buf, buffer->digi.samples, dlen);
	cvt.len = dlen;
	if (SDL_ConvertAudio(&cvt) != 0) {
		sdlperror("SDL_ConvertAudio");
		quit(1);
	}
	
	SDL_LockAudio();
	digi_buffer = cvt.buf;
	digi_playing = 1;
//	digi_remaining_length = buffer->digi.sample_count;
//	digi_remaining_pos = buffer->digi.samples;
	digi_remaining_length = cvt.len_cvt;
	digi_remaining_pos = digi_buffer;
	SDL_UnlockAudio();
	SDL_PauseAudio(0);
#else
	// Convert the DAT sound to WAV, so the Mixer can load it.
//#error "What to put here?..."
	int size = buffer->digi.sample_count;
	int rounded_size = (size+1)&(~1);
	int alloc_size = sizeof(WAV_header_type) + rounded_size;
	WAV_header_type* wav_data = malloc(alloc_size);
	wav_data->ChunkID = fourcc("RIFF");
	wav_data->ChunkSize = 36 + rounded_size;
	wav_data->Format = fourcc("WAVE");
	wav_data->Subchunk1ID = fourcc("fmt ");
	wav_data->Subchunk1Size = 16;
	wav_data->AudioFormat = 1; // PCM
	wav_data->NumChannels = 1; // Mono
	wav_data->SampleRate = buffer->digi.sample_rate;
	wav_data->BitsPerSample = buffer->digi.sample_size;
	wav_data->ByteRate = wav_data->SampleRate * wav_data->NumChannels * wav_data->BitsPerSample/8;
	wav_data->BlockAlign = wav_data->NumChannels * wav_data->BitsPerSample/8;
	wav_data->Subchunk2ID = fourcc("data");
	wav_data->Subchunk2Size = size;
	memcpy(wav_data->Data, buffer->digi.samples, size);
	SDL_RWops* rw = SDL_RWFromConstMem(wav_data, alloc_size);
	Mix_Chunk *chunk = Mix_LoadWAV_RW(rw, 1);
	if (chunk == NULL) {
		FILE* fp = fopen("dump.wav","wb");
		fwrite(wav_data,alloc_size,1,fp);
		fclose(fp);
	}
	free(wav_data);
	if (chunk == NULL) {
		sdlperror("Mix_LoadWAV_RW");
		return;
	}
	buffer->type = sound_chunk;
	buffer->chunk = chunk;
	play_chunk_sound(buffer);
#endif
}

void free_sound(sound_buffer_type far *buffer) {
	if (buffer == NULL) return;
#ifdef USE_MIXER
	if (buffer->type == sound_chunk) {
		Mix_FreeChunk(buffer->chunk);
	}
#endif
	free(buffer);
}

// seg009:7220
void __pascal far play_sound_from_buffer(sound_buffer_type far *buffer) {
	// stub
	if (buffer == NULL) {
		printf("Tried to play NULL sound.");
		quit(1);
		//return;
	}
	switch (buffer->type & 3) {
		case sound_speaker:
			play_speaker_sound(buffer);
		break;
		case sound_digi:
			play_digi_sound(buffer);
		break;
#ifdef USE_MIXER
		case sound_chunk:
			play_chunk_sound(buffer);
		break;
#endif
		default:
			printf("Tried to play unimplemented sound type %d.", buffer->type);
			quit(1);
		break;
	}
}

// seg009:7273
void __pascal far turn_sound_on_off(byte new_state) {
	// stub
	is_sound_on = new_state;
	//if (!is_sound_on) stop_sounds();
#ifdef USE_MIXER
	init_digi();
	if (digi_unavailable) return;
	Mix_Volume(-1, is_sound_on ? MIX_MAX_VOLUME : 0);
#endif
}

// seg009:7299
int __pascal far check_sound_playing() {
	return speaker_playing || digi_playing || midi_playing;
}

// seg009:38ED
void __pascal far set_gr_mode(byte grmode) {
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_NOPARACHUTE) != 0) {
		sdlperror("SDL_Init");
		quit(1);
	}
	SDL_EnableUNICODE(1);
	Uint32 flags = 0;
	int fullscreen = check_param("full") != 0;
	if (fullscreen) flags |= SDL_FULLSCREEN;
	//onscreen_surface_ = SDL_SetVideoMode(320, 200, 24, flags);
	onscreen_surface_ = SDL_SetVideoMode(320, 240, 16, SDL_FULLSCREEN | SDL_SWSURFACE | SDL_DOUBLEBUF);
	if (onscreen_surface_ == NULL) {
		sdlperror("SDL_SetVideoMode");
		quit(1);
	}
	if (fullscreen) {
		SDL_ShowCursor(SDL_DISABLE);
	}
	SDL_WM_SetCaption(WINDOW_TITLE, NULL);
	if (SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL) != 0) {
		sdlperror("SDL_EnableKeyRepeat");
		quit(1);
	}
	graphics_mode = gmMcgaVga;
#ifdef USE_TEXT
	load_font();
#endif
}

// seg009:9289
void __pascal far set_pal_arr(int start,int count,const rgb_type far *array,int vsync) {
	// stub
	int i;
	for (i = 0; i < count; ++i) {
		if (array) {
			set_pal(start + i, array[i].r, array[i].g, array[i].b, vsync);
		} else {
			set_pal(start + i, 0, 0, 0, vsync);
		}
	}
}

rgb_type palette[256];

// seg009:92DF
void __pascal far set_pal(int index,int red,int green,int blue,int vsync) {
	// stub
	//palette[index] = ((red&0x3F)<<2)|((green&0x3F)<<2<<8)|((blue&0x3F)<<2<<16);
	palette[index].r = red;
	palette[index].g = green;
	palette[index].b = blue;
}

// seg009:969C
int __pascal far add_palette_bits(byte n_colors) {
	// stub
	return 0;
}

// seg009:97E4
void __pascal far process_palette(void *target,dat_pal_type far *source) {
	// stub
}

// seg009:98F0 ; imghdr_type far *__pascal decode_image_(imghdr_type far *source,char near *xlat_tbl)

// seg009:9C36
int __pascal far find_first_pal_row(int which_rows_mask) {
	word which_row = 0;
	word row_mask = 1;
	do {
		if (row_mask & which_rows_mask) {
			return which_row;
		}
		++which_row;
		row_mask <<= 1;
	} while (which_row < 16);
	return 0;
}

// seg009:9C6C
int __pascal far get_text_color(int cga_color,int low_half,int high_half_mask) {
	if (graphics_mode == gmCga || graphics_mode == gmHgaHerc) {
		return cga_color;
	} else if (graphics_mode == gmMcgaVga && high_half_mask != 0) {
		return (find_first_pal_row(high_half_mask) << 4) + low_half;
	} else {
		return low_half;
	}
}

// seg009:9CAC
//void __pascal far set_loaded_palette(void far *area1_ptr) {
//	// stub
//}

void load_from_opendats_metadata(int resource_id, const char* extension, FILE** out_fp, data_location* result, byte* checksum, int* size) {
	char image_filename[256];
	FILE* fp = NULL;
	dat_type* pointer;
	*result = data_none;
	// Go through all open DAT files.
	for (pointer = dat_chain_ptr; fp == NULL && pointer != NULL; pointer = pointer->next_dat) {
		if (pointer->handle != NULL) {
			// If it's an actual DAT file:
			fp = pointer->handle;
			dat_table_type* dat_table = pointer->dat_table;
			int i;
			for (i = 0; i < dat_table->res_count; ++i) {
				if (dat_table->entries[i].id == resource_id) {
					break;
				}
			}
			if (i < dat_table->res_count) {
				// found
				*result = data_DAT;
				*size = dat_table->entries[i].size;
				fseek(fp, dat_table->entries[i].offset, SEEK_SET);
				fread(checksum, 1, 1, fp);
			} else {
				// not found
				fp = NULL;
			}
		} else {
			// If it's a directory:
			snprintf(image_filename,sizeof(image_filename),"data/%s/res%d.%s",pointer->filename, resource_id, extension);
			//printf("loading (binary) %s",image_filename);
			fp = fopen(image_filename, "rb");
			if (fp != NULL) {
				*result = data_directory;
				struct stat buf;
				fstat(fileno(fp), &buf);
				*size = buf.st_size;
			}
		}
	}
	*out_fp = fp;
	if (fp == NULL) {
		*result = data_none;
//		printf(" FAILED\n");
		//return NULL;
	}
	//...
}

// seg009:9F34
void __pascal far close_dat(dat_type far *pointer) {
	dat_type** prev = &dat_chain_ptr;
	dat_type* curr = dat_chain_ptr;
	while (curr != NULL) {
		if (curr == pointer) {
			*prev = curr->next_dat;
			if (curr->handle) fclose(curr->handle);
			if (curr->dat_table) free(curr->dat_table);
			free(curr);
			return;
		}
		curr = curr->next_dat;
		prev = &((*prev)->next_dat);
	}
	// stub
}

// seg009:9F80
void far *__pascal load_from_opendats_alloc(int resource, const char* extension, data_location* out_result, int* out_size) {
	// stub
	//printf("id = %d\n",resource);
	data_location result;
	byte checksum;
	int size;
	FILE* fp = NULL;
	load_from_opendats_metadata(resource, extension, &fp, &result, &checksum, &size);
	if (out_result != NULL) *out_result = result;
	if (out_size != NULL) *out_size = size;
	if (result == data_none) return NULL;
	void* area = malloc(size);
	//read(fd, area, size);
	fread(area, size, 1, fp);
	if (result == data_directory) fclose(fp);
	return area;
}

// seg009:A172
int __pascal far load_from_opendats_to_area(int resource,void far *area,int length, const char* extension) {
	// stub
	//return 0;
	data_location result;
	byte checksum;
	int size;
	FILE* fp = NULL;
	load_from_opendats_metadata(resource, extension, &fp, &result, &checksum, &size);
	if (result == data_none) return 0;
	fread(area, MIN(size, length), 1, fp);
	if (result == data_directory) fclose(fp);
	return 0;
}

// SDL-specific implementations

void rect_to_sdlrect(const rect_type* rect, SDL_Rect* sdlrect) {
	sdlrect->x = rect->left;
	sdlrect->y = rect->top;
	sdlrect->w = rect->right - rect->left;
	sdlrect->h = rect->bottom - rect->top;
}

void __pascal far method_1_blit_rect(surface_type near *target_surface,surface_type near *source_surface,const rect_type far *target_rect, const rect_type far *source_rect,int blit) {
	SDL_Rect src_rect;
	rect_to_sdlrect(source_rect, &src_rect);
	SDL_Rect dest_rect;
	rect_to_sdlrect(target_rect, &dest_rect);
	/*
	if (blit == blitters_0_no_transp) {
		SDL_Rect dest_rect2 = dest_rect;
		// SDL_FillRect modifies the rect!
#ifndef USE_ALPHA
		SDL_FillRect(target_surface, &dest_rect2, SDL_MapRGB(current_target_surface->format, 0, 0, 0));
#else
		SDL_FillRect(target_surface, &dest_rect2, SDL_MapRGBA(current_target_surface->format, 0, 0, 0, SDL_ALPHA_TRANSPARENT));
#endif
	}
	*/
	if (blit == blitters_0_no_transp) {
		// Disable transparency.
		if (SDL_SetColorKey(source_surface, 0, 0) != 0) {
			sdlperror("SDL_SetColorKey");
			quit(1);
		}
	} else {
		// Enable transparency.
		if (SDL_SetColorKey(source_surface, SDL_SRCCOLORKEY, 0) != 0) {
			sdlperror("SDL_SetColorKey");
			quit(1);
		}
	}
	if (SDL_BlitSurface(source_surface, &src_rect, target_surface, &dest_rect) != 0) {
		sdlperror("SDL_BlitSurface");
		quit(1);
	}
	if (target_surface == onscreen_surface_) {
		//SDL_UpdateRect(onscreen_surface_, 0, 0, 0, 0); // debug
		SDL_UpdateRects(onscreen_surface_, 1, &dest_rect);
	}
	//SDL_Delay(10);
}

image_type far * __pascal far method_3_blit_mono(image_type far *image,int xpos,int ypos,int blitter,byte color) {
	SDL_Surface* colored_image;
	int w = image->w;
	int h = image->h;
	colored_image = SDL_CreateRGBSurface(SDL_SRCALPHA, w, h, 32, 0xFF, 0xFF<<8, 0xFF<<16, 0xFF<<24);
	//SDL_Rect rect = {0, 0, image->w, image->h};
	if (SDL_SetColorKey(image, SDL_SRCCOLORKEY, 0) != 0) {
		sdlperror("SDL_SetColorKey");
		quit(1);
	}
	if (SDL_BlitSurface(image, NULL, colored_image, NULL) != 0) {
		sdlperror("SDL_BlitSurface");
		quit(1);
	}
	if (SDL_LockSurface(colored_image) != 0) {
		sdlperror("SDL_LockSurface");
		quit(1);
	}
	int y,x;
	rgb_type palette_color = palette[color];
	uint32_t rgb_color = SDL_MapRGB(colored_image->format, palette_color.r<<2, palette_color.g<<2, palette_color.b<<2) & 0xFFFFFF;
	int stride = colored_image->pitch;
	for (y = 0; y < h; ++y) {
		uint32_t* pixel_ptr = (uint32_t*) ((byte*)colored_image->pixels + stride * y);
		for (x = 0; x < w; ++x) {
			// set RGB but leave alpha
			*pixel_ptr = (*pixel_ptr & 0xFF000000) | rgb_color;
			++pixel_ptr;
		}
	}
	SDL_UnlockSurface(colored_image);
	method_6_blit_img_to_scr(colored_image, xpos, ypos, 0x10);
	SDL_FreeSurface(colored_image);
	return image;
}

const rect_type far * __pascal far method_5_rect(const rect_type far *rect,int blit,byte color) {
	SDL_Rect dest_rect;
	rect_to_sdlrect(rect, &dest_rect);
	rgb_type palette_color = palette[color];
#ifndef USE_ALPHA
	uint32_t rgb_color = SDL_MapRGB(current_target_surface->format, palette_color.r<<2, palette_color.g<<2, palette_color.b<<2);
#else
	uint32_t rgb_color = SDL_MapRGBA(current_target_surface->format, palette_color.r<<2, palette_color.g<<2, palette_color.b<<2, color == 0 ? SDL_ALPHA_TRANSPARENT : SDL_ALPHA_OPAQUE);
#endif
	if (SDL_FillRect(current_target_surface, &dest_rect, rgb_color) != 0) {
		sdlperror("SDL_FillRect");
		quit(1);
	}
	if (current_target_surface == onscreen_surface_) {
		//SDL_UpdateRect(onscreen_surface_, 0, 0, 0, 0); // debug
		SDL_UpdateRects(onscreen_surface_, 1, &dest_rect);
	}
	//SDL_Delay(10);
	return rect;
}

void blit_xor(SDL_Surface* target_surface, SDL_Rect* dest_rect, SDL_Surface* image, SDL_Rect* src_rect) {
	if (dest_rect->w != src_rect->w || dest_rect->h != src_rect->h) {
		printf("blit_xor: dest_rect and src_rect have different sizes\n");
		quit(1);
	}
	SDL_Surface* helper_surface = SDL_CreateRGBSurface(0, dest_rect->w, dest_rect->h, 24, 0xFF, 0xFF<<8, 0xFF<<16, 0);
	if (helper_surface == NULL) {
		sdlperror("SDL_CreateRGBSurface");
		quit(1);
	}
	SDL_Surface* image_24 = SDL_ConvertSurface(image, helper_surface->format, 0);
	//SDL_CreateRGBSurface(0, src_rect->w, src_rect->h, 24, 0xFF, 0xFF<<8, 0xFF<<16, 0);
	if (image_24 == NULL) {
		sdlperror("SDL_CreateRGBSurface");
		quit(1);
	}
	SDL_Rect dest_rect2 = *src_rect;
	// Read what is currently where we want to draw the new image.
	if (SDL_BlitSurface(target_surface, dest_rect, helper_surface, &dest_rect2) != 0) {
		sdlperror("SDL_BlitSurface");
		quit(1);
	}
	if (SDL_LockSurface(image_24) != 0) {
		sdlperror("SDL_LockSurface");
		quit(1);
	}
	if (SDL_LockSurface(helper_surface) != 0) {
		sdlperror("SDL_LockSurface");
		quit(1);
	}
	int size = helper_surface->h * helper_surface->pitch;
	int i;
	byte *p_src = (byte*) image_24->pixels;
	byte *p_dest = (byte*) helper_surface->pixels;
/*
	int fd;
	fd = creat("xor_src.raw", O_WRONLY);
	write(fd, p_src, size);
	close(fd);
	fd = creat("xor_dst.raw", O_WRONLY);
	write(fd, p_dest, size);
	close(fd);
*/
	// Xor the old area with the image.
	for (i = 0; i < size; ++i) {
		*p_dest ^= *p_src;
		++p_src; ++p_dest;
	}
	SDL_UnlockSurface(image_24);
	SDL_UnlockSurface(helper_surface);
	// Put the new area in place of the old one.
	if (SDL_BlitSurface(helper_surface, src_rect, target_surface, dest_rect) != 0) {
		sdlperror("SDL_BlitSurface");
		quit(1);
	}
	SDL_FreeSurface(image_24);
	SDL_FreeSurface(helper_surface);
	if (target_surface == onscreen_surface_) {
		//SDL_UpdateRect(onscreen_surface_, 0, 0, 0, 0); // debug
		SDL_UpdateRects(onscreen_surface_, 1, /*&*/dest_rect);
	}
}

image_type far * __pascal far method_6_blit_img_to_scr(image_type far *image,int xpos,int ypos,int blit) {
	if (image == NULL) {
		printf("method_6_blit_img_to_scr: image == NULL\n");
		quit(1);
	}
	if (blit == blitters_9_black) {
		method_3_blit_mono(image, xpos, ypos, blitters_10h_transp, 0);
		return image;
	}
	SDL_Rect src_rect;
	src_rect.x = 0;
	src_rect.y = 0;
	src_rect.w = image->w;
	src_rect.h = image->h;
	SDL_Rect dest_rect;
	dest_rect.x = xpos;
	dest_rect.y = ypos;
	dest_rect.w = image->w;
	dest_rect.h = image->h;
	if (blit == blitters_3_xor) {
		blit_xor(current_target_surface, &dest_rect, image, &src_rect);
		return image;
	}
	/*
	if (blit == blitters_0_no_transp) {
		SDL_Rect dest_rect2 = dest_rect;
		// SDL_FillRect modifies the rect!
#ifndef USE_ALPHA
		SDL_FillRect(current_target_surface, &dest_rect2, SDL_MapRGB(current_target_surface->format, 0, 0, 0));
#else
		SDL_FillRect(current_target_surface, &dest_rect2, SDL_MapRGBA(current_target_surface->format, 0, 0, 0, SDL_ALPHA_TRANSPARENT));
#endif
	}
	*/
	if (blit == blitters_0_no_transp) {
		// Disable transparency.
		if (SDL_SetColorKey(image, 0, 0) != 0) {
			sdlperror("SDL_SetColorKey");
			quit(1);
		}
	} else {
		// Enable transparency.
		if (SDL_SetColorKey(image, SDL_SRCCOLORKEY, 0) != 0) {
			sdlperror("SDL_SetColorKey");
			quit(1);
		}
		if (SDL_SetAlpha(image, image->format->Amask ? SDL_SRCALPHA : 0, 0) != 0) {
			sdlperror("SDL_SetAlpha");
			quit(1);
		}
	}
	/*{ // debug
		SDL_Rect dest_rect2 = dest_rect;
		// SDL_FillRect modifies the rect!
		SDL_FillRect(current_target_surface, &dest_rect2, SDL_MapRGB(current_target_surface->format, 255, 0, 0));
	}*/
	if (SDL_BlitSurface(image, &src_rect, current_target_surface, &dest_rect) != 0) {
		sdlperror("SDL_BlitSurface");
		quit(1);
	}
	if (SDL_SetAlpha(image, 0, 0) != 0) {
		sdlperror("SDL_SetAlpha");
		quit(1);
	}
	if (current_target_surface == onscreen_surface_) {
		//SDL_UpdateRect(onscreen_surface_, 0, 0, 0, 0); // debug
		SDL_UpdateRects(onscreen_surface_, 1, &dest_rect);
	}
	//SDL_Delay(10);
	return image;
}

// seg009:68CC
void far *__pascal method_7_alloc(int size) {
	if (size < 0 || size >= 0xFFFE) {
		return NULL;
	} else {
		return malloc_far(size);
	}
}

// seg009:68E9
void __pascal far method_8_free(void far *pointer) {
	free_far(pointer);
}

#ifndef USE_COMPAT_TIMER
int fps = 60;
SDL_TimerID timer_handles[2] = {0,0};
int timer_stopped[2] = {1,1};
#else
int wait_time[2] = {0,0};
#endif

void remove_timer(int timer_index) {
#ifndef USE_COMPAT_TIMER
	if (timer_handles[timer_index]) {
		if (!SDL_RemoveTimer(timer_handles[timer_index])) {
			printf("timer_handles[%d] = %p\n", timer_index, (void*) timer_handles[timer_index]);
			sdlperror("SDL_RemoveTimer in remove_timer");
			//quit(1);
		}
		timer_handles[timer_index] = NULL;
	}
#endif
}

Uint32 timer_callback(Uint32 interval, void *param) {
	SDL_Event event;
	memset(&event, 0, sizeof(event));
	event.type = SDL_USEREVENT;
	event.user.code = userevent_TIMER;
	event.user.data1 = param;
	int timer_index = (uintptr_t)param;
	remove_timer(timer_index);
	// First remove the timer, then allow the other thread to continue.
	SDL_PushEvent(&event);
#ifndef USE_COMPAT_TIMER
//	remove_timer(timer_index);
	return 0;
#else
	return interval;
#endif
}

void __pascal start_timer(int timer_index, int length) {
	//return; // debug
	// stub
#ifndef USE_COMPAT_TIMER
	if (timer_handles[timer_index]) {
		remove_timer(timer_index);
		timer_handles[timer_index] = NULL;
	}
	timer_stopped[timer_index] = length<=0;
	if (length <= 0) return;
	SDL_TimerID timer = SDL_AddTimer(length*1000/fps, timer_callback, (void*)(uintptr_t)timer_index);
	if (timer == NULL) {
		sdlperror("SDL_AddTimer");
		quit(1);
	}
	timer_handles[timer_index] = timer;
#else
	wait_time[timer_index] = length;
#endif
}

void toggle_fullscreen() {
	//printf("Pressed alt-enter\n");
	Uint32 flags = onscreen_surface_->flags;
	flags ^= SDL_FULLSCREEN;
	// The simplest way to copy a surface:
	surface_type* temp_surface = SDL_ConvertSurface(onscreen_surface_, onscreen_surface_->format, 0);
	if (temp_surface == NULL) {
		sdlperror("SDL_ConvertSurface (toggle_fullscreen)");
		quit(1);
	}
	//surface_type* new_onscreen_surface_ = SDL_SetVideoMode(320, 200, 24, flags);
	surface_type* new_onscreen_surface_ = SDL_SetVideoMode(320, 240, 16, SDL_FULLSCREEN | SDL_SWSURFACE | SDL_DOUBLEBUF);
	if (new_onscreen_surface_ == NULL) {
		sdlperror("SDL_SetVideoMode (toggle_fullscreen)");
		quit(1);
	}
	// Note: the old onscreen_surface_ is invalid at this point.
	if (current_target_surface == onscreen_surface_) {
		// Without this, the game crashes if I switch into fullscreen while the game is paused.
		current_target_surface = new_onscreen_surface_;
	}
	onscreen_surface_ = new_onscreen_surface_;
	int fullscreen = (flags & SDL_FULLSCREEN) != 0;
	/*if (fullscreen) {
		SDL_ShowCursor(SDL_DISABLE);
	} else {
		SDL_ShowCursor(SDL_ENABLE);
	}*/
	// The new surface is empty. Copy the old image over onto it.
	// offscreen_surface is not good instead of temp_surface, because it does not contain the bottom 8 rows (hitpoints).
	SDL_BlitSurface(temp_surface, NULL, onscreen_surface_, NULL);
	SDL_UpdateRect(onscreen_surface_,0,0,0,0);
	SDL_FreeSurface(temp_surface);
}

void idle() {
	// Wait for *one* event and process it, then return.
	// Much like the x86 HLT instruction.
	SDL_Event event;
	if (SDL_WaitEvent(&event) == 0) {
		sdlperror("SDL_WaitEvent");
		quit(1);
	}
	switch (event.type) {
		case SDL_KEYDOWN:
			if ((event.key.keysym.mod & (KMOD_LALT|KMOD_RALT)) && event.key.keysym.sym == SDLK_RETURN) {
				// Alt-Enter: toggle fullscreen mode
				toggle_fullscreen();
			} else {
                if(event.key.keysym.sym == SDLK_RETURN) { // START
                    quit(9);
                }
				last_key = event.key.keysym.unicode;
				if (!last_key) {
					if (event.key.keysym.sym < SDLK_NUMLOCK) {
						last_key = event.key.keysym.scancode << 8;
					}
				}
				key_states[event.key.keysym.sym] = 1;
			}
		break;
		case SDL_KEYUP:
//			key_states[event.key.keysym.sym] = (event.type==SDL_KEYDOWN ? 0x01 : 0x00);
			key_states[event.key.keysym.sym] = 0;
		break;
		case SDL_ACTIVEEVENT:
			// In case the user switches away while holding a key: do as if all keys were released.
			// (DOSBox does the same.)
			if ((event.active.state & SDL_APPINPUTFOCUS) && event.active.gain == 0) {
				memset(key_states, 0, sizeof(key_states));
			}
			// Note: event.active.state can contain multiple flags or'ed.
			// If the game is in full screen, and I switch away (alt-tab) and back, most of the screen will be black, until it is redrawn.
			if ((event.active.state & SDL_APPACTIVE) && event.active.gain == 1) {
				SDL_UpdateRect(onscreen_surface_,0,0,0,0);
			}
		break;
		case SDL_USEREVENT:
			if (event.user.code == userevent_TIMER /*&& event.user.data1 == (void*)timer_index*/) {
#ifndef USE_COMPAT_TIMER
				int timer_index = (uintptr_t)event.user.data1;
				timer_stopped[timer_index] = 1;
				//printf("timer_index = %d\n", timer_index);
				// 2014-08-27: According to the output of the next line, handle is always NULL.
				// 2014-08-28: Except when you interrupt fading of the cutscene.
				//printf("timer_handles[timer_index] = %p\n", timer_handles[timer_index]);
				// 2014-08-27: However, this line will change something: it makes the game too fast. Weird...
				// 2014-08-28: Wait, now it doesn't...
				//timer_handles[timer_index] = NULL;
#else
				int index;
				for (index = 0; index < 2; ++index) {
					if (wait_time[index] > 0) --wait_time[index];
				}
#endif
			} else if (event.user.code == userevent_SOUND) {
				//sound_timer = NULL;
#ifndef USE_MIXER
				//stop_sounds();
#endif
			}
		break;
		case SDL_QUIT:
			quit(0);
		break;
	}
}

word word_1D63A = 1;
// seg009:0EA9
int __pascal do_wait(int timer_index) {
	//return; // debug
	//if (timer_handles[timer_index] == NULL) return;
	/*
	while (! timer_stopped[timer_index]) {
		idle();
		do_paused();
	}
	*/
//	while (! timer_stopped[timer_index]) {
	while (! has_timer_stopped(timer_index)) {
	//do {
		idle();
		int key = do_paused();
		if (key != 0 && (word_1D63A != 0 || key == 0x1B)) return 1;
	} //while (! timer_stopped[timer_index]);
	return 0;
}

void __pascal do_simple_wait(int timer_index) {
//	while (! timer_stopped[timer_index]) {
	while (! has_timer_stopped(timer_index)) {
	//do {
		idle();
	} //while (! timer_stopped[timer_index]);
}

#ifdef USE_COMPAT_TIMER
SDL_TimerID global_timer = NULL;
#endif
// seg009:78E9
void __pascal far init_timer(int frequency) {
#ifndef USE_COMPAT_TIMER
	fps = frequency;
#else
	if (global_timer != NULL) {
		if (!SDL_RemoveTimer(global_timer)) {
			sdlperror("SDL_RemoveTimer");
		}
	}
	global_timer = SDL_AddTimer(1000/frequency, timer_callback, NULL);
	if (global_timer == NULL) {
		sdlperror("SDL_AddTimer");
		quit(1);
	}
#endif
}

// seg009:35F6
void __pascal far set_clip_rect(const rect_type far *rect) {
	SDL_Rect clip_rect;
	rect_to_sdlrect(rect, &clip_rect);
	SDL_SetClipRect(current_target_surface, &clip_rect);
}

// seg009:365C
void __pascal far reset_clip_rect() {
	SDL_SetClipRect(current_target_surface, NULL);
}

// seg009:1983
void __pascal far set_bg_attr(int vga_pal_index,int hc_pal_index) {
	// stub
#ifdef USE_FLASH
	//palette[vga_pal_index] = vga_palette[hc_pal_index];
	if (vga_pal_index == 0) {
		/*
		if (SDL_SetAlpha(offscreen_surface, SDL_SRCALPHA, 0) != 0) {
			sdlperror("SDL_SetAlpha");
			quit(1);
		}
		*/
		// Make the black pixels transparent.
		if (SDL_SetColorKey(offscreen_surface, SDL_SRCCOLORKEY, 0) != 0) {
			sdlperror("SDL_SetColorKey");
			quit(1);
		}
		SDL_Rect rect = {0,0,0,0};
		rect.w = offscreen_surface->w;
		rect.h = offscreen_surface->h;
		rgb_type palette_color = palette[hc_pal_index];
		uint32_t rgb_color = SDL_MapRGB(onscreen_surface_->format, palette_color.r<<2, palette_color.g<<2, palette_color.b<<2) /*& 0xFFFFFF*/;
		//SDL_UpdateRect(onscreen_surface_, 0, 0, 0, 0);
		// First clear the screen with the color of the flash.
		if (SDL_FillRect(onscreen_surface_, &rect, rgb_color) != 0) {
			sdlperror("SDL_FillRect");
			quit(1);
		}
		//SDL_UpdateRect(onscreen_surface_, 0, 0, 0, 0);
		// Then draw the offscreen image onto it.
		if (SDL_BlitSurface(offscreen_surface, &rect, onscreen_surface_, &rect) != 0) {
			sdlperror("SDL_BlitSurface");
			quit(1);
		}
		// And show it!
		SDL_UpdateRect(onscreen_surface_, 0, 0, 0, 0);
		// Give some time to show the flash.
		//SDL_Flip(onscreen_surface_);
		if (hc_pal_index != 0) SDL_Delay(2*1000/60);
		//SDL_Flip(onscreen_surface_);
		/*
		if (SDL_SetAlpha(offscreen_surface, 0, 0) != 0) {
			sdlperror("SDL_SetAlpha");
			quit(1);
		}
		*/
		if (SDL_SetColorKey(offscreen_surface, 0, 0) != 0) {
			sdlperror("SDL_SetColorKey");
			quit(1);
		}
	}
#endif // USE_FLASH
}

// seg009:07EB
rect_type far *__pascal offset4_rect_add(rect_type far *dest,const rect_type far *source,int d_left,int d_top,int d_right,int d_bottom) {
	*dest = *source;
	dest->left += d_left;
	dest->top += d_top;
	dest->right += d_right;
	dest->bottom += d_bottom;
	return dest;
}

// seg009:3AA5
rect_type far *__pascal offset2_rect(rect_type far *dest,const rect_type far *source,int delta_x,int delta_y) {
	dest->top    = source->top    + delta_y;
	dest->left   = source->left   + delta_x;
	dest->bottom = source->bottom + delta_y;
	dest->right  = source->right  + delta_x;
	return dest;
}

#ifdef USE_FADE
// seg009:19EF
void __pascal far fade_in_2(surface_type near *source_surface,int which_rows) {
	palette_fade_type far* palette_buffer;
	if (graphics_mode == gmMcgaVga) {
		palette_buffer = make_pal_buffer_fadein(source_surface, which_rows, 2);
		while (fade_in_frame(palette_buffer) == 0) {
			pop_wait(1, 0); // modified
		}
		pal_restore_free_fadein(palette_buffer);
	} else {
		// ...
	}
}

// seg009:1A51
palette_fade_type far *__pascal make_pal_buffer_fadein(surface_type *source_surface,int which_rows,int wait_time) {
	palette_fade_type far* palette_buffer;
	word curr_row;
	word var_8;
	word curr_row_mask;
	palette_buffer = (palette_fade_type*) malloc_far(sizeof(palette_fade_type));
	palette_buffer->which_rows = which_rows;
	palette_buffer->wait_time = wait_time;
	palette_buffer->fade_pos = 0x40;
	palette_buffer->proc_restore_free = &pal_restore_free_fadein;
	palette_buffer->proc_fade_frame = &fade_in_frame;
	read_palette_256(palette_buffer->original_pal);
	memcpy_far(palette_buffer->faded_pal, palette_buffer->original_pal, sizeof(palette_buffer->faded_pal));
	var_8 = 0;
	for (curr_row = 0, curr_row_mask = 1; curr_row < 0x10; ++curr_row, curr_row_mask<<=1) {
		if (which_rows & curr_row_mask) {
			memset_far(palette_buffer->faded_pal + (curr_row<<4), 0, sizeof(rgb_type[0x10]));
			set_pal_arr(curr_row<<4, 0x10, NULL, (var_8++&3)==0);
		}
	}
	//method_1_blit_rect(onscreen_surface_, source_surface, &screen_rect, &screen_rect, 0);
	// for RGB
	//method_5_rect(&screen_rect, 0, 0);
	return palette_buffer;
}

// seg009:1B64
void __pascal far pal_restore_free_fadein(palette_fade_type far *palette_buffer) {
	set_pal_256(palette_buffer->original_pal);
	free_far(palette_buffer);
	// for RGB
	method_1_blit_rect(onscreen_surface_, offscreen_surface, &screen_rect, &screen_rect, 0);
}

// seg009:1B88
int __pascal far fade_in_frame(palette_fade_type far *palette_buffer) {
	rgb_type* faded_pal_ptr;
	word start;
	word column;
	rgb_type* original_pal_ptr;
	word current_row_mask;
//	void* var_12;
	/**/start_timer(1, palette_buffer->wait_time); // too slow?
	//printf("start ticks = %u\n",SDL_GetTicks());
	--palette_buffer->fade_pos;
	for (start=0,current_row_mask=1; start<0x100; start+=0x10, current_row_mask<<=1) {
		if (palette_buffer->which_rows & current_row_mask) {
			//var_12 = palette_buffer->
			original_pal_ptr = palette_buffer->original_pal + start;
			faded_pal_ptr = palette_buffer->faded_pal + start;
			for (column = 0; column<0x10; ++column) {
				if (original_pal_ptr[column].r > palette_buffer->fade_pos) {
					++faded_pal_ptr[column].r;
				}
				if (original_pal_ptr[column].g > palette_buffer->fade_pos) {
					++faded_pal_ptr[column].g;
				}
				if (original_pal_ptr[column].b > palette_buffer->fade_pos) {
					++faded_pal_ptr[column].b;
				}
			}
		}
	}
	column = 0;
	for (start = 0, current_row_mask = 1; start<0x100; start+=0x10, current_row_mask<<=1) {
		if (palette_buffer->which_rows & current_row_mask) {
			set_pal_arr(start, 0x10, palette_buffer->faded_pal + start, (column++&3)==0);
		}
	}
	
	int h = offscreen_surface->h;
	if (SDL_LockSurface(onscreen_surface_) != 0) {
		sdlperror("SDL_LockSurface");
		quit(1);
	}
	if (SDL_LockSurface(offscreen_surface) != 0) {
		sdlperror("SDL_LockSurface");
		quit(1);
	}
	int y,x;
	int on_stride = onscreen_surface_->pitch;
	int off_stride = offscreen_surface->pitch;
	int fade_pos = palette_buffer->fade_pos;
	for (y = 0; y < h; ++y) {
		byte* on_pixel_ptr = (byte*)onscreen_surface_->pixels + on_stride * y;
		byte* off_pixel_ptr = (byte*)offscreen_surface->pixels + off_stride * y;
		for (x = 0; x < on_stride; ++x) {
			//if (*off_pixel_ptr > palette_buffer->fade_pos) *pixel_ptr += 4;
			int v = *off_pixel_ptr - fade_pos*4;
			if (v<0) v=0;
			*on_pixel_ptr = v;
			++on_pixel_ptr; ++off_pixel_ptr;
		}
	}
	SDL_UnlockSurface(onscreen_surface_);
	SDL_UnlockSurface(offscreen_surface);

	SDL_UpdateRect(onscreen_surface_, 0, 0, 0, 0); // debug
		
//	/**/do_simple_wait(1); // too slow?
	do_wait(1);
	//printf("end ticks = %u\n",SDL_GetTicks());
	return palette_buffer->fade_pos == 0;
}

// seg009:1CC9
void __pascal far fade_out_2(int rows) {
	palette_fade_type far *palette_buffer;
	if (graphics_mode == gmMcgaVga) {
		palette_buffer = make_pal_buffer_fadeout(rows, 2);
		while (fade_out_frame(palette_buffer) == 0) {
			pop_wait(1, 0); // modified
		}
		pal_restore_free_fadeout(palette_buffer);
	} else {
		// ...
	}
}

// seg009:1D28
palette_fade_type far *__pascal make_pal_buffer_fadeout(int which_rows,int wait_time) {
	palette_fade_type far *palette_buffer;
	palette_buffer = (palette_fade_type*) malloc_far(sizeof(palette_fade_type));
	palette_buffer->which_rows = which_rows;
	palette_buffer->wait_time = wait_time;
	palette_buffer->fade_pos = 00; // modified
	palette_buffer->proc_restore_free = &pal_restore_free_fadeout;
	palette_buffer->proc_fade_frame = &fade_out_frame;
	read_palette_256(palette_buffer->original_pal);
	memcpy_far(palette_buffer->faded_pal, palette_buffer->original_pal, sizeof(palette_buffer->faded_pal));
	// for RGB
	method_1_blit_rect(onscreen_surface_, offscreen_surface, &screen_rect, &screen_rect, 0);
	return palette_buffer;
}

// seg009:1DAF
void __pascal far pal_restore_free_fadeout(palette_fade_type far *palette_buffer) {
	surface_type* surface;
	surface = current_target_surface;
	current_target_surface = onscreen_surface_;
	draw_rect(&screen_rect, 0);
	current_target_surface = surface;
	set_pal_256(palette_buffer->original_pal);
	free_far(palette_buffer);
	// for RGB
	method_5_rect(&screen_rect, 0, 0);
}

// seg009:1DF7
int __pascal far fade_out_frame(palette_fade_type far *palette_buffer) {
	rgb_type* faded_pal_ptr;
	word start;
	word var_8;
	word column;
	word current_row_mask;
	byte* curr_color_ptr;
	var_8 = 1;
	++palette_buffer->fade_pos; // modified
	/**/start_timer(1, palette_buffer->wait_time); // too slow?
	for (start=0,current_row_mask=1; start<0x100; start+=0x10, current_row_mask<<=1) {
		if (palette_buffer->which_rows & current_row_mask) {
			//var_12 = palette_buffer->
			//original_pal_ptr = palette_buffer->original_pal + start;
			faded_pal_ptr = palette_buffer->faded_pal + start;
			for (column = 0; column<0x10; ++column) {
				curr_color_ptr = &faded_pal_ptr[column].r;
				if (*curr_color_ptr != 0) {
					--*curr_color_ptr;
					var_8 = 0;
				}
				curr_color_ptr = &faded_pal_ptr[column].g;
				if (*curr_color_ptr != 0) {
					--*curr_color_ptr;
					var_8 = 0;
				}
				curr_color_ptr = &faded_pal_ptr[column].b;
				if (*curr_color_ptr != 0) {
					--*curr_color_ptr;
					var_8 = 0;
				}
			}
		}
	}
	column = 0;
	for (start = 0, current_row_mask = 1; start<0x100; start+=0x10, current_row_mask<<=1) {
		if (palette_buffer->which_rows & current_row_mask) {
			set_pal_arr(start, 0x10, palette_buffer->faded_pal + start, (column++&3)==0);
		}
	}
	
	int h = offscreen_surface->h;
	if (SDL_LockSurface(onscreen_surface_) != 0) {
		sdlperror("SDL_LockSurface");
		quit(1);
	}
	if (SDL_LockSurface(offscreen_surface) != 0) {
		sdlperror("SDL_LockSurface");
		quit(1);
	}
	int y,x;
	int on_stride = onscreen_surface_->pitch;
	int off_stride = offscreen_surface->pitch;
	int fade_pos = palette_buffer->fade_pos;
	for (y = 0; y < h; ++y) {
		byte* on_pixel_ptr = (byte*)onscreen_surface_->pixels + on_stride * y;
		byte* off_pixel_ptr = (byte*)offscreen_surface->pixels + off_stride * y;
		for (x = 0; x < on_stride; ++x) {
			//if (*pixel_ptr >= 4) *pixel_ptr -= 4;
			int v = *off_pixel_ptr - fade_pos*4;
			if (v<0) v=0;
			*on_pixel_ptr = v;
			++on_pixel_ptr; ++off_pixel_ptr;
		}
	}
	SDL_UnlockSurface(onscreen_surface_);
	SDL_UnlockSurface(offscreen_surface);

	SDL_UpdateRect(onscreen_surface_, 0, 0, 0, 0); // debug
	
//	/**/do_simple_wait(1); // too slow?
	do_wait(1);
	return var_8;
}

// seg009:1F28
void __pascal far read_palette_256(rgb_type far *target) {
	int i;
	for (i = 0; i < 256; ++i) {
		target[i] = palette[i];
	}
}

// seg009:1F5E
void __pascal far set_pal_256(rgb_type far *source) {
	int i;
	for (i = 0; i < 256; ++i) {
		palette[i] = source[i];
	}
}
#endif // USE_FADE

void set_chtab_palette(chtab_type* chtab, byte* colors, int n_colors) {
	if (chtab != NULL) {
		SDL_Color* scolors = (SDL_Color*) malloc(n_colors*sizeof(SDL_Color));
		int i;
		//printf("scolors\n",i);
		for (i = 0; i < n_colors; ++i) {
			//printf("i=%d\n",i);
			scolors[i].r = *colors << 2; ++colors;
			scolors[i].g = *colors << 2; ++colors;
			scolors[i].b = *colors << 2; ++colors;
		}
		//printf("setcolors\n",i);
		for (i = 0; i < chtab->n_images; ++i) {
			//printf("i=%d\n",i);
			if (chtab->pointers[i] != NULL) {
				//fprintf(stderr, "i=%d, BitsPerPixel=%d, palette=%p\n", i, chtab->pointers[i]->format->BitsPerPixel, chtab->pointers[i]->format->palette); // debug
				if (SDL_SetColors(chtab->pointers[i], scolors, 0, n_colors) != 1) {
					sdlperror("SDL_SetColors");
					quit(1);
				}
			}
		}
		free(scolors);
	}
}

int has_timer_stopped(int index) {
#ifdef USE_COMPAT_TIMER
	return wait_time[index] == 0;
#else
	return timer_stopped[index];
#endif
}
