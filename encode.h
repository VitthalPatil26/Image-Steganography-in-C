#ifndef ENCODE_H
#define ENCODE_H
#include <stdio.h>
#include <stdint.h>

#include "types.h" 

typedef struct _EncodeInfo
{

	char *src_image_fname;
	FILE *fptr_src_image;
	uint image_capacity;


	char *secret_fname;
	FILE *fptr_secret;
	char extn_secret_file[5];
	char secret_data[100];
	long size_secret_file;

	char *stego_image_fname;
	FILE *fptr_stego_image;

} EncodeInfo;

Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo);

Status do_encoding(EncodeInfo *encInfo);

Status open_files(EncodeInfo *encInfo);

Status check_capacity(EncodeInfo *encInfo);

uint get_image_size_for_bmp(FILE *fptr_image);

uint get_file_size(FILE *fptr);

Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image);

Status encode_magic_string(const char *file_extn, EncodeInfo *encInfo);

Status encode_secret_file_extn_size(int size, EncodeInfo *encInfo);

Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo);

Status encode_secret_file_size(long file_size, EncodeInfo *encInfo);

Status encode_secret_file_data(EncodeInfo *encInfo);

Status encode_byte_to_lsb(char data, char *image_buffer);

Status encode_size_to_lsb(int size, char *imageBuffer);

Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest);

#endif
