# SPDX-License-Identifier: Apache-2.0

set(OPENOCD_USE_LOAD_IMAGE NO)

board_runner_args(openocd "--config=${BOARD_DIR}/support/openocd_digilent_hs2.cfg")

include(${ZEPHYR_BASE}/boards/common/openocd.board.cmake)
