#ifndef DECODE_H
#define DECODE_H

#include<stdio.h>
#include<string.h>

#include "types.h" // contains user defined types

/* Structure to store information required for decoding
 * secret file from stego image
 */

typedef struct _DecodeInfo
{
    /* Secret File Info */
    char extn_secret_file[20]; 
    int size_extn;    
    int size_secret_file;

    /* Stego Image Info */
    char *stego_image_fname; 
    FILE *fptr_stego_image; 

    /* Output File Info */
    char output_fname[100];
    FILE *fptr_output;    

} DecodeInfo;

/* Decoding function prototype */

/* Read and validate Decode args from argv */
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo);

/* Open files required for decoding */
Status open_files_decode(DecodeInfo *decInfo);

/* Store Magic String */
Status decode_magic_string(const char *magic_string, DecodeInfo *decInfo);

/*Decode extension size*/
Status decode_secret_file_extn_size(int *size, DecodeInfo *decInfo);

/* Decode secret file extenstion */
Status decode_secret_file_extn(int size, DecodeInfo *decInfo);

/* Decode secret file size */
Status decode_secret_file_size(DecodeInfo *decInfo);

/* Decode secret file data*/
Status decode_secret_file_data( DecodeInfo *decInfo);

/* Decode a byte into LSB of image data array */
Status decode_byte_to_lsb(char *data, char *image_buffer);

// Decode a size to lsb
Status decode_size_to_lsb(int *size, char *imageBuffer);

/* Perform the decoding */
Status do_decoding(DecodeInfo *decInfo);

#endif
