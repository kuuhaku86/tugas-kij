// https://scanftree.com/programs/c/c-code-to-implement-rsa-algorithmencryption-and-decryption/

#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

// int main() {
//     uint8_t message[16]="halohalobandungg", 
//         encrypted[16] = {0};
    
//     puts(message);
//     rsa_encrypt(message, encrypted);
//     memset(message, 0, 16);
//     puts(encrypted);
//     rsa_decrypt(message, encrypted);
//     int i;
//     for ( i = 0; i < 16; i++)
//     {
//         printf("%c", message[i]);
//     }
// }

void rsa_encrypt(uint8_t *message, uint8_t *encrypted) {
	long int pt, ct, key=5, k, len, n=7*17;
	int i=0, j;

	while(i < 16) {
		pt = message[i];
		pt = pt - 96;
		k = 1;

		for (j = 0;j < key;j++) {
			k = k * pt;
			k = k % n;
		}

		encrypted[i] = k;
		i++;
	}
}

void rsa_decrypt(uint8_t *message, uint8_t *encrypted) {
	long int pt, ct, key=77, k, n=7*17;
	int i=0, j;

	while(i < 16) {
		ct = encrypted[i];
		k = 1;

		for (j = 0;j < key;j++) {
			k = k * ct;
			k = k % n;
		}

		pt = k + 96;
		message[i] = pt;
		i++;
	}
}