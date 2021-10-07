#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include "custom-lib.h"

#define DEV 0

struct sockaddr_in address;
int server_fd,
    new_socket,
    valread,
    opt = 1,
    addrlen = sizeof(address);
FILE *fileptr;

void subtitute_bytes(uint8_t *encrypt_buffer)
{
    uint8_t i, j;

    for (i = 0; i < 4; i++)
    {
        for (j = 0; j < 4; j++)
        {
            encrypt_buffer[j * 4 + i] = SBOX[encrypt_buffer[j * 4 + i]];
        }
    }
}

void shift_rows(uint8_t *encrypt_buffer)
{
    uint8_t temp;

    temp = encrypt_buffer[4 * 0 + 1];
    encrypt_buffer[4 * 0 + 1] = encrypt_buffer[4 * 1 + 1];
    encrypt_buffer[4 * 1 + 1] = encrypt_buffer[4 * 2 + 1];
    encrypt_buffer[4 * 2 + 1] = encrypt_buffer[4 * 3 + 1];
    encrypt_buffer[4 * 3 + 1] = temp;

    temp = encrypt_buffer[4 * 0 + 2];
    encrypt_buffer[4 * 0 + 2] = encrypt_buffer[4 * 2 + 2];
    encrypt_buffer[4 * 2 + 2] = temp;

    temp = encrypt_buffer[4 * 1 + 2];
    encrypt_buffer[4 * 1 + 2] = encrypt_buffer[4 * 3 + 2];
    encrypt_buffer[4 * 3 + 2] = temp;

    temp = encrypt_buffer[4 * 0 + 3];
    encrypt_buffer[4 * 0 + 3] = encrypt_buffer[4 * 3 + 3];
    encrypt_buffer[4 * 3 + 3] = encrypt_buffer[4 * 2 + 3];
    encrypt_buffer[4 * 2 + 3] = encrypt_buffer[4 * 1 + 3];
    encrypt_buffer[4 * 1 + 3] = temp;
}

void mix_columns(uint8_t *encrypt_buffer)
{
    uint8_t i, temp1, temp2, temp3;

    for (i = 0; i < 4; i++)
    {
        temp3 = encrypt_buffer[i * 4 + 0];
        temp1 = encrypt_buffer[i * 4 + 0] ^ encrypt_buffer[i * 4 + 1] ^ encrypt_buffer[i * 4 + 2] ^ encrypt_buffer[i * 4 + 3];
        temp2 = encrypt_buffer[i * 4 + 0] ^ encrypt_buffer[i * 4 + 1];
        temp2 = xtime(temp2);
        encrypt_buffer[i * 4 + 0] ^= temp2 ^ temp1;
        temp2 = encrypt_buffer[i * 4 + 1] ^ encrypt_buffer[i * 4 + 2];
        temp2 = xtime(temp2);
        encrypt_buffer[i * 4 + 1] ^= temp2 ^ temp1;
        temp2 = encrypt_buffer[i * 4 + 2] ^ encrypt_buffer[i * 4 + 3];
        temp2 = xtime(temp2);
        encrypt_buffer[i * 4 + 2] ^= temp2 ^ temp1;
        temp2 = encrypt_buffer[i * 4 + 3] ^ temp3;
        temp2 = xtime(temp2);
        encrypt_buffer[i * 4 + 3] ^= temp2 ^ temp1;
    }
}

void encrypt(uint8_t *counter, uint8_t *encrypt_buffer, uint8_t *expanded_key_buffer)
{
    int i;

    memcpy(encrypt_buffer, counter, BLOCK_SIZE);

    add_round_key(0, encrypt_buffer, expanded_key_buffer);

    for (i = 1; i <= 10; i++)
    {
        subtitute_bytes(encrypt_buffer);
        shift_rows(encrypt_buffer);
        if (i != 10)
            mix_columns(encrypt_buffer);
        add_round_key(i, encrypt_buffer, expanded_key_buffer);
    }
}

void send_file()
{
    int read_length, i;
    uint8_t file_buffer[BLOCK_SIZE],
        counter[BLOCK_SIZE] = {0},
        encrypt_buffer[BLOCK_SIZE],
        expanded_key_buffer[EXPANSION_KEY_SIZE];

    expand_key(expanded_key_buffer);

    while (1)
    {
        memset(file_buffer, 0, BLOCK_SIZE);
        read_length = fread(file_buffer, sizeof(uint8_t), BLOCK_SIZE, fileptr);
        printf("%d\n", read_length);

        if (read_length < 1)
            break;

        encrypt(counter, encrypt_buffer, expanded_key_buffer);

        for (i = 0; i < read_length; i++)
            file_buffer[i] ^= encrypt_buffer[i];

        printf("%ld\n", sizeof(file_buffer));

        send(new_socket, file_buffer, sizeof(file_buffer), 0);

        change_counter(counter);
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

    fileptr = fopen(argv[1], "rb");
    if (!fileptr)
    {
        puts("File cannot be opened!");
        exit(EXIT_FAILURE);
    }

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        puts("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        puts("setsockopt failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        puts("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0)
    {
        puts("listen failed");
        exit(EXIT_FAILURE);
    }

    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
    {
        puts("accept failed");
        exit(EXIT_FAILURE);
    }

    send_file();
#else
#endif

    close(server_fd);
    close(new_socket);
    fclose(fileptr);

    return 0;
}