/**********************************************************************************************/
/* The MIT License                                                                            */
/*                                                                                            */
/* Copyright 2016-2017 Twitch Interactive, Inc. or its affiliates. All Rights Reserved.       */
/*                                                                                            */
/* Permission is hereby granted, free of charge, to any person obtaining a copy               */
/* of this software and associated documentation files (the "Software"), to deal              */
/* in the Software without restriction, including without limitation the rights               */
/* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell                  */
/* copies of the Software, and to permit persons to whom the Software is                      */
/* furnished to do so, subject to the following conditions:                                   */
/*                                                                                            */
/* The above copyright notice and this permission notice shall be included in                 */
/* all copies or substantial portions of the Software.                                        */
/*                                                                                            */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR                 */
/* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,                   */
/* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE                */
/* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER                     */
/* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,              */
/* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN                  */
/* THE SOFTWARE.                                                                              */
/**********************************************************************************************/
#ifndef LIBCAPTION_XDS_H
#define LIBCAPTION_XDS_H
#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h>
#include <stddef.h>

typedef enum {
    CONTENT = 0,
    POST_CONTENT = 1,
    CHECKSUM = 2
} frame_location;

// Define Class Type Start Codes
#define CURRENT 0x01
#define FUTURE 0x03
#define CHANNEL 0x05
#define MISC 0x07
#define PUBLIC_SERVICE 0x09
#define CLASS_RESERVED 0x0B
#define PRIVATE_DATA 0x0D

// Define Program Information Class Types
#define START_TIME 0x01
#define LENGTH 0x02
#define TITLE 0x03
#define PROGRAM_TYPE 0x04
#define CONTENT_ADVISORY 0x05
#define AUDIO_SERVICES 0x06
#define CAPTION_SERVICES 0x07
#define COPYRIGHT 0x08
#define PI_RESERVED 0x09
#define COMPOSITE_P1 0x0C
#define COMPOSITE_p2 0x0D
//0x10 to 0x17 are program description rows 1 to 8


typedef struct {
    int state;
    frame_location location;

    uint8_t class;
    uint8_t type;
    uint32_t size;
    uint8_t content[32];
    uint8_t checksum;
} xds_t;

void xds_init(xds_t* xds);
int xds_decode(xds_t* xds, uint16_t cc);

#ifdef __cplusplus
}
#endif
#endif
