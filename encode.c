#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "types.h"
#include "common.h"

/* Function Definitions */

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    // Seek to 18th byte
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);
    printf("     width = %u\n", width);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);
    printf("     height = %u\n", height);

    // Return image capacity
    return width * height * 3;
}

uint get_file_size(FILE *fptr)
{
    // Find the size of secret file data
    fseek(fptr, 0, SEEK_END);
    uint size = ftell(fptr);
    // rewind the file pointer to start
    rewind(fptr);
    return size;
}
/*
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */

Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    // Step 1 - source file name check
    if(strstr(argv[2], ".bmp") == NULL) {
        printf("Error : Source file is not a .bmp file\n");
        return e_failure;
    }
    // store source file name
    encInfo->src_image_fname = argv[2];
    encInfo->secret_fname = argv[3];

    // step 2 - secret file name check
    char *extn = strrchr(argv[3], '.'); 
    if(extn == NULL) {
        printf("Error : Secret file has no extension\n");
        return e_failure;
    } 
    strcpy(encInfo->extn_secret_file, extn); // store extension

    // step 3 - stego file name check
    if(argv[4]) {
        if(strstr(argv[4], ".bmp") == NULL) {
            printf("Error : Stego file (output file) must be a (.bmp) file\n");
            return e_failure;
        }
        encInfo->stego_image_fname = argv[4]; // store stego file name
    } else {
        encInfo->stego_image_fname = "stego.bmp"; // default name
    }
    // step 4 - return success
    return e_success;
}
// get file pointer for source image, secret file and stego image
Status open_files(EncodeInfo *encInfo)
{
    // Src Image file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "rb");
    // Do Error handling
    if (encInfo->fptr_src_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);
        return e_failure;
    }
    // Secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "rb");
    // Do Error handling
    if (encInfo->fptr_secret == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);
        return e_failure;
    }
    // Stego Image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "wb");
    // Do Error handling
    if (encInfo->fptr_stego_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);
        return e_failure;
    }
    // No failure return e_success
    return e_success;
}
// Checking capacity of the image to hold your secret file data
Status check_capacity(EncodeInfo *encInfo)
{
    // step1 -> encInfo->image_capacity =get_image_size_for_bmp(source_file_pointer)
    encInfo->image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image);
    printf("     Image capacity : %u bytes\n", encInfo->image_capacity);

    // step2 -> find secret file size encInfo -> size_secret_file = get_file_size(secret file pointer)
    encInfo->size_secret_file = get_file_size(encInfo->fptr_secret);
    printf("     Secret file size : %ld bytes\n", encInfo->size_secret_file);

    printf("     Magic string length : %lu\n", sizeof(MAGIC_STRING) - 1);
    // step 3 -> total required size = (54 bytes for header) + (8* sizeof(magic string)) + (8* sizeof(extn size)) + (8* sizeof(extn)(.txt file)) + (8* sizeof(secret file size)) + (8* size of secret file data)
    uint total_required_size = ((54) + (strlen(MAGIC_STRING) * 8) + 32 + (strlen(encInfo->extn_secret_file) * 8) + (32) + ((encInfo->size_secret_file) * 8));
    printf("     Total required size for encoding : %u bytes\n", total_required_size);

    // step 4 -> if image capacity > total required size return e_success else e_failure
    if(encInfo->image_capacity >= total_required_size) {
        return e_success;
    } else {
        return e_failure;
    }
}
// copy bmp image header from source file to destination file
Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    // step 1 -> read file position to start of header using rewind() function
    rewind(fptr_src_image);

    // step 2 -> copy header (54 bytes) from source image to stego image
    char header[54];
    fread(header, sizeof(char), 54, fptr_src_image);
    fwrite(header, sizeof(char), 54, fptr_dest_image);

    // step 3 -> return scuccess
    return e_success;
}
// encode a byte into lsb of image data string array
Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
    // step 1 -> create string pointer and pointer to image data buffer
    char image_buffer[8];
    for(int i = 0; magic_string[i] != '\0'; i++) {
        // step 2 -> read 8 bytes from source image to image data buffer using fread() function
        fread(image_buffer, sizeof(char), 8, encInfo->fptr_src_image);
        // step 3 -> encode a byte of magic string to image data buffer using encode byte
        encode_byte_to_lsb(magic_string[i], image_buffer);
        // step 4 -> write encoded image data buffer to stego image using fwrite() function
        fwrite(image_buffer, sizeof(char), 8, encInfo->fptr_stego_image);
    }
    // step 5 -> return success
    return e_success;
}
// encode secret file extension size to lsb of image data string array
Status encode_secret_file_extn_size(int size, EncodeInfo *encInfo)
{
    // step 1 -> create pointer to image data buffer
    char image_buffer[32];
    // step 2 -> read 32 bytes from source image to image data buffer using fread() function
    fread(image_buffer, sizeof(char), 32, encInfo->fptr_src_image);
    // step 3 -> encode a size to image data buffer using encode size to lsb function
    encode_size_to_lsb(size, image_buffer);
    // step 4 -> write encode image data buffer to stego image using fwrite() function
    fwrite(image_buffer, sizeof(char), 32, encInfo->fptr_stego_image);
    // step 5 -> return success
    return e_success;
}
// encode secret file extension to lsb of image data to string array
Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
    // step 1 -> create pointer to image data buffer
    char image_buffer[8];
    for(int i = 0; i < strlen(file_extn); i++) {
        // step 2 -> read 8 bytes from source image to image data buffer using fread() function
        fread(image_buffer, sizeof(char), 8, encInfo->fptr_src_image);
        // step 3 -> encode a byte of file extension to image data buffer using encode byte function
        encode_byte_to_lsb(file_extn[i], image_buffer);
        // step 4 -> write encoded image data buffer to stego image using fwrite() function
        fwrite(image_buffer, sizeof(char), 8, encInfo->fptr_stego_image);
    }
    // step 5 -> return success
    return e_success;
}
// encode secret file size to lsb of image data string array
Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
{
    // step 1 -> create pointer to image data buffer
    char image_buffer[32];
    // step 2 -> read 32 bytes from source image to image data buffer using fread() funtion
    fread(image_buffer, sizeof(char), 32, encInfo->fptr_src_image);
    // step 3 -> encode a size to image data buffer to lsb using encode size to lsb function
    encode_size_to_lsb(file_size, image_buffer);
    // step 4 -> write encoded image data buffer to stego imgae using fwrite() function
    fwrite(image_buffer, sizeof(char), 32, encInfo->fptr_stego_image);
    // step 5 -> return success
    return e_success;
}
// encode secret file data to lsb of image data string array
Status encode_secret_file_data(EncodeInfo *encInfo)
{
    // step 1 -> create pointer to image data buffer
    char image_buffer[8];
    char data;
    for(int i = 0; i < encInfo->size_secret_file; i++) {
        // step 2 -> read a byte from secret file to data variable using fread() function
        fread(&data, sizeof(char), 1, encInfo->fptr_secret);
        // step 3 -> read 8 bytes from source image to image data buffer using fread() function
        fread(image_buffer, sizeof(char), 8, encInfo->fptr_src_image);
        // step 4 -> encode a byte of secret file data to image data buffer using encode byte function
        encode_byte_to_lsb(data, image_buffer);
        // step 5 -> write encoded image data buffer to stego image using fwrite() function
        fwrite(image_buffer, sizeof(char), 8, encInfo->fptr_stego_image);
    }
    // setp 6 -> return success
    return e_success;
}
// copying remaining image data from source image to stego image after encoding is done
Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    // step 1 -> create a buffer of variable to copy data from source to stego image
    char ch;
    // step 2 -> read a byte from source image to buffer upto end of  the file
    while(fread(&ch, sizeof(char), 1, fptr_src)) {
        // step 3 -> write a byte from buffer to stego image using fwrite() function
        fwrite(&ch, sizeof(char), 1, fptr_dest);
    }
    // step 4 -> return success
    return e_success;
}
// Encode a bytes into LSB of image data array
Status encode_byte_to_lsb(char data, char *image_buffer)
{
    for(int i = 0; i < 8; i++) {
        // clear the lsb bit
        image_buffer[i] = image_buffer[i] & 0xFE;
        // set the lsb bit to data bit
        image_buffer[i] = image_buffer[i] | ((data >> (7 - i)) & 1);
    }
    return e_success;
}  
// Encode a size to LSB of image data array
Status encode_size_to_lsb(int size, char *imageBuffer)
{
    for(int i = 0; i < 32; i++) {
        // clear the lsb bit
        imageBuffer[i] = imageBuffer[i] & 0xFE;
        // set the lsb bit to size bit
        imageBuffer[i] = imageBuffer[i] | ((size >> (31 - i)) & 1);
    }
    // return success
    return e_success;
}
// Perform the encoding process
Status do_encoding(EncodeInfo *encInfo)
{
    printf("  Starting Encoding process\n");
    // step 1 -> call open_files() function to open source image file, secret file and stego image file
    printf("   Opening Files...\n");
    if(open_files(encInfo) == e_failure) {
        printf("Error in opening files\n");
        return e_failure;
    }
    // step2 -> call check_capacity() function to check whether the image has enough capacity to hold secret data or not
    printf("   Checking capacity...\n");
    if(check_capacity(encInfo) == e_failure) {
        printf("Error : Image capacity does not have enough space to hold secret data\n");
        return e_failure;
    }
    // step 3 -> call copy_bmp_header() function to copy header from source image to stego image
    printf("   Copying BMP header from source image to stego image...\n"); 
    if(copy_bmp_header(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_failure) {
        printf("Error : in copying BMP header from source image to stego image\n");
        return e_failure;
    }
    // step 4 -> call encode_magic_string() function to encode magic string to image data
    printf("   Encoding magic string to image data...\n");
    if(encode_magic_string(MAGIC_STRING, encInfo) == e_failure) {
        printf("Error : in encoding magic string to image data\n");
        return e_failure;
    }
    // step 5 -> call encode_secret_file_extn_size() function to encode secret file extension size to image data
    printf("   Encoding secret file extension size to image data...\n");
    if(encode_secret_file_extn_size(strlen(encInfo->extn_secret_file), encInfo) == e_failure) {
        printf("Error : in encoding secret file extension size to image data\n");
        return e_failure;
    }
    // step 6 -> call encode_secret_file_extn() function to encode secret file extension to image data
    printf("   Encoding secret file extension to image data...\n");
    if(encode_secret_file_extn(encInfo->extn_secret_file, encInfo) == e_failure) {
        printf("Error : in encoding secret file extension to image data\n");
        return e_failure;
    }
    // step 7 -> call encode_secret file size() function to encode secret file size to image data
    printf("   Encoding secret file size to image data...\n");
    if(encode_secret_file_size(encInfo->size_secret_file, encInfo) == e_failure) {
        printf("Error : in encoding secret file size to image data\n");
        return e_failure;
    }
    // step 8 -> call encode_secret_file_data() function to encode secret file data to image data
    printf("   Encoding secret file data to image data...\n");
    if(encode_secret_file_data(encInfo) == e_failure) {
        printf("Error : in encoding secret file data to image data\n");
        return e_failure;
    }
    // step 9 -> call copy_remaining_img_data() function to copy remianing image data from source image to stego image after encoding is done
    printf("   Copying remaining image data from source image to stego image...\n");
    if(copy_remaining_img_data(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_failure) {
        printf("Error : in copying remaining image data from source image to stego image\n");
        return e_failure;
    }
    // step 10 -> return e_success after encoding is done successfully
    return e_success;
}
