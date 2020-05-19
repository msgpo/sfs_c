#include "sfs_crypt.h"
#include <gcrypt.h>
#include "sfs_about.h"

//  Initialization vectors are always 16 bytes long!
#define IV_LEN 16

#define GRYPT_VERSION "1.8.1"

int sfs_startup() {
    const char *v = gcry_check_version(GRYPT_VERSION);

    printf("%s\n%s\n", SFS_C_ABOUT, SFS_C_VERSION);
    printf("libgcrypt Version Check:  %s\n", v);

    if(!v) {
        return 9000;
    }

    return 0;
}

/**
 * Perform key stretching/derivation to convert the given password into a key for encryption
 */
static void *derivePassword(char *password) {
    gcry_error_t err;
    gcry_md_hd_t digest;
    err = gcry_md_open(&digest, GCRY_MD_SHA256, GCRY_MD_FLAG_SECURE);
    if (err != 0){
        printf("%s\n", gcry_strerror(err));
        return NULL;
    }

    //  Add additional algorithm, SHA3:
    err = gcry_md_enable(digest, GCRY_MD_SHA3_256);
    
    if (err != 0){
        printf("%s\n", gcry_strerror(err));
        return NULL;
    }

    gcry_md_write(digest, password, strlen(password));

    void *hashed = gcry_md_read(digest, GCRY_MD_SHA256);

    //  Now derive key
    void *generatedKey = gcry_malloc_secure(32);
    char *salt = "abcdefghijklmno";
    err = gcry_kdf_derive(hashed, 32, GCRY_KDF_SCRYPT, GCRY_KDF_PBKDF2, salt, strlen(salt), 20, 32, generatedKey);
    
    if (err != 0){
        printf("%s\n", gcry_strerror(err));
        return NULL;
    }

    gcry_md_close(digest);

    return generatedKey;
}

char * sfs_encrypt(char *data, char *password, int length){

    void *key = derivePassword(password);

    //  Based on code found here
    //  https://cboard.cprogramming.com/c-programming/105743-how-decrypt-encrypt-using-libgcrypt-arc4.html#post937372
    gcry_error_t error;
    char * txtBuffer = data;
    char * encBuffer = malloc(length);

    //  See also https://gnupg.org/documentation/manuals/gcrypt/Working-with-cipher-handles.html#Working-with-cipher-handles
    gcry_cipher_hd_t handle;
    error = gcry_cipher_open(&handle, GCRY_CIPHER_AES256, GCRY_CIPHER_MODE_CBC, GCRY_CIPHER_SECURE);
    size_t keyLength = gcry_cipher_get_algo_keylen(GCRY_CIPHER_AES256);
    error = gcry_cipher_setkey(handle, key, keyLength);

    //  See also https://www.gnupg.org/(es)/documentation/manuals/gcrypt/Random-Numbers.html#Random-Numbers
    unsigned char *iv = gcry_random_bytes_secure(IV_LEN, GCRY_VERY_STRONG_RANDOM);

    error = gcry_cipher_setiv(handle, iv, 16);
    error = gcry_cipher_encrypt(handle, encBuffer, length, txtBuffer, length);

    char *finalProduct = malloc(length + IV_LEN);
    memcpy(&finalProduct[0], iv, IV_LEN);
    memcpy(&finalProduct[IV_LEN], &encBuffer[0], length);
    
    gcry_cipher_close(handle);

    free(encBuffer);

    return finalProduct;
}

char * sfs_decrypt(char *cipherText, char *password, int length) {
    
    void *key = derivePassword(password);

    gcry_error_t error;

    char * outBuffer = malloc(length);

    //  See also https://gnupg.org/documentation/manuals/gcrypt/Working-with-cipher-handles.html#Working-with-cipher-handles
    gcry_cipher_hd_t handle;
    error = gcry_cipher_open(&handle, GCRY_CIPHER_AES256, GCRY_CIPHER_MODE_CBC, GCRY_CIPHER_SECURE);
    size_t keyLength = gcry_cipher_get_algo_keylen(GCRY_CIPHER_AES256);

    error = gcry_cipher_setkey(handle, key, keyLength);
    printf("Result of keyset:  %d\n", error);

    char *iv = malloc(IV_LEN);
    memcpy(iv, cipherText, IV_LEN);
    error = gcry_cipher_setiv(handle, iv, 16);
    printf("Result of iv set (%s):  %d\n", iv, error);

    char *cipherTextProper = &cipherText[IV_LEN];

    error = gcry_cipher_decrypt(handle, outBuffer, length, cipherTextProper, length);
    printf("Result of decrypt:  %d\n", error);

    gcry_cipher_close(handle);

    return outBuffer;
}