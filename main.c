
#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "decode.h"
#include "types.h"
#include "common.h"

OperationType check_operation_type(char *);

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("Error: Missing operation type\n");
        printf("Usage:\n");
        printf("./Steganography -e <source.bmp> <secret.txt> [output.bmp]\n");
        printf("./Steganography -d <stego.bmp> [output.txt]\n");
        return 1;
    }

    if (check_operation_type(argv[1]) == e_encode)
    {
        if (argc < 4 || argc > 5)
        {
            printf("Usage: ./Steganography -e <source.bmp> <secret.txt> [output.bmp]\n");
            return 1;
        }

        EncodeInfo encInfo;

        if (read_and_validate_encode_args(argv, &encInfo) != e_success)
        {
            printf("Error: Invalid encoding arguments\n");
            return 1;
        }

        if (do_encoding(&encInfo) != e_success)
        {
            printf("Error: Encoding failed\n");
            return 1;
        }

        printf("Encoding completed successfully\n");
    }
    else if (check_operation_type(argv[1]) == e_decode)
    {
        if (argc < 3 || argc > 4)
        {
            printf("Usage: ./Steganography -d <stego.bmp> [output.txt]\n");
            return 1;
        }

        DecodeInfo decInfo;

        if (read_and_validate_decode_args(argv, &decInfo) != e_success)
        {
            printf("Error: Invalid decoding arguments\n");
            return 1;
        }

        if (do_decoding(&decInfo) != e_success)
        {
            printf("Error: Decoding failed\n");
            return 1;
        }

        printf("Decoding completed successfully\n");
    }
    else
    {
        printf("Error: Unsupported operation\n");
        printf("Use -e for encoding or -d for decoding\n");
        return 1;
    }

    return 0;
}

OperationType check_operation_type(char *symbol)
{
    if (symbol == NULL)
    {
        return e_unsupported;
    }

    if (strcmp(symbol, "-e") == 0)
    {
        return e_encode;
    }

    if (strcmp(symbol, "-d") == 0)
    {
        return e_decode;
    }

    return e_unsupported;
}

