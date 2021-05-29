/*
 * fbkeys.c : a simple literate framebuffer softkeyboard
 *
 * Copyright 2021 Travis West <travis@traviswest.ca>

 * Initially based on fbkeyboard.c:
 *     Copyright 2017 Julian Winkler <julia.winkler1@web.de>
 *     Copyright 2020 Ferenc Bakonyi <bakonyi.ferenc@gmail.com>

 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <unistd.h>
#include <getopt.h>
#include <errno.h>
#include <stddef.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/fb.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
const char * help =
"\n\
fbkeys: a simple literate framebuffer softkeyboard -- version %s\n\
usage: %s [-h] [-d inputdevice] [-f font] [-r rotation]\n\
options:\n\
  -h  print this help text\n\
  -d  path to the touchscreen device\n\
      if none is given, fbkeys will use the first available device with\n\
      absolute coordinate axes.\n\
  -f  path to the font to use to render the keys\n\
      defaults to: '/usr/share/fonts/ttf-dejavu/DejaVuSans.ttf'\n\
  -r  an integer representing the rotation of the screen\n\
      defaults to no rotation\n\
\n";
char * device = NULL;
char * font = "/usr/share/fonts/ttf-dejavu/DejaVuSans.ttf";
int rotate = -1; 
struct
{
    int file;
    unsigned int width;       /* in pixels */
    unsigned int height;      /* in pixels */
    unsigned int line_length; /* in bytes  */
} framebuffer;
struct
{
    int landscape;
    int width;
    int row_height;
    int touch_row_height;
    int line_length;
    char * bitmap;
    size_t bitmap_size;
} keyboard;
int done = 0;
void exit_fail_if(int condition, char * message, ...)
{
    va_list args;
    if (!condition) return;
    va_start(args, message);
    vfprintf(stderr, message, args);
    va_end(args);
    exit(EXIT_FAILURE);
}

void debug_printf(char * message, ...)
{
#   ifdef DEBUG
    va_list args;
    va_start(args, message);
    vfprintf(stderr, message, args);
    va_end(args);
#   endif
}

int main(int argc, char ** argv)
{
    {
        char c;
        char * p = NULL;
        while ((c = getopt(argc, argv, "d:f:r:h")) != (char) -1) 
        {
            switch (c) 
            {
            case 'd':
                device = optarg;
                break;
            case 'f':
                font = optarg;
                break;
            case 'r':
                errno = 0;
                rotate = strtol(optarg, &p, 10) % 4;
                exit_fail_if (errno != 0 || p == optarg || p == NULL || *p != '\0',
                        "error: invalid numeric value for -r option, '%s'\n",
                        optarg);
            case 'h':
                printf(help, VERSION, argv[0]);
                exit(0);
            case '?':
                fprintf(stderr, "unrecognized option -%c\n", optopt);
                break;
            }
        }

        if (device == NULL)
        {
        }

        if (rotate == -1)
        {
            char r;
            int f = open("/sys/class/graphics/fbcon/rotate", O_RDONLY);
            exit_fail_if (f == -1, "error opening fbcon/rotate");
            exit_fail_if (read(f, &r, 1) != 1, "error reading fbcon/rotate");
            exit_fail_if (!isdigit(r), 
                    "error: reading from fbcom/rotate '%c' is not a digit???", r);
            rotate = (r - '0') % 4;
        }
    }
    {
        struct fb_var_screeninfo vinfo;
        struct fb_fix_screeninfo finfo;

        framebuffer.file = open("/dev/fb0", O_RDWR);
        exit_fail_if (framebuffer.file == -1,
                "error opening framebuffer device /dev/fb0");
        exit_fail_if (ioctl(framebuffer.file, FBIOGET_FSCREENINFO, &finfo) == -1,
                "error reading fixed framebuffer information");
        exit_fail_if (ioctl(framebuffer.file, FBIOGET_VSCREENINFO, &vinfo) == -1,
                "error reading fixed framebuffer information");

        framebuffer.width  = vinfo.xres;
        framebuffer.height = vinfo.yres;
        framebuffer.line_length = finfo.line_length;
    }
    debug_printf("framebuffer width:       %d\n", framebuffer.width);
    debug_printf("framebuffer height:      %d\n", framebuffer.height);
    debug_printf("framebuffer line length: %d\n", framebuffer.line_length);
    switch(rotate)
    {
        case FB_ROTATE_UR:
        case FB_ROTATE_UD:
            keyboard.landscape = framebuffer.height < framebuffer.width;
            keyboard.width = framebuffer.width;
            keyboard.row_height = framebuffer.height / (keyboard.landscape ? 2 : 3) / 5;
            keyboard.touch_row_height = keyboard.row_height * 0x10000 / framebuffer.height;
            keyboard.line_length = framebuffer.line_length;
            keyboard.bitmap_size = framebuffer.line_length * (keyboard.row_height * 5 + 1);
            break;

        case FB_ROTATE_CW:
        case FB_ROTATE_CCW:
        	keyboard.landscape = framebuffer.height > framebuffer.width;
        	keyboard.width = framebuffer.height;
        	keyboard.row_height = framebuffer.width / (keyboard.landscape ? 2 : 3) / 5;
        	keyboard.touch_row_height = keyboard.row_height * 0x10000 / framebuffer.width;
        	keyboard.line_length = keyboard.row_height * 5 * 4;
        	keyboard.bitmap_size = keyboard.width * 4 * (keyboard.row_height * 5 + 1);
        	break;
    }
    keyboard.bitmap = malloc(keyboard.bitmap_size);
    exit_fail_if(keyboard.bitmap == NULL, 
            "error: failed to allocate drawing bitmap");

    debug_printf("keyboard landscape:   %d\n", keyboard.landscape);
    debug_printf("keyboard width:       %d\n", keyboard.width);
    debug_printf("keyboard row height:  %d\n", keyboard.row_height);
    debug_printf("keyboard line length: %d\n", keyboard.line_length);
    debug_printf("keyboard bitmap size: %d\n", keyboard.bitmap_size);

    {
    size_t scanline;
    memset(keyboard.bitmap, ~0, keyboard.bitmap_size);
    for(scanline = 0; scanline < keyboard.width; ++scanline)
    {
        lseek(framebuffer.file, scanline * framebuffer.line_length, SEEK_SET);
        write(framebuffer.file, keyboard.bitmap + keyboard.line_length * scanline, keyboard.line_length);
    }
    }

    while(!done)
    {
    }

    return 0;
}
