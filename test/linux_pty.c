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

#define _XOPEN_SOURCE 9999
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include <termios.h>
#include <sys/ioctl.h>
#include <sys/epoll.h>

#include <msgblend.h>


static void ShowHelp() {
	puts("Usage: test_linux_pty <m|s <master_dev>>");
}

static int set_tty_raw(int fd) {
	struct termios tio;

	if (ioctl(fd, TCGETS, &tio)) {
		abort();
	}

	tio.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
	tio.c_oflag &= ~OPOST;
	tio.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
	tio.c_cflag &= ~(CSIZE | PARENB);
	tio.c_cflag |= CS8;

	if (ioctl(fd, TCSETS, &tio)) {
		abort();
	}

	return 0;
}


static int fds[16];

static void mb_rx_callback(uint16_t chan, uint8_t *data, size_t len, void *userp) {
	if (chan && chan < 16) {
		size_t wr_len = 0;

		while (wr_len < len) {
			ssize_t wrc = write(fds[chan], data + wr_len, len - wr_len);

			if (wrc > 0) {
				wr_len += wrc;
			} else {
				abort();
			}
		}
	}
}

int main(int argc, char **argv) {
	if (argc < 2) {
		ShowHelp();
		return 1;
	}

	char mode = argv[1][0];
	char *master_dev = argv[2];



	for (size_t i=0; i<16; i++) {
		fds[i] = -1;
	}

	int epfd = epoll_create(43);

	struct epoll_event ev;

	if (mode == 's') {
		assert(master_dev);
		fds[0] = open(master_dev, O_RDWR);
		assert(fds[0] != -1);
		set_tty_raw(fds[0]);

		ev.data.u32 = 0;
		ev.events = EPOLLIN;
		epoll_ctl(epfd, EPOLL_CTL_ADD, fds[0], &ev);
	}

	for (size_t i=0; i<16; i++) {
		if (fds[i] == -1) {
			fds[i] = posix_openpt(O_RDWR | O_NOCTTY);
			assert(fds[0] != -1);
			assert(grantpt(fds[i]) == 0);
			assert(unlockpt(fds[i]) == 0);

			set_tty_raw(fds[0]);

			ev.data.u32 = i;
			ev.events = EPOLLIN;
			epoll_ctl(epfd, EPOLL_CTL_ADD, fds[i], &ev);
		}

		printf("[%zu]: %s\n", i, ptsname(fds[i]));
	}


	msgblend_receiver_ctx_t mb_r_ctx;

	uint8_t mb_rx_buf[32];

	msgblend_receiver_init(&mb_r_ctx, mb_rx_buf, sizeof(mb_rx_buf), mb_rx_callback, NULL);

	struct epoll_event evs[16];

	uint8_t io_buf[128];

	while (1) {
		int rc_e = epoll_wait(epfd, evs, 16, 1000);

		if (rc_e > 0) {
			for (size_t i=0; i<rc_e; i++) {
				struct epoll_event *cur_ev = &evs[i];
				size_t cur_chan = cur_ev->data.u32;
				int cur_fd = fds[cur_chan];

				if (cur_ev->events & EPOLLIN) {
					ssize_t rc_r = read(cur_fd, io_buf, sizeof(io_buf));

					if (rc_r > 0) {
						if (cur_chan == 0) { // Mux
							msgblend_receiver_decode(&mb_r_ctx, io_buf, rc_r);
						} else {
							uint8_t *mb_tx_buf = malloc(msgblend_sender_encoded_len(rc_r));

							size_t e_len = msgblend_sender_encode(cur_chan, io_buf, rc_r, mb_tx_buf);

							size_t ew_len = 0;

							while (ew_len < e_len) {
								ssize_t rc_w = write(fds[0], mb_tx_buf, e_len);

								if (rc_w > 0) {
									ew_len += rc_w;
								} else {
									abort();
								}
							}

							free(mb_tx_buf);
						}
					} else {
						abort();
					}
				}

			}

		}

	}


}