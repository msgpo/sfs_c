/*
SFS In C Project:
Unit tests to investigate using the libgcrypt library to satisfy the requirements
of the SFS crypto in C

Requires installation of:
libgcrypt20-dev
*/

#include <gcrypt.h>
#include <check.h>
#include "../src/sfs_crypt_gcry.c"

START_TEST(InitGCrypt) {
    //  This testing was done as of version 1.8.1 installed on my machine and also Github task runner
    const char *v = gcry_check_version(GRYPT_VERSION);
    printf("Version Check:  %s\n", v);

    fail_if(!v);
}
END_TEST

START_TEST(OpenCipherHandle) {

    //  See also https://gnupg.org/documentation/manuals/gcrypt/Working-with-cipher-handles.html#Working-with-cipher-handles
    gcry_cipher_hd_t handle;
    gcry_cipher_open(&handle, GCRY_CIPHER_AES256, GCRY_CIPHER_MODE_CBC, GCRY_CIPHER_SECURE);

    fail_if(handle == NULL, "System should have created cipher");

    gcry_cipher_close(handle);

}
END_TEST

START_TEST(DoAESEncrypt) {

    char * txtBuffer = "1234567890123456";
    char *cipherText = 
        sfs_encrypt(txtBuffer, "one test AES keyone test AES key", 16);

    printf("cipherText:  %s\n", cipherText);
    //printf("CipherText:(%ld v %ld)  '%s'\n", strlen(cipherText), strlen(txtBuffer), cipherText);

    printf("Decrypting Now...\n");
    char *outBuffer = sfs_decrypt(cipherText, "one test AES keyone test AES key", 16);

    printf("OutBuff:  %s\n", outBuffer);
    fail_if(memcmp(outBuffer, txtBuffer, strlen(txtBuffer)) != 0);


}
END_TEST

START_TEST(DoATwoFishEncrypt) {

    gcry_error_t error;
    
    size_t keyLength = gcry_cipher_get_algo_keylen(GCRY_CIPHER_TWOFISH);
    char *key = malloc(sizeof(char)*keyLength);
    char *password = "secret123";
    char *salt = "fasfasfasdfsf";

    error = gcry_kdf_derive(password, strlen(password), GCRY_KDF_SCRYPT, GCRY_KDF_SIMPLE_S2K, salt, strlen(salt), 100, keyLength, key);
    fail_if(error);
    printf("(%s)\tKey='%s'\n", password, key);

    //  Based on code found here
    //  https://cboard.cprogramming.com/c-programming/105743-how-decrypt-encrypt-using-libgcrypt-arc4.html#post937372
    

    char * txtBuffer = "123456789 abcdefghijklmnopqrstuvwzyz ABCDEFGHIJKLMNOPQRSTUVWZYZ";
    size_t txtLength = strlen(txtBuffer)+1; // string plus termination
    char * encBuffer = malloc(txtLength);
    char * outBuffer = malloc(txtLength);

    //  See also https://gnupg.org/documentation/manuals/gcrypt/Working-with-cipher-handles.html#Working-with-cipher-handles
    gcry_cipher_hd_t handle;
    error = gcry_cipher_open(&handle, GCRY_CIPHER_TWOFISH, GCRY_CIPHER_MODE_CBC, GCRY_CIPHER_SECURE);
    fail_if(error);

    
    

    error = gcry_cipher_setkey(handle, key, keyLength);
    fail_if(error);

    printf("Result of keyset:  %d\n", error);

    char *iv = "a test ini value";

    error = gcry_cipher_setiv(handle, iv, 16);
    fail_if(error);

    error = gcry_cipher_encrypt(handle, encBuffer, txtLength, txtBuffer, txtLength);
    fail_if(error);

    char *ivBytesIHope = malloc(16*sizeof(char));
    memcpy(ivBytesIHope, encBuffer, 16);
    printf("IV Bytes?  '%s'\n", ivBytesIHope);
    printf("Ciphertext:  '%s'\n", encBuffer);

    error = gcry_cipher_setiv(handle, iv, 16);
    fail_if(error);

    error = gcry_cipher_decrypt(handle, outBuffer, txtLength, encBuffer, txtLength);
    fail_if(error);

    printf("OutBuff:  %s\n", outBuffer);
    fail_if(memcmp(outBuffer, txtBuffer, txtLength) != 0);

    gcry_cipher_close(handle);

}
END_TEST

int main(int argc, char const *argv[])
{
    Suite *suite = suite_create("Crypto Stuff");
    SRunner *runner = srunner_create(suite);

    TCase *case1 = tcase_create("Cipher Fiddling Around");

    tcase_add_test(case1, InitGCrypt);
    tcase_add_test(case1, OpenCipherHandle);
    tcase_add_test(case1, DoAESEncrypt);
    tcase_add_test(case1, DoATwoFishEncrypt);
    suite_add_tcase(suite, case1);

    srunner_run_all(runner, CK_ENV);

    int failCount = srunner_ntests_failed(runner);
    return failCount == 0 ? 0 : -1;

    return 0;
}
