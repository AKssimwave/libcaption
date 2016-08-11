/**********************************************************************************************/
/* Copyright 2016-2016 Twitch Interactive, Inc. or its affiliates. All Rights Reserved.               */
/*                                                                                            */
/* Licensed under the Apache License, Version 2.0 (the "License"). You may not use this file  */
/* except in compliance with the License. A copy of the License is located at                 */
/*                                                                                            */
/*     http://aws.amazon.com/apache2.0/                                                       */
/*                                                                                            */
/* or in the "license" file accompanying this file. This file is distributed on an "AS IS"    */
/* BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the    */
/* License for the specific language governing permissions and limitations under the License. */
/**********************************************************************************************/

#include "cea708.h"
#include <memory.h>

int cea708_cc_count (user_data_t* data)
{
    return data->cc_count;
}

uint16_t cea708_cc_data (user_data_t* data, int index, int* valid, cea708_cc_type_t* type)
{
    (*valid) = data->cc_data[index].cc_valid;
    (*type) = data->cc_data[index].cc_type;
    return data->cc_data[index].cc_data;
}


int cea708_init (cea708_t* cea708)
{
    memset (cea708,0,sizeof (cea708_t));
    cea708->country = country_united_states;
    cea708->provider = t35_provider_atsc;
    cea708->user_identifier = ('G'<<24) | ('A'<<16) | ('9'<<8) | ('4');
    cea708->atsc1_data_user_data_type_code = 3; //what does 3 mean here?
    cea708->directv_user_data_length = 0;
    ///////////
    cea708->user_data.process_em_data_flag = 0;
    cea708->user_data.process_cc_data_flag = 1;
    cea708->user_data.additional_data_flag = 0;
    cea708->user_data.cc_count = 0;
    return 1;
}

// 00 00 00  06 C1  FF FC 34 B9 FF : onCaptionInfo.
int cea708_parse (uint8_t* data, size_t size, cea708_t* cea708)
{
    int i;
    cea708->country = (itu_t_t35_country_code_t) (data[0]);
    cea708->provider = (itu_t_t35_provider_code_t) ( (data[1] <<8) | data[2]);
    cea708->atsc1_data_user_data_type_code = 0;
    cea708->user_identifier = 0;
    data += 3; size -= 3;

    if (t35_provider_atsc == cea708->provider) {
        // GA94
        cea708->user_identifier = (data[0] <<24) | (data[1] <<16) | (data[2] <<8) | data[3];
        data += 4; size -= 4;
    }

    // Im not sure what this extra byt is. It sonly seesm to come up in onCaptionInfo
    // where country and provider are zero
    if (0 == cea708->provider) {
        data += 1; size -= 1;
    } else if (t35_provider_atsc == cea708->provider || t35_provider_direct_tv == cea708->provider) {
        cea708->atsc1_data_user_data_type_code = data[0];
        data += 1; size -= 1;
    }

    if (t35_provider_direct_tv == cea708->provider) {
        cea708->directv_user_data_length = data[0];
        data += 1; size -= 1;
    }

    // TODO I believe this is condational on the above.
    cea708->user_data.process_em_data_flag = !! (data[0]&0x80);
    cea708->user_data.process_cc_data_flag = !! (data[0]&0x40);
    cea708->user_data.additional_data_flag = !! (data[0]&0x20);
    cea708->user_data.cc_count             = (data[0]&0x1F);
    cea708->user_data.em_data              = data[1];
    data += 2; size -= 2;

    if (size < 3 * cea708->user_data.cc_count) {
        cea708_init (cea708);
        return 0;
    }

    for (i = 0 ; i < (int) cea708->user_data.cc_count ; ++i) {
        cea708->user_data.cc_data[i].marker_bits = data[0]>>3;
        cea708->user_data.cc_data[i].cc_valid    = data[0]>>2;
        cea708->user_data.cc_data[i].cc_type     = data[0]>>0;
        cea708->user_data.cc_data[i].cc_data     = data[1]<<8|data[2];
        data += 3; size -= 3;
    }

    return 1;
}

