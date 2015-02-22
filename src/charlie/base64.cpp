#include <openssl/buffer.h>
#include <charlie/base64.h>

char* base64Encode(const unsigned char *message, const size_t length) {
    BIO *bmem, *b64;
    BUF_MEM *bptr;

    b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    bmem = BIO_new(BIO_s_mem());
    b64 = BIO_push(b64, bmem);
    BIO_write(b64, message, length);
    BIO_flush(b64);
    BIO_get_mem_ptr(b64, &bptr);

    char* buf = (char*)malloc(bptr->length);
    memcpy(buf, bptr->data, bptr->length-1);
    buf[bptr->length-1] = 0;

    BIO_free_all(b64);
    return buf;
}

int base64Decode(const char *b64message, const size_t length, unsigned char **buffer) {
    BIO *bio;
    BIO *b64;
    int decodedLength = calcDecodeLength(b64message, length);

    *buffer = (unsigned char*)malloc(decodedLength+1);
    if(*buffer == NULL) {
        fprintf(stderr, "Failed to allocate memory\n");
        exit(1);
    }


    bio = BIO_new(BIO_s_mem());
    BIO_puts(bio, b64message);
    b64 = BIO_new(BIO_f_base64());
    bio = BIO_push(b64, bio);

    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    decodedLength = BIO_read(bio, *buffer, length);
    (*buffer)[decodedLength] = '\0';

    BIO_free_all(bio);

    return decodedLength;
}

int calcDecodeLength(const char *b64input, const size_t length) {
    int padding = 0;

    // Check for trailing '=''s as padding
    if(b64input[length-1] == '=' && b64input[length-2] == '=')
        padding = 2;
    else if (b64input[length-1] == '=')
        padding = 1;

    return (int)length*0.75 - padding;
}
