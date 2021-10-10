Cara run :
1. run `gcc main_version2.c -o main_version2 -lm -lcrypto -lssl` di terminal
2. run `./main_version2` di terminal
3. Keterangan inputan :
- Masukkan angka 1 jika ingin encrypt file
- Masukkan angka 2 jika ingin decrypt file
- Masukkan angka 3 jika sudah selesai

UPDATED :
4. `gcc senderMain.c -o sender -lm -lcrypto -lssl`
5. `gcc receiverMain.c -o receiver -lm -lcrypto -lssl`
6. `./sender input.txt`
7. `./receiver recovered.txt`
