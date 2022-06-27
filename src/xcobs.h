/*
    This file is part of MessageBlend.
    Copyright (C) 2022 Reimu NotMoe <reimu@sudomaker.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as
    published by the Free Software Foundation, either version 3 of the
    License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#pragma once

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	uint8_t *buf_in, *buf_out;
	size_t avail_in, avail_out;
	size_t drained_in, drained_out;

	uint8_t code, block;
} xcobs_decode_ctx_t;

enum {
	XCOBS_DECODE_OK = 0,
	XCOBS_DECODE_EOF = 1,
	XCOBS_DECODE_EAGAIN = 2,
};

#define xcobs_encoded_len(x)		((x) + ((x) / 253 + 1) + 2)

extern size_t xcobs_encode(const void *input_buf, size_t input_len, uint8_t *output_buf);
extern size_t xcobs_encode_multi(const void **input_bufs, const size_t *input_lens, size_t nr_inputs, uint8_t *output_buf);
extern void xcobs_decoder_init(xcobs_decode_ctx_t *ctx);
extern int xcobs_decode(xcobs_decode_ctx_t *ctx);

#ifdef __cplusplus
}
#endif
