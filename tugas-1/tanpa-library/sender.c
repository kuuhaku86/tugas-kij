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

void send_file()
{
#if DEV == 0

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

        if (read_length < 1)
            break;

        encrypt(counter, encrypt_buffer, expanded_key_buffer);

        // print_number(encrypt_buffer, BLOCK_SIZE);
        // print_number(file_buffer, BLOCK_SIZE);
        for (i = 0; i < read_length; i++)
            file_buffer[i] ^= encrypt_buffer[i];
        // print_number(file_buffer, BLOCK_SIZE);

        send(new_socket, file_buffer, read_length, 0);

        change_counter(counter);
    }
#else
    uint8_t encrypt_buffer[BLOCK_SIZE] = {
        'a',
        'b',
        'c',
        'd',
        'e',
        'f',
        'g',
        'h',
        'i',
        'j',
        'k',
        'l',
        'm',
        'n',
        'o',
        'p',
    };
    // subtitute_bytes(encrypt_buffer);
    // shift_rows(encrypt_buffer);
    print_number(encrypt_buffer, BLOCK_SIZE);
    mix_columns(encrypt_buffer);
    puts(encrypt_buffer);
    print_number(encrypt_buffer, BLOCK_SIZE);
    send(new_socket, encrypt_buffer, BLOCK_SIZE, 0);
#endif
}

int main(int argc, char const *argv[])
{
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

    close(server_fd);
    close(new_socket);
    fclose(fileptr);

    return 0;
}