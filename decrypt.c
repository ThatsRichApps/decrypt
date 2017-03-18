/*
 * decrypt.c
 *
 *  Created on: Mar 7, 2017
 *      Author: Richard Humphrey
 *
 *  Last edited: 2017-03-17
 */


#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>

const int KEYWORDSIZE = 10;

void print_help() {
	printf("Usage:\n");
	printf("./decrypt <binary_file> <key1> <key2>\n");
}

void print_try_help() {
	printf("Try decrypt -h for help.\n");
}

char *validate_key(char *key) {

	// validate_key

	// key should be KEYWORDSIZE characters with no duplicates
	// convert to uppercase, drop any dups, drop no alpha chars

	// convert to uppercase
	for(char* c = key; (*c=toupper(*c)); ++c) ;

	char *validkey;
	validkey = malloc(sizeof(char) * (KEYWORDSIZE + 1));

	// in order to use the C str functions, it must have a null terminator
	validkey[0] = '\0';

	// require the keyword to be part of the alphabet, upper or lower
	char *alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ\0";

	int unique_characters = 0;

	for(int i = 0; key[i] != '\0'; i++) {

		// is ch already in validkey? ch in alphabet?
		if ((strchr(validkey, key[i]) == NULL) && (strchr(alphabet, key[i]) != NULL)){
			strncat(validkey, &key[i], 1);
			unique_characters++;
		}

		if (unique_characters == KEYWORDSIZE){
			break;
		}
	}

	// Key must be KEYWORDSIZE characters
	if (unique_characters < KEYWORDSIZE) {
		return NULL;
	}

	for(char* c=validkey; (*c=toupper(*c)); ++c) ;

	//printf ("validkey = %s - %d chars\n", validkey, unique_characters);

	return validkey;
}

char *decrypt_vigenere (char *buff, char *key, int bufferlength) {

	char *decrypted;

	decrypted =(char *)malloc(sizeof(char) * bufferlength);

	// Do a bitwise vigenere decryption with key
	// key = character array of size KEYWORDSIZE

	int i = 0;

	while (i < bufferlength) {

		for (int j = 0; j < KEYWORDSIZE; j++) {

			//printf ("decrypt %c with char = %c \n", buff[i], key[j]);

			if (i == bufferlength) {
				break;
			}

			decrypted[i] = buff[i] ^ key[j];

			i++;
		}

	}

	return decrypted;

}

int * decrypt_map (char *key) {

	// Given the key, return an array of integers with the
	// relative position of each letter for the transposition decryption
	// example: key = BADFECIHJG ; order = 1 0 3 5 4 2 8 7 9 6

	int * column_map;
	column_map = (int *)malloc(sizeof(int) * KEYWORDSIZE);

	int map = 0;

	for (char alpha = 'A'; alpha <= 'Z'; alpha++) {

		for (int key_i = 0; key_i < KEYWORDSIZE; key_i++) {

			if (key[key_i] == alpha) {
				//printf("found %c at index %i\n", alpha, key_i);
				//printf(" %i ", key_i);
				column_map[key_i] = map;
				map++;
				break;

			}

		}

	}
	//printf ("\n");

	return (column_map);

}

int *encrypt_map (char *key) {

	// Given the key, return an array of integers with the
	// position of each letter for the transposition encryption
	// example: key = BADFECIHJG ; order = 1 0 5 2 4 3 9 7 6 8

	// this does not give the order of the characters in the keyword
	// but rather the order of the columns to be read in the transpo

	int *key_map;
	key_map = (int *)malloc(sizeof(int) * KEYWORDSIZE);

	int map = 0;

	//printf("key is %s\n", key);
	//printf ("trans = ");

	for (char alpha = 'A'; alpha <= 'Z'; alpha++) {

		for (int key_i = 0; key_i < KEYWORDSIZE; key_i++) {

			if (key[key_i] == alpha) {
				//printf("found %c at index %i\n", alpha, key_i);
				//printf(" %i ", key_i);
				key_map[map] = key_i;
				map++;
				break;

			}

		}

	}
	//printf ("\n");

	for (int j = 0; j < KEYWORDSIZE; j++) {
		printf ("%i ", key_map[j]);
	}
	printf ("\n");


	return (key_map);

}

