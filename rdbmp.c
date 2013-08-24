/* Copyright (C) 2009 - Yu, Le <yeolar@gmail.com>
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 * 
 */

/** 
 * @file rdbmp.c
 * @brief routine for reading BMP file.
 * @author Yu, Le <yeolar@gmail.com>
 * @version 1.0.0
 * @date 2009-11-23 11:02:02
 */

#include "cjpeg.h"

UINT32
extract_uint(const UINT8 *dataptr, UINT32 start, UINT32 len)
{
    UINT32 uint = 0;
    if (len <= 0 || len > 4)
        return uint;
    if (len > 0)
        uint += dataptr[start];
    if (len > 1)
        uint += dataptr[start+1] * 1 << 8;
    if (len > 2)
        uint += dataptr[start+2] * 1 << 16;
    if (len > 3)
        uint += dataptr[start+3] * 1 << 24;
    return uint;
}

int
get_file_size(FILE *bmp_fp)
{
    fpos_t fpos;
    int len;
    fgetpos(bmp_fp, &fpos);
    fseek(bmp_fp, 0, SEEK_END);
    len = ftell(bmp_fp);
    fsetpos(bmp_fp, &fpos);
    return len;
}

void
read_bmp(FILE *bmp_fp, bmp_info *binfo)
{
    size_t len = BMP_HEAD_LEN;
    UINT8 bmp_head[len];
    if (fread(bmp_head, sizeof (UINT8), len, bmp_fp) != len)
        err_exit(FILE_READ_ERR);

    binfo->size     = extract_uint(bmp_head, 2, 4);
    binfo->offset   = extract_uint(bmp_head, 10, 4);
    binfo->width    = extract_uint(bmp_head, 18, 4);
    binfo->height   = extract_uint(bmp_head, 22, 4);
    binfo->bitppx   = extract_uint(bmp_head, 28, 2);
    binfo->datasize = extract_uint(bmp_head, 34, 4);
    if (binfo->datasize == 0)   /* data size not included in some BMP */
        binfo->datasize = get_file_size(bmp_fp) - binfo->offset;
}

