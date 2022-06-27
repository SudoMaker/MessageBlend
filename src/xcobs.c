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

#include "xcobs.h"

size_t xcobs_encode(const void *input_buf, size_t input_len, uint8_t *output_buf) {
	uint8_t *encode = output_buf; // Encoded byte pointer
	uint8_t *codep = encode++; // Output code pointer
	uint8_t code = 1; // Code value

	for (const uint8_t *byte = (const uint8_t *)input_buf; input_len--; ++byte) {
		if (*byte) // Byte not zero, write it
			*encode++ = *byte, ++code;

		if (!*byte || code == 0xff) { // Input is zero or block completed, restart
			*codep = code, code = 1, codep = encode;
			if (!*byte || input_len)
				++encode;
		}
	}
	*codep = code; // Write final code value

	size_t enc_len = encode - output_buf;
	output_buf[enc_len] = 0;

	return enc_len + 1;
}


size_t xcobs_encode_multi(const void **input_bufs, const size_t *input_lens, size_t nr_inputs, uint8_t *output_buf) {
	uint8_t *encode = output_buf; // Encoded byte pointer
	uint8_t *codep = encode++; // Output code pointer
	uint8_t code = 1; // Code value

	for (size_t i=0; i<nr_inputs; i++) {
		const void *input_buf = input_bufs[i];
		size_t input_len = input_lens[i];

		for (const uint8_t *byte = (const uint8_t *) input_buf; input_len--; ++byte) {
			if (*byte) // Byte not zero, write it
				*encode++ = *byte, ++code;

			if (!*byte || code == 0xff) { // Input is zero or block completed, restart
				*codep = code, code = 1, codep = encode;
				if (!*byte || input_len)
					++encode;
			}
		}
	}

	*codep = code; // Write final code value

	size_t enc_len = encode - output_buf;
	output_buf[enc_len] = 0;

	return enc_len + 1;
}


void xcobs_decoder_init(xcobs_decode_ctx_t *ctx) {
	ctx->code = 255;
	ctx->block = 0;
}


int xcobs_decode(xcobs_decode_ctx_t *ctx) {
	size_t avail_in_prev = ctx->avail_in;
	size_t avail_out_prev = ctx->avail_out;

	int rc = XCOBS_DECODE_OK;

	if (ctx->avail_in && ctx->avail_out) {
		do {
			uint8_t byte = *ctx->buf_in++;
			ctx->avail_in--;

			if (byte == 0) {
				rc = XCOBS_DECODE_EOF;
				break;
			}

			if (ctx->block) {
				ctx->avail_out--;
				*ctx->buf_out++ = byte;
			} else {
				if (ctx->code != 255) {
					*ctx->buf_out++ = 0;
					ctx->avail_out--;
				}

				ctx->block = ctx->code = byte;
			}

			ctx->block--;
		} while (ctx->avail_in && ctx->avail_out);
	} else {
		rc = XCOBS_DECODE_EAGAIN;
	}

	ctx->drained_out = avail_out_prev - ctx->avail_out;
	ctx->drained_in = avail_in_prev - ctx->avail_in;

	return rc;
}
