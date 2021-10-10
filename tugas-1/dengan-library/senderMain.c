#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <openssl/aes.h>
#include <openssl/rand.h>
#include "aes_ctr.h"
#define PORT 8080

struct ctr_state
{
    unsigned char ivec[16];
    unsigned int num;
    unsigned char ecount[16];
};

FILE *fileptr;
size_t count;
char *buffer;
AES_KEY key;
struct sockaddr_in address;
int server_fd, new_socket, valread, opt = 1, addrlen = sizeof(address);

int bytes_read, bytes_written;
unsigned char indata[AES_BLOCK_SIZE];
unsigned char outdata[AES_BLOCK_SIZE];
unsigned char ckey[] = "afifgantengbnget"; // It is 128bits though..
unsigned char iv[8] = {0};                 //This should be generated by RAND_Bytes I will take into    consideration your previous post
struct ctr_state state;

int init_ctr(struct ctr_state *state, const unsigned char iv[8]);
void encrypt();

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

    encrypt();

    close(server_fd);
    close(new_socket);
    fclose(fileptr);

    return 0;
}

int init_ctr(struct ctr_state *state, const unsigned char iv[8])
{
    state->num = 0;
    memset(state->ecount, 0, 16);
    memset(state->ivec + 8, 0, 8);
    memcpy(state->ivec, iv, 8);
}

void encrypt()
{
    //Opening files where text plain text is read and ciphertext stored

    if (fileptr == NULL)
    {
        fputs("File error", stderr);
        exit(1);
    }

    //Initializing the encryption KEY
    AES_set_encrypt_key(ckey, 128, &key);

    //Encrypting Blocks of 16 bytes and writing the output.txt with ciphertext
    init_ctr(&state, iv); //Counter call
    while (1)
    {
        bytes_read = fread(indata, 1, AES_BLOCK_SIZE, fileptr);
        AES_ctr128_encrypt(indata, outdata, bytes_read, &key, state.ivec, state.ecount, &state.num);
        puts(outdata);
        send(new_socket, outdata, bytes_read, 0);
        if (bytes_read < AES_BLOCK_SIZE)
            break;
    }

    // fclose(fileptr);
    // free(buffer);
}