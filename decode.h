
#ifndef DECODE_H
#define DECODE_H

#include <stdio.h>
#include <stdint.h>
#include "types.h"

typedef struct _DecodeInfo
{
    char *stego_image_fname;
    FILE *fptr_stego_image;

    char extn_secret_file[5];
    uint32_t size_secret_file;

    char *output_fname;
    FILE *fptr_output;
} DecodeInfo;

Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo);
Status do_decoding(DecodeInfo *decInfo);

Status decode_magic_string(DecodeInfo *decInfo);
Status decode_secret_file_extn_size(DecodeInfo *decInfo);
Status decode_secret_file_extn(DecodeInfo *decInfo);
Status decode_secret_file_size(DecodeInfo *decInfo);
Status decode_secret_file_data(DecodeInfo *decInfo);

Status decode_byte_from_lsb(FILE *fptr, char *data);
Status decode_size_from_lsb(FILE *fptr, uint32_t *size);

#endif