int cea708_add_cc_data (cea708_t* cea708, int valid, cea708_cc_type_t type, uint16_t cc_data)
{
    if (31 <= cea708->user_data.cc_count) {
        return 0;
    }

    cea708->user_data.cc_data[cea708->user_data.cc_count].marker_bits = 0x1F;
    cea708->user_data.cc_data[cea708->user_data.cc_count].cc_valid = valid;
    cea708->user_data.cc_data[cea708->user_data.cc_count].cc_type = type;
    cea708->user_data.cc_data[cea708->user_data.cc_count].cc_data = cc_data;
    ++cea708->user_data.cc_count;
    return 1;
}

int cea708_render (cea708_t* cea708, uint8_t* data, size_t size)
{
    int i;    size_t total = 0;
    data[0] = cea708->country;
    data[1] = cea708->provider>>8;
    data[2] = cea708->provider>>0;
    total += 3; data += 3; size -= 3;

    if (t35_provider_atsc == cea708->provider) {

        data[0] = cea708->user_identifier >> 24;
        data[1] = cea708->user_identifier >> 16;
        data[2] = cea708->user_identifier >> 8;
        data[3] = cea708->user_identifier >> 0;
        total += 4; data += 4; size -= 4;
    }

    if (t35_provider_atsc == cea708->provider || t35_provider_direct_tv == cea708->provider) {
        data[0] = cea708->atsc1_data_user_data_type_code;
        total += 1; data += 1; size -= 1;
    }

    if (t35_provider_direct_tv == cea708->provider) {
        data[0] = cea708->directv_user_data_length;
        total += 1; data += 1; size -= 1;
    }

    data[1] = cea708->user_data.em_data;
    data[0] = (cea708->user_data.process_em_data_flag?0x80:0x00)
              | (cea708->user_data.process_cc_data_flag?0x40:0x00)
              | (cea708->user_data.additional_data_flag?0x20:0x00)
              | (cea708->user_data.cc_count & 0x1F);

    total += 2; data += 2; size -= 2;

    for (i = 0 ; i < (int) cea708->user_data.cc_count ; ++i) {
        data[0] = (cea708->user_data.cc_data[i].marker_bits<<3)
                  | (data[0] = cea708->user_data.cc_data[i].cc_valid<<2)
                  | (data[0] = cea708->user_data.cc_data[i].cc_type);
        data[1] = cea708->user_data.cc_data[i].cc_data>>8;
        data[2] = cea708->user_data.cc_data[i].cc_data>>0;
        total += 3; data += 3; size -= 3;
    }

    data[0] = 0xFF;
    return (int) (total + 1);
}

cc_data_t cea708_encode_cc_data (int cc_valid, cea708_cc_type_t type, uint16_t cc_data)
{
    cc_data_t data = { 0x1F, cc_valid,type,cc_data};
    return data;
}

void cea708_dump (cea708_t* cea708)
{
    int i;

    for (i = 0 ; i < (int) cea708->user_data.cc_count ; ++i) {
        cea708_cc_type_t type; int valid;
        uint16_t cc_data = cea708_cc_data (&cea708->user_data, i, &valid, &type);

        if (valid && (cc_type_ntsc_cc_field_1 == type || cc_type_ntsc_cc_field_2 == type)) {
            eia608_dump (cc_data);
        }
    }
}

int cea708_to_caption_frame (caption_frame_t* frame, cea708_t* cea708, double pts)
{
    int i, count = cea708_cc_count (&cea708->user_data);

    for (i = 0 ; i < count ; ++i) {
        cea708_cc_type_t type; int valid;
        uint16_t cc_data = cea708_cc_data (&cea708->user_data, i, &valid, &type);

        if (valid && (cc_type_ntsc_cc_field_1 == type || cc_type_ntsc_cc_field_2 == type)) {
            caption_frame_decode (frame,cc_data, pts);
        }
    }

    // TODO look at the result of caption_frame_decode and return the rite value
    return 2;
}
