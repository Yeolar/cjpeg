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
 * @file cio.h
 * @brief memory manager and operations for compressing JPEG IO.
 * @author Yu, Le <yeolar@gmail.com>
 * @version 1.0.0
 * @date 2009-11-23 10:53:20
 */

#ifndef __CIO_H
#define __CIO_H

typedef bool (*CIO_METHOD) (void *);

typedef struct {
    UINT8 *set;
    UINT8 *pos;
    UINT8 *end;
    CIO_METHOD flush_buffer;
    FILE *fp;
} mem_mgr;

typedef struct {
    mem_mgr *in;
    mem_mgr *out;
    BITS temp_bits;
} compress_io;


bool flush_cin_buffer(void *cio);
bool flush_cout_buffer(void *cio);

void init_mem(compress_io *cio,
              FILE *in_fp, int in_size, FILE *out_fp, int out_size);
void free_mem(compress_io *cio);

void write_byte(compress_io *cio, UINT8 val);
void write_word(compress_io *cio, UINT16 val);
void write_marker(compress_io *cio, JPEG_MARKER mark);
void write_bits(compress_io *cio, BITS bits);
void write_align_bits(compress_io *cio);

#endif /* __CIO_H */

