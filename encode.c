#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "types.h"
#include "common.h"

uint get_image_size_for_bmp(FILE *fptr_image)
{
	uint width, height;

	fseek(fptr_image, 18, SEEK_SET);

	fread(&width, sizeof(int), 1, fptr_image);
	printf("width  %u\n", width);

	fread(&height, sizeof(int), 1, fptr_image);
	printf("height = %u\n", height);

	return width * height * 3;
}

uint get_file_size(FILE *fptr)
{

	fseek(fptr, 0, SEEK_END);

	uint size  = ftell(fptr);

	rewind(fptr);

	return size;
}

Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
	char *extn = strrchr(argv[2], '.');

	if(extn == NULL || strcmp(extn, ".bmp") != 0)
	{
		printf("Error: Souece image must have .bmp extension\n");
		return e_failure;
	}

	encInfo->src_image_fname = argv[2];

	extn = strrchr(argv[3], '.');

	if(extn == NULL || extn == argv[3] || extn[1] == '\0')
	{
		printf("Error: Invalid secret file extension\n");
		return e_failure;
	}

	if(strlen(extn) >= sizeof(encInfo->extn_secret_file))
	{
		printf("Error: Secret file extension is too long\n");
		return e_failure;
	}

	encInfo->secret_fname = argv[3];
	strcpy(encInfo->extn_secret_file, extn);

	if(argv[4] == NULL)
	{
		encInfo->stego_image_fname = "stego.bmp";
	}

	else
	{
		extn = strrchr(argv[4], '.');

		if(extn == NULL || strcmp(extn, ".bmp") != 0){
			printf("Error: Output image must have .bmp extension\n");
			return e_failure;
		}

		encInfo->stego_image_fname = argv[4];
	}

	return e_success;
}

Status open_files(EncodeInfo *encInfo)
{
   
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "r");
    
    if (encInfo->fptr_src_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);

        return e_failure;
    }

    encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");
    
    if (encInfo->fptr_secret == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);

        return e_failure;
    }

    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "w");
    
    if (encInfo->fptr_stego_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);

        return e_failure;
    }

    return e_success;
}

Status check_capacity(EncodeInfo *encInfo)
{
	
	encInfo->image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image);

	encInfo->size_secret_file = get_file_size(encInfo->fptr_secret);
		
	if(encInfo->image_capacity > (16 + 32 + (strlen(encInfo->extn_secret_file) * 8) + 32 + (encInfo->size_secret_file * 8)))		
	{
		return e_success;
	}

	else{

		return e_failure;
	}
}

Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
	char buffer[54];

	rewind(fptr_src_image);

	if(fread(buffer, 1, 54, fptr_src_image) != 54)
	{
		printf("Error: Unable to read BMP file\n");
		return e_failure;
	}

	if(fwrite(buffer, 1, 54, fptr_dest_image) != 54)
	{
		printf("Error: Unable to write BMP filr\n");
		return e_failure;
	}

	return e_success;
}

Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
	char buffer[8];

	for(int i = 0; i < strlen(magic_string); i++)
	{
		if(fread(buffer, 1, 8, encInfo->fptr_src_image) != 8)
		{
			return e_failure;
		}

		if(encode_byte_to_lsb(magic_string[i], buffer) != e_success)
		{
			return e_failure;
		}

		if(fwrite(buffer, 1, 8, encInfo->fptr_stego_image) != 8)
		{
			return e_failure;
		}
	}

	return e_success;

}

Status encode_secret_file_extn_size(int size, EncodeInfo *encInfo)
{
	char buffer[32];

	if(fread(buffer, 1, 32, encInfo->fptr_src_image) != 32)
	{
		printf("Error: Unable to read image data\n");
		return e_failure;
	}

	if(encode_size_to_lsb(size, buffer) != e_success)
	{
		printf("Error: Unable to encode extension size\n");
		return e_failure;
	}

	if(fwrite(buffer, 1, 32, encInfo->fptr_stego_image) != 32)
	{
		printf("Error: Unable to write encoded data\n");
		return e_failure;
	}

	return e_success;

}

Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
	char buffer[8];
	int len = strlen(file_extn);

	for(int i = 0; i < len; i++)
	{
		if(fread(buffer, 1, 8, encInfo->fptr_src_image) != 8)
		{
			printf("Error: Unable to read image data\n");
			return e_failure;
		}

		if(encode_byte_to_lsb(file_extn[i], buffer) != e_success)
		{
			printf("Error: Unable to code extension\n");
			return e_failure;
		}

		if(fwrite(buffer, 1, 8, encInfo->fptr_stego_image) != 8)
		{
			printf("Error: unable to write image data\n");
			return e_failure;
		}
	}

	return e_success;

}

Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
{

	char buffer[32];

	if(fread(buffer, 1, 32, encInfo->fptr_src_image) != 32)
	{
		printf("Error: Unable to read image data\n");
		return e_failure;
	}

	if(encode_size_to_lsb((int)file_size, buffer) != e_success)
	{
		printf("Error: Unable to encode secret file size\n");
		return e_failure;
	}

	if(fwrite(buffer, 1, 32, encInfo->fptr_stego_image) != 32)
	{
		printf("Error: Unable to write image data\n");
		return e_failure;
	}

	return e_success;

}

