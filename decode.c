#include <stdio.h>
#include <string.h>

#include "decode.h"
#include "types.h"
#include "common.h"

/* Function Definitions */

// read and validate decode arugments from argv
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
    // step 1 -> stego image file name check
    if(strstr(argv[2], ".bmp") == NULL) {
        printf("Error : stego image file is not a .bmp file\n");
        return e_failure;
    }
    // store stego image file name
    decInfo->stego_image_fname = argv[2];

    // step 2 -> output file name check
    if(argv[3]) {
        if(strstr(argv[3], ".") == NULL) {
            printf("Error : Output file must have an extension\n");
            return e_failure;
        }
        strcpy(decInfo->output_fname, argv[3]); // store output file name
    } else {
        strcpy(decInfo->output_fname, "decoded_secret"); // default name
    }
    // step 3 -> return success
    return e_success;
}
// get file pointer for stego image and output file
Status open_files_decode(DecodeInfo *decInfo) 
{
    // stego image file
    decInfo->fptr_stego_image = fopen(decInfo->stego_image_fname, "rb");
    if (decInfo->fptr_stego_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", decInfo->stego_image_fname);
        return e_failure;
    }
    // No failure return e_success
    return e_success;
}
// Decode Magic String from stego image data and compare with MAGIC_STRING
Status decode_magic_string(const char *magic_string, DecodeInfo *decInfo) 
{
    // step 1 -> create a string pointer and pointer to image data buffer
    char image_buffer[8];
    char ch;
    for(int i = 0; magic_string[i] != '\0'; i++) {
        // step 2 -> read 8 bytes from stego image to image data buffer using fread() function
        fread(image_buffer, sizeof(char), 8, decInfo->fptr_stego_image);
        // step 3 -> decode a byte of magic string from image data buffer using decode byte function
        decode_byte_to_lsb(&ch, image_buffer);
        // step 4 -> compare decoded byte with magic string byte, if not same we have to return failure
        if(ch != magic_string[i]) {
            printf("Error : Magic string mismatch. Decoding failed.\n");
            return e_failure;
        }
    }
    // step 5 -> return success
    return e_success;
}
// Decode extension size of sceret file from stego image data
Status decode_secret_file_extn_size(int *size, DecodeInfo *decInfo) 
{
    // step 1 -> create a pointer to image data buffer
    char image_buffer[32];
    // step 2 -> read 32 bytes from stego image to image data buffer using fread() funtion
    fread(image_buffer, sizeof(char), 32, decInfo->fptr_stego_image);
    // step 3 -> decode a size from image data buffer using decode size function
    decode_size_to_lsb(size, image_buffer);
    // step 4 -> return success
    return e_success;
}
// Decode secret file extenstion from stego image data
Status decode_secret_file_extn(int size, DecodeInfo *decInfo) 
{
    // step 1 -> create a pointer to image data buffer and a variable to store decoded file extension size
    char image_buffer[8];
    char ch;
    // step 2 -> decode extension characters one by one from stego image data and store in file extension string
    for(int i = 0; i < size; i++) {
        // read 8 bytes from stego image to image data buffer using fread() function
        fread(image_buffer, sizeof(char), 8, decInfo->fptr_stego_image);
        // decode one character of file extension from image data buffer using decode byte function
        decode_byte_to_lsb(&ch, image_buffer);
        // store decode character in file extension string
        decInfo->extn_secret_file[i] = ch;
    }
    decInfo->extn_secret_file[size] = '\0'; // null terminate the string
    // step 4 -> create output file name by concatenating decoded file extension to (decoded_output) string
    strcat(decInfo->output_fname, decInfo->extn_secret_file);
    // step 5 -> open output file in write mode and store file pointer in decode info structure
    decInfo->fptr_output = fopen(decInfo->output_fname, "wb");
    // step 6 -> if fopen() returns null we have to return failure
    if(decInfo->fptr_output == NULL) {
        perror("fopen");
        fprintf(stderr, "Error : Unable to open output file %s\n", decInfo->output_fname);
        return e_failure;
    }
    // step 6 -> return success
    return e_success;
}
// Decode secret file size from stego image data
Status decode_secret_file_size(DecodeInfo *decInfo) 
{
    // step 1 -> create a pointer to image data buffer
    char image_buffer[32];
    int file_size;
    // step 2 -> read 32 bytes from stego image to image data buffer using fread() function
    fread(image_buffer, sizeof(char), 32, decInfo->fptr_stego_image);
    // step 3 -> decode a size from image data buffer using decode size function
    decode_size_to_lsb(&file_size, image_buffer);
    // step 4 -> store decode secret file size in decode info structure
    decInfo->size_secret_file = file_size;
    // step 5 -> return success
    return e_success;
}
// Decode secret file data from setgo image data and write to output file
Status decode_secret_file_data(DecodeInfo *decInfo) 
{
    // step 1 -> create a pointer to image data buffer and a variable to store decoded byte
    char image_buffer[8];
    char ch;
    // step 2 -> decode secret file data byte by byte from stego image data and write to output file
    for(int i = 0; i < decInfo->size_secret_file; i++) {
        // read 8 bytes from stego image to image data buffer using fread() function
        fread(image_buffer, sizeof(char), 8, decInfo->fptr_stego_image);
        // decode one byte of secret file data from image data buffer using decode byte function
        decode_byte_to_lsb(&ch, image_buffer);
        // write decoded byte to output file using fwrite() function
        fwrite(&ch, sizeof(char), 1, decInfo->fptr_output);
    }
    return e_success;
}
// Decode a byte into LSB of image data array  
Status decode_byte_to_lsb(char *data, char *image_buffer) 
{
    // step 1 -> create a variable to store decoded data
    *data = 0;
    // step 2 -> decode a byte from lsb of image data array using bitwise operation
    for(int i = 0; i < 8; i++) {
        *data = *data << 1; // left shift the data variable by 1 bit
        *data = *data | (image_buffer[i] & 0x01); // get the lsb bit and OR it with data variable
    }
    // step 3 -> return success
    return e_success;
}
// Decode a size to lsb of image data array
Status decode_size_to_lsb(int *size, char *imageBuffer) 
{
    // step 1 -> create a variable to store decoded size
    *size = 0;
    // step 2 -> decode a size from lsb of image data array using bitwise operation
    for(int i = 0; i < 32; i++) {
        *size = *size << 1; // left shift the size variable by 1 bit
        *size = *size | (imageBuffer[i] & 0x01); // get the lsb bit and OR it with size variable
    }
    // step 3 -> return success
    return e_success;
}
/* Perform the decoding */
Status do_decoding(DecodeInfo *decInfo) 
{
    printf("  Starting Decoding process...\n");
    // step 1 -> call open_files_decode() function to open stego image file and output file
    printf("   Opening Files...\n");
    if(open_files_decode(decInfo) == e_failure) {
        printf("Error in opening files\n");
        return e_failure;
    }
    // step 2 -> skip the header of stego image file using fseek() function
    fseek(decInfo->fptr_stego_image, 54, SEEK_SET);

    // step 3 -> call decode_magic_string() function to decode magic string from stego image data, if decoding fails we have to return failure
    printf("   Decoding magic string from stego image data...\n");
    if(decode_magic_string(MAGIC_STRING, decInfo) == e_failure) {
        return e_failure;
    }
    // step 4 -> call decode_secret_file_extn_size() function to decode secret file extension size from stego image data, if decoding fails we have to return failure
    printf("   Decoding secret file extension size from stego image data...\n");
    if(decode_secret_file_extn_size(&decInfo->size_extn, decInfo) == e_failure) {
        return e_failure;
    }
    // step 5 -> call decode_secret_file_extn() function to decode secret file extension from stego image data, if decoding fails we have to return failure
    printf("   Decoding secret file extension from stego image data...\n");
    if(decode_secret_file_extn(decInfo->size_extn, decInfo) == e_failure) {
        return e_failure;
    }
    // step 6 -> call decode_secret_file_size() function to decode secret file size from stego image data, if decoding fails we have to return failure
    printf("   Decoding secret file size from stego image data...\n");
    if(decode_secret_file_size(decInfo) == e_failure) {
        return e_failure;
    }
    // step 7 -> call decode_secret_file_data() function to decode secret file data from stego image data, if decoding fails we have to return failure
    printf("   Decoding secret file data from stego image data...\n");
    if(decode_secret_file_data(decInfo) == e_failure) {
        return e_failure;
    }
    // step 8 -> return success
    return e_success;
}