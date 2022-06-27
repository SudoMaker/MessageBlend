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

#include "msgblend.h"

#ifdef MSGBLEND_DEBUG
#include <stdio.h>
#else
#define printf
#endif

void msgblend_receiver_init(msgblend_receiver_ctx_t *ctx, uint8_t *rx_buf, size_t rx_buf_len, msgblend_receive_callback_t rx_callback, void *userp) {
	{
		ctx->xcobs_dec_ctx.buf_out = ctx->cobs_dec_buf = rx_buf;
		ctx->xcobs_dec_ctx.avail_out = ctx->cobs_dec_buf_len = rx_buf_len;

		xcobs_decoder_init(&ctx->xcobs_dec_ctx);
	}

	ctx->receive_callback = rx_callback;
	ctx->userp = userp;
	ctx->cur_chan = -1;
}

static void msgblend_receiver_parse_decoded(msgblend_receiver_ctx_t *ctx) {
	size_t decode_len = ctx->cobs_dec_buf_len - ctx->xcobs_dec_ctx.avail_out;

	if (ctx->cur_chan != -1) { // Channel ID already resolved
		ctx->receive_callback((uint16_t)ctx->cur_chan, ctx->cobs_dec_buf, decode_len, ctx->userp);
	} else { // Channel ID not resolved
		if (decode_len >= 2) {
			uint16_t w = ctx->cobs_dec_buf[0] | (ctx->cobs_dec_buf[1] << 8);
			ctx->cur_chan = w;
			printf("-- msgblend_receiver_parse_decoded: channel is %u\n", w);

		} else {
			printf("-- msgblend_receiver_parse_decoded: channel not found yet, len=%u\n", decode_len);

			return;
		}

		size_t decode_len_remain = decode_len - 2;

		if (decode_len_remain) {
			ctx->receive_callback((uint16_t) ctx->cur_chan, ctx->cobs_dec_buf + 2, decode_len_remain, ctx->userp);
		}
	}

	{
		ctx->xcobs_dec_ctx.buf_out = ctx->cobs_dec_buf;
		ctx->xcobs_dec_ctx.avail_out = ctx->cobs_dec_buf_len;
	}

	printf("-- msgblend_receiver_parse_decoded: done\n");

}

void msgblend_receiver_decode(msgblend_receiver_ctx_t *ctx, const void *data, size_t len) {
	if (len) {
		printf("-- msgblend_receiver_decode: read %u bytes\n", len);


		printf("---- cobs_dec: before: avail_in=%u, avail_out=%u\n", ctx->xcobs_dec_ctx.avail_in, ctx->xcobs_dec_ctx.avail_out);

		ctx->xcobs_dec_ctx.buf_in = (uint8_t *)data;
		ctx->xcobs_dec_ctx.avail_in = len;

		printf("---- cobs_dec: apply: buf_in=0x%x, avail_in=%u\n", (uintptr_t)data, len);

		while (1) {
			int rc_cdec = xcobs_decode(&ctx->xcobs_dec_ctx);

			printf("---- cobs_dec: after: rc=%d, avail_in=%u, drained_in=%u, avail_out=%u, drained_out=%u\n", rc_cdec,
			       ctx->xcobs_dec_ctx.avail_in, ctx->xcobs_dec_ctx.drained_in,
			       ctx->xcobs_dec_ctx.avail_out, ctx->xcobs_dec_ctx.drained_out);


			if (rc_cdec == XCOBS_DECODE_OK) {
				printf("---- cobs_dec: OK\n");

				msgblend_receiver_parse_decoded(ctx);
			} else if (rc_cdec == XCOBS_DECODE_EOF) {
				printf("---- cobs_dec: EOF\n");

				msgblend_receiver_parse_decoded(ctx);
				xcobs_decoder_init(&ctx->xcobs_dec_ctx);
				ctx->receive_callback((uint16_t)ctx->cur_chan, NULL, 0, ctx->userp);
				ctx->cur_chan = -1;
			} else if (rc_cdec == XCOBS_DECODE_EAGAIN) {
				printf("---- cobs_dec: EAGAIN\n");

				if (ctx->xcobs_dec_ctx.avail_out == 0) {
					printf("---- cobs_dec: output unavailable\n");
				}

				if (ctx->xcobs_dec_ctx.avail_in == 0) {
					printf("---- cobs_dec: input unavailable, left ctx intact and exit for now\n");
					break;
				}
			}

		}
	}
}

size_t msgblend_sender_encode(uint16_t channel, const void *input_buf, size_t input_len, uint8_t *output_buf) {
	const void *multi_inputs[2] = {&channel, input_buf};
	const size_t multi_lengths[2] = {2, input_len};

	return xcobs_encode_multi(multi_inputs, multi_lengths, 2, output_buf);
}
