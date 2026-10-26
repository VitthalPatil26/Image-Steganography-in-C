#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "decode.h"
#include "common.h"

Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
	char *extn = strrchr(argv[2], '.');

	if(extn == NULL || strcmp(extn, ".bmp") != 0)
	{
		printf("Error: Input image must have .bmp extension\n");
		return e_failure;
	}

	decInfo->stego_image_fname = argv[2];

	if(argv[3] != NULL)
		decInfo->output_fname = argv[3];

	else
		decInfo->output_fname = NULL;

	return e_success;
}

Status decode_byte_from_lsb(FILE *fptr, char *data)
{
	unsigned char byte;
	unsigned char value = 0;

	for(int i = 0; i < 8; i++)
	{
		if(fread(&byte, 1, 1, fptr) != 1)
			return e_failure;

		value = (value << 1) | (byte & 1U);
	}

	*data = (char)value;
	return e_success;
}

Status decode_size_from_lsb(FILE *fptr, uint32_t *size)
{
	unsigned char byte;
	uint32_t value = 0;

	for(int i = 0; i < 32; i++)
	{
		if(fread(&byte, 1, 1, fptr) != 1)
			return e_failure;

		value = (value << 1) | (byte & 1U);

	}

	*size = value;
	return e_success;
}

Status decode_magic_string(DecodeInfo *decInfo)
{
	char decoded[sizeof(MAGIC_STRING)] = {0};

	for(size_t i = 0; i < strlen(MAGIC_STRING); i++)
	{
		if(decode_byte_from_lsb(decInfo->fptr_stego_image, &decoded[i]) != e_success)
			return e_failure;

	}

	decoded[strlen(MAGIC_STRING)] = '\0';

	if(strcmp(decoded, MAGIC_STRING) != 0)
	{
		printf("Error: Magic string does not match\n");
		return e_success;
	}

	printf("Magic string verified successfully\n");
	return e_success;
}

Status decode_secret_file_extn_size(DecodeInfo *decInfo)
{
	uint32_t size;

	if(decode_size_from_lsb(decInfo->fptr_stego_image, &size) != e_success)
	{
		printf("Error: Unable to decode extension sizde\n");
		return e_failure;
	}

	if(size == 0 || size >= sizeof(decInfo->extn_secret_file))
	{
		printf("Error: Invalid secret file extension size\n");
		return e_failure;
	}

	decInfo->extn_secret_file[size] = '\0';

	decInfo->extn_secret_file[4] =  (char)size;

	return e_success;
}

Status decode_secret_file_extn(DecodeInfo *decInfo)
{
	uint32_t size = (unsigned char)decInfo->extn_secret_file[4];

	for(uint32_t i = 0; i < size; i++)
	{
		if(decode_byte_from_lsb(decInfo->fptr_stego_image, &decInfo->extn_secret_file[i]) != e_success)
		{
			printf("Error: Unable to decode extension\n");
			return e_failure;
		}
	}

	decInfo->extn_secret_file[size] = '\0';

	printf("Secret file extension: %s\n", decInfo->extn_secret_file);
	return e_success;
}

Status decode_file_size(DecodeInfo *decInfo)
{
	if(decode_size_from_lsb(decInfo->fptr_stego_image, &decInfo->size_secret_file) != e_success)
	{
		printf("Error: Unable to decode secret file size\n");
		return e_failure;
	}

	printf("Secret file size: %u bytes\n", (unsigned int)decInfo->size_secret_file);

	return e_success;
}

Status decode_secret_file_data(DecodeInfo *decInfo)
{
	for(uint32_t i = 0; i < decInfo->size_secret_file; i++)
	{
		char data;

		if(decode_byte_from_lsb(decInfo->fptr_stego_image, &data) != e_success)
		{
			printf("Error: Unable to decode secret data\n");
			return e_failure;
		}

		if(fwrite(&data, 1, 1, decInfo->fptr_output) != 1)
		{
			printf("Error: Unable to write decoded data\n");
			return e_failure;
		}
	}

	return e_success;

}

Status do_decoding(DecodeInfo *decInfo)
{
	decInfo->fptr_stego_image = fopen(decInfo->stego_image_fname, "rb");

	if(decInfo->fptr_stego_image == NULL)
	{
		perror("Unable to open stego file");
		return e_failure;
	}

	if(fseek(decInfo->fptr_stego_image, 54, SEEK_SET) != 0)
	{
		fclose(decInfo->fptr_stego_image);
		return e_failure;
	}

	if(decode_magic_string(decInfo) != e_success)
		goto failure;

	if(decode_secret_file_extn_size(decInfo) != e_success)
		goto failure;

	if(decode_secret_file_extn(decInfo) != e_success)
		goto failure;

	if(decode_file_size(decInfo) != e_success)
		goto failure;

	if(decInfo->output_fname == NULL)
	{
		char filename[64];

		snprintf(filename, sizeof(filename), "decode%s", decInfo->extn_secret_file);

		decInfo->fptr_output = fopen(filename, "wb");
		if(decInfo->fptr_output == NULL)
		{
			perror("Unable to create decode file");
			goto failure;
		}

		printf("Output file: %s\n", filename);

	}
	else
	{
		decInfo->fptr_output = fopen(decInfo->output_fname, "wb");

		if(decInfo->fptr_output == NULL)
		{
			perror("Unable to create output file");
			goto failure;
		}
	}

	if(decode_secret_file_data(decInfo) != e_success)
		goto failure;

	fclose(decInfo->fptr_output);
	fclose(decInfo->fptr_stego_image);

	return e_success;

failure:
	fclose(decInfo->fptr_stego_image);
	return e_failure;
	
}
