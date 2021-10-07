#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "custom-lib.h"

#define DEV 0

struct sockaddr_in server_address;
int sock,
    valread,
    opt = 1,
    addrlen = sizeof(server_address);
FILE *fileptr;

void inverse_shift_rows(uint8_t *decrypt_buffer)
{
    uint8_t temp;

    temp = decrypt_buffer[4 * 3 + 1];
    decrypt_buffer[4 * 3 + 1] = decrypt_buffer[4 * 2 + 1];
    decrypt_buffer[4 * 2 + 1] = decrypt_buffer[4 * 1 + 1];
    decrypt_buffer[4 * 1 + 1] = decrypt_buffer[4 * 0 + 1];
    decrypt_buffer[4 * 0 + 1] = temp;

    temp = decrypt_buffer[4 * 0 + 2];
    decrypt_buffer[4 * 0 + 2] = decrypt_buffer[4 * 2 + 2];
    decrypt_buffer[4 * 2 + 2] = temp;

    temp = decrypt_buffer[4 * 1 + 2];
    decrypt_buffer[4 * 1 + 2] = decrypt_buffer[4 * 3 + 2];
    decrypt_buffer[4 * 3 + 2] = temp;

    temp = decrypt_buffer[4 * 0 + 3];
    decrypt_buffer[4 * 0 + 3] = decrypt_buffer[4 * 1 + 3];
    decrypt_buffer[4 * 1 + 3] = decrypt_buffer[4 * 2 + 3];
    decrypt_buffer[4 * 2 + 3] = decrypt_buffer[4 * 3 + 3];
    decrypt_buffer[4 * 3 + 3] = temp;
}

void inverse_subtitute_bytes(uint8_t *decrypt_buffer)
{
    uint8_t i, j;

    for (i = 0; i < 4; i++)
    {
        for (j = 0; j < 4; j++)
        {
            decrypt_buffer[j * 4 + i] = REVERSE_SBOX[decrypt_buffer[j * 4 + i]];
        }
    }
}

uint8_t multiply(uint8_t x, uint8_t y)
{
    return (((y & 1) * x) ^
            ((y >> 1 & 1) * xtime(x)) ^
            ((y >> 2 & 1) * xtime(xtime(x))) ^
            ((y >> 3 & 1) * xtime(xtime(xtime(x)))) ^
            ((y >> 4 & 1) * xtime(xtime(xtime(xtime(x))))));
}

void inverse_mix_columns(uint8_t *decrypt_buffer)
{
    int i;
    uint8_t temp1, temp2, temp3, temp4;

    for (i = 0; i < 4; ++i)
    {
        temp1 = decrypt_buffer[4 * i + 0];
        temp2 = decrypt_buffer[4 * i + 1];
        temp3 = decrypt_buffer[4 * i + 2];
        temp4 = decrypt_buffer[4 * i + 3];

        decrypt_buffer[4 * i + 0] = multiply(temp1, 0x0e) ^ multiply(temp2, 0x0b) ^ multiply(temp3, 0x0d) ^ multiply(temp4, 0x09);
        decrypt_buffer[4 * i + 1] = multiply(temp1, 0x09) ^ multiply(temp2, 0x0e) ^ multiply(temp3, 0x0b) ^ multiply(temp4, 0x0d);
        decrypt_buffer[4 * i + 2] = multiply(temp1, 0x0d) ^ multiply(temp2, 0x09) ^ multiply(temp3, 0x0e) ^ multiply(temp4, 0x0b);
        decrypt_buffer[4 * i + 3] = multiply(temp1, 0x0b) ^ multiply(temp2, 0x0d) ^ multiply(temp3, 0x09) ^ multiply(temp4, 0x0e);
    }
}

void decrypt(uint8_t *counter, uint8_t *decrypt_buffer, uint8_t *expanded_key_buffer)
{
    int i;

    memcpy(decrypt_buffer, counter, BLOCK_SIZE);

    add_round_key(0, decrypt_buffer, expanded_key_buffer);

    for (i = 1; i <= 10; i++)
    {
        inverse_shift_rows(decrypt_buffer);
        inverse_subtitute_bytes(decrypt_buffer);
        add_round_key(i, decrypt_buffer, expanded_key_buffer);
        if (i != 10)
            inverse_mix_columns(decrypt_buffer);
    }
}

void receive_file()
{
    int i, length;
    uint8_t decrypt_buffer[BLOCK_SIZE],
        file_buffer[BLOCK_SIZE],
        expanded_key_buffer[EXPANSION_KEY_SIZE],
        counter[BLOCK_SIZE] = {0};

    memset(file_buffer, 0, BLOCK_SIZE);
    expand_key(expanded_key_buffer);

    while ((length = recv(sock, file_buffer, BLOCK_SIZE, 0)) > 0)
    {
        decrypt(counter, decrypt_buffer, expanded_key_buffer);

        for (i = 0; i < length; i++)
            file_buffer[i] ^= decrypt_buffer[i];

        change_counter(counter);

        fwrite(file_buffer, sizeof(uint8_t), BLOCK_SIZE, fileptr);
        memset(file_buffer, 0, BLOCK_SIZE);
        memset(decrypt_buffer, 0, BLOCK_SIZE);
    }
}

int main(int argc, char const *argv[])
{
#if DEV == 0

    if (argc < 2)
    {
        puts("File name not specified");
        exit(EXIT_FAILURE);
    }

    fileptr = fopen(argv[1], "wb");
    if (!fileptr)
    {
        puts("File cannot be opened!");
        exit(EXIT_FAILURE);
    }

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }

    memset(&server_address, '0', sizeof(server_address));

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr) <= 0)
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        printf("\nConnection Failed \n");
        return -1;
    }

    receive_file();
#else
#endif

    close(sock);
    fclose(fileptr);
    return 0;
}