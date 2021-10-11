#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "custom-lib.h"
#include "rsa.c"

#define DEV 0

struct sockaddr_in server_address;
int sock,
    valread,
    opt = 1,
    addrlen = sizeof(server_address);
FILE *fileptr;

void receive_file()
{
#if DEV == 0
    int i, length;
    uint8_t decrypt_buffer[BLOCK_SIZE],
        file_buffer[BLOCK_SIZE],
        expanded_key_buffer[EXPANSION_KEY_SIZE],
        counter[BLOCK_SIZE] = {0},
        key[BLOCK_SIZE] = {0},
        encrypted_key[BLOCK_SIZE] = {0},
        temp_key[BLOCK_SIZE] = {0};

    memset(key, 0, BLOCK_SIZE);
    recv(sock, encrypted_key, BLOCK_SIZE, 0);
    rsa_decrypt(temp_key, encrypted_key);

    for (i = 0; i < BLOCK_SIZE; i++)
        key[i] = temp_key[i];

    memset(file_buffer, 0, BLOCK_SIZE);
    expand_key(expanded_key_buffer, key);

    while ((length = recv(sock, file_buffer, BLOCK_SIZE, 0)) > 0)
    {
        encrypt(counter, decrypt_buffer, expanded_key_buffer);

        // print_number(decrypt_buffer, BLOCK_SIZE);
        // print_number(file_buffer, BLOCK_SIZE);
        for (i = 0; i < length; i++)
            file_buffer[i] ^= decrypt_buffer[i];
        // print_number(file_buffer, BLOCK_SIZE);

        change_counter(counter);

        fwrite(file_buffer, sizeof(uint8_t), length, fileptr);
        memset(file_buffer, 0, BLOCK_SIZE);
        memset(decrypt_buffer, 0, BLOCK_SIZE);
    }
#else
    uint8_t file_buffer[BLOCK_SIZE];
    recv(sock, file_buffer, BLOCK_SIZE, 0);
    print_number(file_buffer, BLOCK_SIZE);
    // inverse_shift_rows(file_buffer);
    // inverse_subtitute_bytes(file_buffer);
    inverse_mix_columns(file_buffer);
    print_number(file_buffer, BLOCK_SIZE);
    puts(file_buffer);
#endif
}

int main(int argc, char const *argv[])
{
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

    close(sock);
    fclose(fileptr);
    return 0;
}