#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#define PORT 8080
#define BLOCK_SIZE 16
#define KEY                                                                                            \
    {                                                                                                  \
        0x4f, 0x6e, 0x65, 0xc9, 0x54, 0x77, 0x6f, 0x59, 0x54, 0x77, 0x6f, 0xd6, 0x4e, 0x69, 0x6e, 0x65 \
    }

struct sockaddr_in address;
int server_fd,
    new_socket,
    valread,
    opt = 1,
    addrlen = sizeof(address);
FILE *fileptr;

void encrypt(uint8_t *counter, uint8_t *encrypt_buffer)
{
}

void send_file()
{
    int read_length, i;
    uint8_t file_buffer[BLOCK_SIZE],
        counter[BLOCK_SIZE] = {0},
        encrypt_buffer[BLOCK_SIZE];

    while (1)
    {
        memset(file_buffer, 0, BLOCK_SIZE);
        read_length = fread(file_buffer, sizeof(uint8_t), BLOCK_SIZE, fileptr);

        if (read_length < 1)
            break;

        encrypt(counter, encrypt_buffer);

        for (i = 0; i < read_length; i++)
            file_buffer[i] ^= encrypt_buffer[i];

        if (send(new_socket, file_buffer, sizeof(file_buffer), 0))
        {
            puts("Error in sending file.");
            exit(EXIT_FAILURE);
        };

        for (i = BLOCK_SIZE - 1; i >= 0; i--)
        {
            counter[i]++;
            if (counter[i])
                break;
        }
    }
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

    // valread = read( new_socket , buffer, 1024);
    // printf("%s\n",buffer );
    // send(new_socket , hello , strlen(hello) , 0 );
    close(server_fd);
    close(new_socket);
    fclose(fileptr);
    return 0;
}