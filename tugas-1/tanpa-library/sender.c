#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#define PORT 8080
#define BLOCK_SIZE 16
#define EXPANSION_KEY_SIZE 176
const uint8_t KEY[] =
    {
        0x4f, 0x6e, 0x65, 0xc9, 0x54, 0x77, 0x6f, 0x59, 0x54, 0x77, 0x6f, 0xd6, 0x4e, 0x69, 0x6e, 0x65};
const uint8_t SBOX[] =
    {
        0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
        0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
        0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
        0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
        0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
        0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
        0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
        0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
        0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
        0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
        0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
        0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
        0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
        0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
        0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
        0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16};
const uint8_t RCON[] =
    {
        0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36};

struct sockaddr_in address;
int server_fd,
    new_socket,
    valread,
    opt = 1,
    addrlen = sizeof(address);
FILE *fileptr;

void expand_key(uint8_t *expanded_key_buffer)
{
    int i, j, k;
    uint8_t temp[4];

    for (i = 0; i < 4; i++)
    {
        expanded_key_buffer[(i * 4) + 0] = KEY[(i * 4) + 0];
        expanded_key_buffer[(i * 4) + 1] = KEY[(i * 4) + 1];
        expanded_key_buffer[(i * 4) + 2] = KEY[(i * 4) + 2];
        expanded_key_buffer[(i * 4) + 3] = KEY[(i * 4) + 3];
    }

    for (i = 4; i < 44; i++)
    {
        k = (i - 1) * 4;
        temp[0] = expanded_key_buffer[k + 0];
        temp[1] = expanded_key_buffer[k + 1];
        temp[2] = expanded_key_buffer[k + 2];
        temp[3] = expanded_key_buffer[k + 3];

        if (i % 4 == 0)
        {
            {
                const uint8_t u8temp1 = temp[0];
                temp[0] = temp[1];
                temp[1] = temp[2];
                temp[2] = temp[3];
                temp[3] = u8temp1;
            }

            {
                temp[0] = SBOX[temp[0]];
                temp[1] = SBOX[temp[1]];
                temp[2] = SBOX[temp[2]];
                temp[3] = SBOX[temp[3]];
            }

            temp[0] = temp[0] ^ RCON[i / 4];
        }

        j = i * 4;
        k = (i - 4) * 4;

        expanded_key_buffer[j + 0] = expanded_key_buffer[k + 0] ^ temp[0];
        expanded_key_buffer[j + 1] = expanded_key_buffer[k + 1] ^ temp[1];
        expanded_key_buffer[j + 2] = expanded_key_buffer[k + 2] ^ temp[2];
        expanded_key_buffer[j + 3] = expanded_key_buffer[k + 3] ^ temp[3];
    }
}

void add_round_key(int round, uint8_t *encrypt_buffer, uint8_t *expanded_key_buffer)
{
    uint8_t i, j;

    for (i = 0; i < 4; i++)
    {
        for (j = 0; j < 4; j++)
        {
            encrypt_buffer[i * 4 + j] ^= expanded_key_buffer[(round * 16) + (i * 4) + j];
        }
    }
}

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

    // Rotate third row 3 columns to left
    temp = encrypt_buffer[4 * 0 + 3];
    encrypt_buffer[4 * 0 + 3] = encrypt_buffer[4 * 3 + 3];
    encrypt_buffer[4 * 3 + 3] = encrypt_buffer[4 * 2 + 3];
    encrypt_buffer[4 * 2 + 3] = encrypt_buffer[4 * 1 + 3];
    encrypt_buffer[4 * 1 + 3] = temp;
}

static uint8_t xtime(uint8_t x)
{
    return ((x << 1) ^ (((x >> 7) & 1) * 0x1b));
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

        if (read_length < 1)
            break;

        encrypt(counter, encrypt_buffer, expanded_key_buffer);

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

    send_file();

    // valread = read( new_socket , buffer, 1024);
    // printf("%s\n",buffer );
    // send(new_socket , hello , strlen(hello) , 0 );
    close(server_fd);
    close(new_socket);
    fclose(fileptr);
    return 0;
}