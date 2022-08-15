// Copyright (C) <2019> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#ifndef MediaUtilities_h
#define MediaUtilities_h

static int partial_linear_bitrate[][2] = {
    {0, 0}, {76800, 400}, {307200, 800}, {921600, 2000}, {2073600, 4000}, {8294400, 16000}
};

inline unsigned int calcBitrate(unsigned int width, unsigned int height, float framerate = 30) {
    unsigned int bitrate = 0;
    unsigned int prev = 0;
    unsigned int next = 0;
    float portion = 0.0;
    unsigned int def = width * height * framerate / 30;
    int lines = sizeof(partial_linear_bitrate) / sizeof(partial_linear_bitrate[0][0]) / 2;

    // find the partial linear section and calculate bitrate
    for (int i = 0; i < lines - 1; i++) {
        prev = partial_linear_bitrate[i][0];
        next = partial_linear_bitrate[i+1][0];
        if (def > prev && def <= next) {
            portion = static_cast<float>(def - prev) / (next - prev);
            bitrate = partial_linear_bitrate[i][1] + (partial_linear_bitrate[i+1][1] - partial_linear_bitrate[i][1]) * portion;
            break;
        }
    }

    // set default bitrate for over large resolution
    if (0 == bitrate)
        bitrate = 8000;

    return bitrate;
}

inline int findNALU(uint8_t* buf, int size, int* nal_start, int* nal_end, int* sc_len)
{
    int i = 0;
    *nal_start = 0;
    *nal_end = 0;
    *sc_len = 0;

    while (true) {
        if (size < i + 3)
            return -1; /* Did not find NAL start */

        /* ( next_bits( 24 ) == {0, 0, 1} ) */
        if (buf[i] == 0 && buf[i + 1] == 0 && buf[i + 2] == 1) {
            i += 3;
            *sc_len = 3;
            break;
        }

        /* ( next_bits( 32 ) == {0, 0, 0, 1} ) */
        if (size > i + 3 && buf[i] == 0 && buf[i + 1] == 0 && buf[i + 2] == 0 && buf[i + 3] == 1) {
            i += 4;
            *sc_len = 4;
            break;
        }

        ++i;
    }

    // assert(buf[i - 1] == 1);
    *nal_start = i;

    /*( next_bits( 24 ) != {0, 0, 1} )*/
    while ((size > i + 2) &&
           (buf[i] != 0 || buf[i + 1] != 0 || buf[i + 2] != 1))
        ++i;

    if (size <= i + 2)
        *nal_end = size;
    else if (buf[i - 1] == 0)
        *nal_end = i - 1;
    else
        *nal_end = i;

    return (*nal_end - *nal_start);
}

#endif // MediaUtilities_h
