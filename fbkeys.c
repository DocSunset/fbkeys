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

#include <stddef.h> /* size_t */
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/fb.h>
#include <stdlib.h>
#include <stdarg.h>
struct
{
    int file;
    unsigned int width;       /* in pixels */
    unsigned int height;      /* in pixels */
    unsigned int line_length; /* in bytes  */
    char * buffer;
    size_t buffer_length;
} framebuffer;
int done = 0;
void exit_fail_if(int condition, char * message, ...)
{
    if (!condition) return;
    va_list args;
    va_start(args, message);
    vfprintf(stderr, message, args);
    va_end(args);
    exit(EXIT_FAILURE);
}

int main(int argc, char ** argv)
{
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

    while(!done)
    {
    }

    return 0;
}
