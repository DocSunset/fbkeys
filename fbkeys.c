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

#define _GNU_SOURCE
#include <unistd.h>
#include <getopt.h>
#include <errno.h>
#include <stddef.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/fb.h>
#include <sys/types.h>
#include <dirent.h>
#include <linux/input.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
const char * help =
"\n\
fbkeys: a simple literate framebuffer softkeyboard -- version %s\n\
usage: %s [-h] [-d inputdevice] [-f font]\n\
options:\n\
  -h  print this help text\n\
  -d  path to the touchscreen device\n\
      if none is given, fbkeys will use the first available device with\n\
      absolute coordinate axes.\n\
  -f  path to the font to use to render the keys\n\
      defaults to: '/usr/share/fonts/ttf-dejavu/DejaVuSans.ttf'\n\
\n";
char * font = "/usr/share/fonts/ttf-dejavu/DejaVuSans.ttf";
struct
{
    int file;
    unsigned int width;       /* in pixels */
    unsigned int height;      /* in pixels */
    unsigned int line_length; /* in bytes  */
} framebuffer;
struct
{
    int width;
    int height;
    int row_height;
    int touch_row_height;
} keyboard;
struct
{
    char * path;
    int file;
} touchscreen;
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
        touchscreen.path = NULL;
        while ((c = getopt(argc, argv, "d:f:r:h")) != (char) -1) 
        {
            switch (c) 
            {
            case 'd':
                touchscreen.path = optarg;
                break;
            case 'f':
                font = optarg;
                break;
            case 'h':
                printf(help, VERSION, argv[0]);
                exit(0);
            case '?':
                fprintf(stderr, "unrecognized option -%c\n", optopt);
                break;
            }
        }

        if (touchscreen.path == NULL)
        {
            DIR * dev_input_dir = opendir("/dev/input"); /* (1) */
            struct dirent *dptr;
            exit_fail_if (dev_input_dir == NULL, "error opening '/dev/input' directory");
            touchscreen.file = -1;

            while ((dptr = readdir(dev_input_dir)) != NULL) /* (2.a) */
            {
                touchscreen.file = openat(dirfd(dev_input_dir), dptr->d_name, O_RDONLY); /* (2.b) */
                if (touchscreen.file != -1)
                {
                    int capabilities;
                    int io_ret = ioctl(touchscreen.file, 
                            EVIOCGBIT(0, sizeof(capabilities)), 
                            &capabilities);
                    if (io_ret != -1 && capabilities >> EV_ABS & 1) break; /* (3) */
                    /* otherwise */
                    close(touchscreen.file);
                    touchscreen.file = -1;
                }
            }
            exit_fail_if (touchscreen.file == -1, 
                    "error: no touchscreen device found in /dev/input\n");
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
    keyboard.width = framebuffer.width;
    keyboard.height = framebuffer.height / 3;
    keyboard.row_height = keyboard.height / 5;
    keyboard.touch_row_height = keyboard.row_height * 0x10000 / framebuffer.height;

    debug_printf("keyboard width:       %d\n", keyboard.width);
    debug_printf("keyboard height:      %d\n", keyboard.height);
    debug_printf("keyboard row height:  %d\n", keyboard.row_height);

    while(!done)
    {
    }

    return 0;
}