Status encode_secret_file_data(EncodeInfo *encInfo)
{

	char buffer[8];
	char data;

	for(long i = 0; i < encInfo->size_secret_file; i++)
	{

		if(fread(&data, 1, 1, encInfo->fptr_secret) != 1)
		{
			return e_failure;
		}

		if(fread(buffer, 1, 8, encInfo->fptr_src_image) != 8)
		{
			return e_failure;
		}

		if(encode_byte_to_lsb(data, buffer) != e_success)
		{
			return e_failure;
		}

		if(fwrite(buffer, 1, 8, encInfo->fptr_stego_image) != 8)
		{
			return e_failure;
		}
	}

	return e_success;

}

Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{

	char buffer[1024];
	size_t bytes_read;

	while((bytes_read = fread(buffer, 1, sizeof(buffer), fptr_src)) > 0)
	{
		if(fwrite(buffer, 1, bytes_read, fptr_dest) != bytes_read)
		{
			return e_failure;
		}
	}

	if(ferror(fptr_src))
		return e_failure;

	return e_success;

}

Status encode_byte_to_lsb(char data, char *image_buffer)
{
	unsigned char value = (unsigned char)data;

	for(int i =0; i < 8; i++)
	{
		int bit = (value >> (7 - i)) & 1;

		image_buffer[i] = (image_buffer[i] & 0XFE) | bit;

	}

	return e_success;

}

Status encode_size_to_lsb(int size, char *imageBuffer)
{

	uint32_t value = (uint32_t)size;

	for(int i = 0; i < 32; i++)
	{
		int bit = (value >> (31 - i)) & 1;

		imageBuffer[i] = (imageBuffer[i] & 0XFE) | bit;

	}

	return e_success;

}

Status do_encoding(EncodeInfo *encInfo)
{

	if(open_files(encInfo) != e_success)
	{
		printf("Error: Unable to open files\n");
		return e_failure;
	}

	printf("Files opened successfully\n");

	if(check_capacity(encInfo) != e_success)
	{
		printf("Error: Insufficient image capacity\n");
		return e_failure;
	}

	printf("Image has suffiecient capacity\n");

	if(copy_bmp_header(encInfo->fptr_src_image, encInfo->fptr_stego_image) != e_success)
	{
		printf("Error: Unable to copy BMP header\n");
		return e_failure;
	}

	printf("BMP header copied successfully\n");

	if(encode_magic_string(MAGIC_STRING, encInfo) != e_success)
	{
		printf("Error: Unable to encode magic string\n");
		return e_failure;
	}

	printf("Magic string encoded successfully\n");

	if(encode_secret_file_extn_size(strlen(encInfo->extn_secret_file), encInfo) != e_success)
	{
		printf("Error: Unable to encode secret file extension size\n");
		return e_failure;
	}

	printf("Secret file extension size encoded successfully\n");

	if(encode_secret_file_extn(encInfo->extn_secret_file, encInfo) != e_success)
	{
		printf("Error: Unable to encode extension\n");
		return e_failure;
	}

	printf("Secret file extension encoded successfully\n");

	if(encode_secret_file_size(encInfo->size_secret_file, encInfo) != e_success)
	{
		printf("Error: Unable to encode secret file size\n");
		return e_failure;
	}

	printf("Secret file size encoded successfully\n");

	if(encode_secret_file_data(encInfo) != e_success)
	{
		printf("Error: Unable to encode secret file data\n");
		return e_failure;
	}

	printf("Secret file data encoded successfully\n");

	if(copy_remaining_img_data(encInfo->fptr_src_image, encInfo->fptr_stego_image) != e_success)
	{
		printf("Error: Unable to copy remaining image data\n");
		return e_failure;
	}

	printf("Remaining image data copied successfully\n");

	fclose(encInfo->fptr_src_image);
	fclose(encInfo->fptr_secret);
	fclose(encInfo->fptr_stego_image);

	return e_success;
}