char *columnar_transposition_decrypt (char *buff, char *key, int bufferlength) {

	char *transposed;

	int bitbufferlength = bufferlength * 8;

	transposed =(char *)malloc(bufferlength);

	// the input into KEYWORDSIZE columns, then concat the columns
	// based upon the key

	int colsize = bitbufferlength / KEYWORDSIZE;
	int remainder = bitbufferlength % KEYWORDSIZE;

	if (remainder != 0) {
		colsize++;
	}

	printf ("bufflength: %i\n", bufferlength );
	printf ("bits: %i\n", bitbufferlength );
	printf ("keywordsize: %i\n", KEYWORDSIZE);
	printf ("colsize: %i\n", colsize);
	printf ("remainder: %i\n", remainder);

	int *columns[KEYWORDSIZE];

	for (int k = 0; k < KEYWORDSIZE; k++) {
		columns[k] = (int *)malloc(sizeof(int) * colsize);
	}

	// get the key mapping for decryption
	int *column_map = decrypt_map(key);
	int *encryption_map = encrypt_map(key);

	// print the key map for debug
	for (int i = 0; i < KEYWORDSIZE; i++) {
		printf("%i ", column_map[i]);
	}
	printf ("\n");

	int i = 0;
	int bitlocation = 0;
	int column_index = 0;
	int column = 0;

	// This time we need to read buffer one bit at a time
	// buff[i] is stored as bytes (8 bits)

	// Split buff[i] into bits, and then populate each of the columns

	while (i < bufferlength) {

		for (int bitpos = 7; bitpos >= 0; bitpos--) {

			column_index = bitlocation % colsize;

			int map = encryption_map[column];

			if ((remainder != 0) && (column_index == (colsize - 1)) && (map >= remainder)) {

				//printf ("skip column_index = %i and column = %i\n", column_index, column);
				column++;
				bitlocation++;
				bitpos++;   // redo this position

			} else {

				int bit = ((buff[i] >> bitpos) & 1);

				//printf ("bitpos %i - %i\n", bitpos, bit);
				//printf ("put %i position col = %i, index = %i \n", bit, column, column_index);
				//printf ("%i ", bit);

				columns[column][column_index] = bit;

				if (column_index == (colsize - 1)) {
					column++;
					//printf("\n");
				}

				bitlocation++;
			}
		}

		i++;

	}

	// now read off the bits according to the key order


	// now we just have to read each bit in the columns, according to the
	// column map, and set each of the bits in an 8 bit char

	char ch = 0;
	int location = 0;
	int bitposition = 0;
	int chlocation = 0;


	for (int j = 0; j < colsize; j++) {

		for (int i = 0; i< KEYWORDSIZE; i++) {

			int map = column_map[i];
			int emap = encryption_map[map];
			//printf ("reading from column %i, row %i - emap = %i - \n", map, j, emap);

			if ((remainder== 0) ||(j != (colsize - 1)) || (emap < remainder)) {

				bitposition = location % 8;

				printf ("%i ", columns[map][j]);
				//printf ("- reading from column %i, row %i - emap = %i\n", map, j, emap);

				ch |= columns[map][j];

				if (bitposition == 7) {
					//printf (" - ");
					//printf ("appending %c to transposed: in position %i %s\n", ch, chlocation, transposed);
					printf ("\n");
					transposed[chlocation] = ch;
					chlocation++;
					transposed[chlocation] = '\0';
					ch = 0;
				} else {
					ch <<= 1;
				}
				location++;
			}
		}

	}
	printf("\n");

	return transposed;

}










int main (int argc, char **argv) {

	char *ciphertext_file;
	char *key1;
	char *key2;
	int c;
	int verbose = 0;

	printf ("Program: decrypt\n");

	if (argc == 1) {
			printf("Error: Provide binary filename and keys.\n");
			print_try_help();
			exit(0);
		}

		while ((c = getopt (argc, argv, "hv")) != -1) {
			// Option argument
			switch (c) {
			case 'h':
				print_help();
				exit(1);
			case 'v':
				verbose = 1;
				break;
			default:
				break;
			}
		}

		// ciphertext file should be non option argument
		ciphertext_file = argv[optind];

		if (!ciphertext_file) {
			printf("Error: Provide the name of the file to decrypt.\n");
			print_try_help();
			exit(0);
		}

		optind++;

		// key1 should be non option argument
		key1 = argv[optind];

		if (!key1) {
			printf("Error: Provide key1.\n");
			print_try_help();
			exit(0);
		}
		optind++;

		// key2 file should be non option argument
		key2 = argv[optind];

		if (!key2) {
			printf("Error: Provide key2.\n");
			print_try_help();
			exit(0);
		}

		printf ("file: %s\n", ciphertext_file);
		//printf ("key1: %s\n", key1);
		//printf ("key2: %s\n", key2);

		// test keys for appropriate length
		char *k1 = validate_key(key1);
		char *k2 = validate_key(key2);

		if (k1 == NULL) {
			printf("Key 1 is too short, must be %i unique characters (A-Z).\n", KEYWORDSIZE);
			print_try_help();
			exit(0);
		}

		if (k2 == NULL) {
			printf("Key 2 is too short, must be %i unique characters (A-Z).\n", KEYWORDSIZE);
			print_try_help();
			exit(0);
		}

		printf ("k1 = %s\n", k1);
		printf ("k2 = %s\n", k2);

		// Read from the binary file into a buffer
		char *buffer;
		unsigned long fileLen;

		//Open the file
		FILE *fp = fopen(ciphertext_file,"rb");
		if(!fp) {
			printf ("Unable to read file");
			return 0;
		}

		//Get file length
		fseek(fp, 0, SEEK_END);
		fileLen=ftell(fp);
		fseek(fp, 0, SEEK_SET);

		//Allocate memory
		buffer=(char *)malloc(fileLen);
		if (!buffer) {
			printf("Memory error!");
	        fclose(fp);
			return 0;
		}

		//Read file entire contents into buffer
		fread(buffer, fileLen, 1, fp);
		fclose(fp);

		char *first_transpo = columnar_transposition_decrypt(buffer, k2, fileLen);

		free(buffer);

		char *second_transpo = columnar_transposition_decrypt(first_transpo, k1, fileLen);

		free(first_transpo);

		// decrypt the buffer with key1
		char *last_decryption = decrypt_vigenere(second_transpo, k1, fileLen);
		free(second_transpo);

		printf ("final decryption = %s", last_decryption);

		// write to output file

		char *output_filename = "decrypted";
		FILE *wp;
		wp = fopen(output_filename,"wb");
		fwrite(last_decryption,fileLen,1,wp);
		fclose(wp);

}


