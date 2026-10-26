# Image Steganography in C (LSB Encoding)

A command-line application written in **C** that hides secret files inside uncompressed 24-bit BMP images using **Least Significant Bit (LSB) steganography**. It also extracts the hidden data and reconstructs the original file.

The technique modifies the least significant bits of image pixel bytes, causing minimal changes to pixel colour values that are generally imperceptible to the human eye.

## Features

- **Encode files:** Hide small files such as `.txt`, `.c`, and `.csv` inside BMP images.
- **Decode files:** Extract hidden data and reconstruct the original file.
- **Magic string verification:** Uses `#*` to identify the expected embedded payload format.
- **File metadata storage:** Stores the secret file's extension and size inside the image.
- **Capacity validation:** Checks whether the image has sufficient capacity before encoding.
- **Input validation:** Validates command-line arguments, operation types, and supported file extensions.
- **Modular architecture:** Separates encoding, decoding, common definitions, and program entry-point logic.

## How It Works

LSB steganography hides information by replacing the least significant bit of each selected image byte with one bit of the secret data.

For example:

```text
Original image byte : 10110100
Secret data bit     :        1
Modified image byte : 10110101
```

Only the least significant bit changes. The numerical difference between these two byte values is just 1, making the modification generally invisible to the human eye.

Each byte of secret data requires **8 image bytes**, because one secret bit is stored in each image byte.

### Encoding Process

The application copies the assumed 54-byte BMP header unchanged and embeds the secret data into the image's pixel-data region.

The information is stored in the following order:

| Order | Information                | Image bytes required |
| ----- | -------------------------- | -------------------: |
| 1     | Magic string (`#*`)        |                   16 |
| 2     | Secret file extension size |                   32 |
| 3     | Secret file extension      | Extension length × 8 |
| 4     | Secret file size           |                   32 |
| 5     | Secret file contents       |        File size × 8 |

Each size field is represented using 32 image bytes, storing one bit of the value in each image byte.

After encoding the secret data, the remaining image data is copied to the output image.

### Decoding Process

The decoding process reverses the encoding steps:

1. Opens the stego image.
2. Extracts and verifies the magic string.
3. Retrieves the secret file extension size.
4. Extracts the secret file extension.
5. Retrieves the original secret file size.
6. Extracts the hidden file contents.
7. Writes the recovered data to the output file.

This allows the application to reconstruct the hidden file using its original extension and stored size.

## Project Structure

```text
.
├── main.c         # Entry point and command-line argument handling
├── encode.c       # File validation, capacity checks, and encoding logic
├── encode.h       # Encoding structures and function declarations
├── decode.c       # Magic string verification and data extraction
├── decode.h       # Decoding structures and function declarations
├── common.h       # Common definitions, including MAGIC_STRING
├── types.h        # Status and operation type definitions
├── beautiful.bmp  # Sample 24-bit BMP image
└── secret.txt     # Sample secret file
```

## Requirements

- GCC compiler
- Linux environment or another compatible environment with GCC
- An uncompressed 24-bit BMP image
- A secret file with a supported extension

## Build Instructions

Compile the source files using GCC:

```bash
gcc -Wall -Wextra -o Steganography main.c encode.c decode.c
```

The `-Wall` and `-Wextra` options enable additional compiler warnings that can help identify potential problems.

## Usage

### 1. Encode a Secret File

```bash
./Steganography -e <source.bmp> <secret_file> [output.bmp]
```

Arguments:

- `-e`: Selects encoding mode.
- `<source.bmp>`: Original BMP image.
- `<secret_file>`: File to hide inside the image.
- `[output.bmp]`: Optional output image filename.

Example:

```bash
./Steganography -e beautiful.bmp secret.txt stego.bmp
```

This embeds `secret.txt` into `beautiful.bmp` and generates `stego.bmp`.

### 2. Decode a Secret File

```bash
./Steganography -d <stego.bmp> [output_file]
```

Arguments:

- `-d`: Selects decoding mode.
- `<stego.bmp>`: Image containing hidden data.
- `[output_file]`: Optional filename for the recovered file.

Example:

```bash
./Steganography -d stego.bmp decoded.txt
```

This extracts the hidden file and saves the recovered contents to `decoded.txt`.

### 3. Verify the Recovered File

Display the recovered text:

```bash
cat decoded.txt
```

Expected output for the sample file:

```text
hello hii
```

You can also compare the original and recovered files:

```bash
cmp secret.txt decoded.txt
```

If `cmp` produces no output and returns a successful exit status, the files are identical.

## Limitations

- **Image format:** Supports uncompressed 24-bit BMP images with the expected 54-byte header layout.
- **File extension:** The secret file extension is limited to four characters, including the dot, such as `.txt`, `.csv`, or `.c`.
- **Image capacity:** The maximum payload depends on the image dimensions, pixel-data layout, and metadata overhead.
- **No encryption:** The secret data is hidden but not encrypted. Someone familiar with the encoding format may be able to extract it.
- **BMP compatibility:** BMP files with different header layouts or unsupported pixel formats may not work correctly.

For sensitive information, encrypt the secret file before encoding it.

## Concepts Demonstrated

This project demonstrates practical applications of:

- **Bitwise operations:** Bit masking, shifting, and least significant bit manipulation.
- **File handling in C:** Binary file operations using `fopen()`, `fread()`, `fwrite()`, and `fseek()`.
- **Image file formats:** BMP header structure and pixel-data handling.
- **Modular programming:** Organizing a C application into source files and header files.
- **Data representation:** Encoding and decoding bytes, file sizes, and metadata.
- **Input validation:** Checking arguments, file extensions, and available image capacity.
- **Error handling:** Detecting invalid operations and handling file-processing failures.

## Future Improvements

- Support additional BMP formats, including 8-bit and 32-bit images.
- Add support for other image formats where appropriate.
- Implement password-protected encryption before encoding.
- Strengthen validation of malformed, truncated, or unsupported image files.
- Improve bounds checking during decoding.
- Add a Makefile for simplified compilation.
- Add automated tests for encoding, decoding, invalid inputs, and insufficient image capacity.

## Author

**Vitthal Patil**
