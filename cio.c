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
 * @file cio.c
 * @brief memory manager and operations for compressing JPEG IO.
 * @author Yu, Le <yeolar@gmail.com>
 * @version 1.0.0
 * @date 2009-11-23 10:44:30
 */

#include <string.h>
#include "cjpeg.h"
#include "cio.h"


/*
 * flush input and output of compress IO.
 */

bool
flush_cin_buffer(void *cio)
{
    mem_mgr *in = ((compress_io *) cio)->in;
    size_t len = in->end - in->set;
    memset(in->set, 0, len);
    if (fread(in->set, sizeof(UINT8), len, in->fp) != len)
        return false;
#ifdef REVERSED
    fseek(in->fp, -len * 2, SEEK_CUR);
#endif
    in->pos = in->set;
    return true;
}

bool
flush_cout_buffer(void *cio)
{
    mem_mgr *out = ((compress_io *) cio)->out;
    size_t len = out->pos - out->set;
    if (fwrite(out->set, sizeof(UINT8), len, out->fp) != len)
        return false;
    memset(out->set, 0, len);
    out->pos = out->set;
    return true;
}


/*
 * init memory manager.
 */

void
init_mem(compress_io *cio,
         FILE *in_fp, int in_size, FILE *out_fp, int out_size)
{
    cio->in = (mem_mgr *) malloc(sizeof(mem_mgr));
    if (!cio->in)
        err_exit(BUFFER_ALLOC_ERR);
    cio->in->set = (UINT8 *) malloc(sizeof(UINT8) * in_size);
    if (!cio->in->set)
        err_exit(BUFFER_ALLOC_ERR);
    cio->in->pos = cio->in->set;
    cio->in->end = cio->in->set + in_size;
    cio->in->flush_buffer = flush_cin_buffer;
    cio->in->fp = in_fp;

    cio->out = (mem_mgr *) malloc(sizeof(mem_mgr));
    if (!cio->out)
        err_exit(BUFFER_ALLOC_ERR);
    cio->out->set = (UINT8 *) malloc(sizeof(UINT8) * out_size);
    if (!cio->out->set)
        err_exit(BUFFER_ALLOC_ERR);
    cio->out->pos = cio->out->set;
    cio->out->end = cio->out->set + out_size;
    cio->out->flush_buffer = flush_cout_buffer;
    cio->out->fp = out_fp;

    cio->temp_bits.len = 0;
    cio->temp_bits.val = 0;
}

void
free_mem(compress_io *cio)
{
    fflush(cio->out->fp);
    free(cio->in->set);
    free(cio->out->set);
    free(cio->in);
    free(cio->out);
}


/*
 * write operations.
 */

void
write_byte(compress_io *cio, UINT8 val)
{
    mem_mgr *out = cio->out;
    *(out->pos)++ = val & 0xFF;
    if (out->pos == out->end) {
        if (!(out->flush_buffer)(cio))
            err_exit(BUFFER_WRITE_ERR);
    }
}

void
write_word(compress_io *cio, UINT16 val)
{
    write_byte(cio, (val >> 8) & 0xFF);
    write_byte(cio, val & 0xFF);
}

void
write_marker(compress_io *cio, JPEG_MARKER mark)
{
    write_byte(cio, 0xFF);
    write_byte(cio, (int) mark);
}

void
write_bits(compress_io *cio, BITS bits)
{
    BITS *temp = &(cio->temp_bits);
    UINT16 word;
    UINT8 byte1, byte2;
    int len = bits.len + temp->len - 16;
    if (len >= 0) {
        word = temp->val | bits.val >> len;
        byte1 = word >> 8;
        write_byte(cio, byte1);
        if (byte1 == 0xFF)
            write_byte(cio, 0);
        byte2 = word & 0xFF;
        write_byte(cio, byte2);
        if (byte2 == 0xFF)
            write_byte(cio, 0);
        temp->len = len;
        temp->val = bits.val << (16 - len);
    }
    else {
        temp->len = 16 + len;
        temp->val |= bits.val << -len;
    }
}

void
write_align_bits(compress_io *cio)
{
    BITS *temp = &(cio->temp_bits);
    BITS align_bits;
    align_bits.len = 8 - temp->len % 8;
    align_bits.val = (UINT16) ~0x0 >> temp->len % 8;
    UINT8 byte;
    write_bits(cio, align_bits);
    if (temp->len == 8) {
        byte = temp->val >> 8;
        write_byte(cio, byte);
        if (byte == 0xFF)
            write_byte(cio, 0);
    }
}

