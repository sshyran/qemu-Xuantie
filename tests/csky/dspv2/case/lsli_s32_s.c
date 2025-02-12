/*
 * Copyright (c) 2021 T-Head Semiconductor Co., Ltd. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#include "testsuite.h"
#include "test_device.h"
#include "dspv2_insn.h"
#include "sample_array.h"
int main(void)
{
    int i = 0;
    init_testsuite("Testing insn lsli.s32.s\n");

    for (i = 0;
         i < sizeof(samples_lsli_s32_s)/sizeof(struct binary_calculation);
         i++) {
        if (samples_lsli_s32_s[i].op2 == 32) {
            TEST(test_lsli_s32_s_32(samples_lsli_s32_s[i].op1)
                 == samples_lsli_s32_s[i].result);
        } else if (samples_lsli_s32_s[i].op2 == 2) {
            TEST(test_lsli_s32_s_2(samples_lsli_s32_s[i].op1)
                 == samples_lsli_s32_s[i].result);
        } else if (samples_lsli_s32_s[i].op2 == 4) {
            TEST(test_lsli_s32_s_4(samples_lsli_s32_s[i].op1)
                 == samples_lsli_s32_s[i].result);
        } else if (samples_lsli_s32_s[i].op2 == 31) {
            TEST(test_lsli_s32_s_31(samples_lsli_s32_s[i].op1)
                 == samples_lsli_s32_s[i].result);
        }
    }
    return done_testing();
}
