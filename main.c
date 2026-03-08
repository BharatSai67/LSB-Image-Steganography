/*
Name : SATTI BHARAT SAI NAGA BABU
Id : 25036A-072
Batch Id : 25036A + 39
Project : LSB IMAGE STEGANOGRAPHY
Description:
    This project implements a simple steganography technique to hide
    secret text messages inside an image file. The main objective of
    this project is to provide secure communication by embedding data
    into the least significant bits (LSB) of image pixels.

    The program allows the user to:
    1. Encode (hide) a secret message inside an image.
    2. Decode (extract) the hidden message from the image.

    This technique ensures that the visual appearance of the image
    remains unchanged while securely storing confidential information at using your password(magic string)
*/

#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "decode.h"
#include "types.h"

OperationType check_operation_type(char *);

int main(int argc, char *argv[])
{
    //check if sufficient arguments are passed
    if(argc < 2) {
        printf("Error : Insufficient arguments\n");
        printf("Usage : \n");
        printf("To Encode : ./a.out -e <source.bmp> <secret_file> [<output.bmp>]\n");
        printf("To Decode : ./a.out -d <stego_image> <output_file>\n");
        return 1;
    }
    //check the operation type
    OperationType op_type = check_operation_type(argv[1]);
    //check if the operation type is e_encode
    if(op_type == e_encode) {
        //declare structure variable EncodeInfo encInfo
        EncodeInfo encInfo;
        printf("Encoding selected\n");
        //read and validate the encode arguments
        if(read_and_validate_encode_args(argv, &encInfo) == e_failure) {
            printf("Error : Invalid arguments for encoding\n");
            return 1;
        }
        //call do_encoding
        if(do_encoding(&encInfo) == e_failure) {
            printf("Error : Encoding process failed\n");
            return 1;  
        }
        printf("Encoding completed successful\n");
    }
    // check if the operation type is e_decode
    else if(op_type == e_decode) {
        // declare structure variable DecodeInfo decoInfo;
        DecodeInfo decoInfo;
        printf("Decoding Selected\n");
        // read and validate the decode arugments
        if(read_and_validate_decode_args(argv, &decoInfo) == e_failure) {
            printf("Error : Invalid arugments for decoding\n");
            return 1;
        }
        // call do_decoding
        if(do_decoding(&decoInfo) == e_failure) {
            printf("Error : Decoding process failed\n");
            return 1;
        }
        printf("Decoding completed successful\n");
    }
    // if the operation type is e_unsupported
    else {
        printf("Error : Unsupported operation\n");
        printf("Use -e for encoding and -d for decoding\n");
        return 1;
    }
    return 0;
}
// function to check the operation type based on the command line argument
OperationType check_operation_type(char *symbol)
{
    // compare the symbol with -e for encoding
    if(strcmp(symbol, "-e") == 0) {
        return e_encode;
    }
    // compare the symbol with -d for decoding
    else if(strcmp(symbol, "-d") == 0) {
        return e_decode;
    }
    // if the symbol does not match for -e or -d we to return e_unsupported
    else {
        return e_unsupported;
    }  
}
