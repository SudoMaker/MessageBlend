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

#include "xcobs.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*msgblend_receive_callback_t)(uint16_t chan, uint8_t *data, size_t len, void *userp);

typedef struct {
	xcobs_decode_ctx_t xcobs_dec_ctx;
	uint8_t *cobs_dec_buf;
	size_t cobs_dec_buf_len;
	msgblend_receive_callback_t receive_callback;
	void *userp;
	int32_t cur_chan;
} msgblend_receiver_ctx_t;

#define msgblend_sender_encoded_len(x)		xcobs_encoded_len((x)+2)

extern void msgblend_receiver_init(msgblend_receiver_ctx_t *ctx, uint8_t *rx_buf, size_t rx_buf_len, msgblend_receive_callback_t rx_callback, void *userp);
extern void msgblend_receiver_decode(msgblend_receiver_ctx_t *ctx, const void *data, size_t len);
extern size_t msgblend_sender_encode(uint16_t channel, const void *input_buf, size_t input_len, uint8_t *output_buf);

#ifdef __cplusplus
}
#endif
