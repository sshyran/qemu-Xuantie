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
#include "dspv3_insn.h"

struct vdsp_reg src[] = {
    {
        .float32 = {0x483471f7, 0x46f2e02b, 0xc785dc35, 0x47ad69d9, },
        .float16 = {0x6738, 0x5cdc, 0xe15b, 0x62f0, 0x5300, 0x48b6, 0xcd31,
            0x4eba, },
        .fixs32 = {0xcfe06686, 0x8262f661, 0x15fc5221, 0xd6b9745a, },
        .fixs16 = {0xd6d6, 0x51f2, 0x10ef, 0x0ea1, 0xa349, 0x4d3f, 0x475d, 0xa164, },
        .fixs8 = {0xa5, 0x34, 0x8c, 0x74, 0xcd, 0x75, 0x92, 0x7a, 0x60, 0x19, 0x3c,
            0x91, 0xfd, 0xab, 0x23, 0x21, },
    },
};

struct vdsp_reg dst[] = {
    {
        .float32 = {0x483471f7, 0x46f2e02b, 0x4785dc35, 0x47ad69d9, },
        .float16 = {0x6738, 0x5cdc, 0x615b, 0x62f0, 0x5300, 0x48b6, 0x4d31,
            0x4eba, },
        .fixs32 = {0x301f997a, 0x7d9d099f, 0x15fc5221, 0x29468ba6, },
        .fixs16 = {0x292a, 0x51f2, 0x10ef, 0x0ea1, 0x5cb7, 0x4d3f, 0x475d, 0x5e9c, },
        .fixs8 = {0x5b, 0x34, 0x74, 0x74, 0x33, 0x75, 0x6e, 0x7a, 0x60, 0x19, 0x3c, 0x6f, 0x03, 0x55, 0x23, 0x21, },
    },
};

struct vdsp_reg res;

int main(void)
{
    int i = 0;
    init_testsuite("Testing insn vabs\n");

    for (i = 0; i < sizeof(src) / sizeof(struct vdsp_reg); i++) {
        test_vabs_s8(src[i].fixs8, res.fixs8);
        result_compare_s8(res.fixs8, dst[i].fixs8);
    }

    for (i = 0; i < sizeof(src) / sizeof(struct vdsp_reg); i++) {
        test_vabs_s16(src[i].fixs16, res.fixs16);
        result_compare_s16(res.fixs16, dst[i].fixs16);
    }

    for (i = 0; i < sizeof(src) / sizeof(struct vdsp_reg); i++) {
        test_vabs_s32(src[i].fixs32, res.fixs32);
        result_compare_s32(res.fixs32, dst[i].fixs32);
    }

#ifdef VDSP_FLOAT
    for (i = 0; i < sizeof(src) / sizeof(struct vdsp_reg); i++) {
        test_vabs_f32(src[i].float32, res.float32);
        result_compare_s32(res.float32, dst[i].float32);
    }

    for (i = 0; i < sizeof(src) / sizeof(struct vdsp_reg); i++) {
        test_vabs_f16(src[i].float16, res.float16);
        result_compare_s16(res.float16, dst[i].float16);
    }
#endif
    return done_testing();
}
