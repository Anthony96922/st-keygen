/*
 * Registration key maker for Stereo Tool
 * Copyright (C) 2021 Anthony96922
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

/*
 * This generates valid keys for a given name. Generated keys will be
 * recognized, but the unregistered message still plays every 12 hours.
 */

#define DEBUG

#include <stdio.h>
#include <string.h>

// the default name to use when none is specified
#define DEFAULT_NAME "Akira Kurosawa"

/*
 * <23a06b710bde7407f92df95901090b25d9f9c119d17991e7697284ad40>
 * <30a00368d7df8aad392df95901090b25d9f9c119d17991e76972848447>
 * <49a02768375e833d592df95901090b25d9f9c119d17991e7697284c44e>
 * <b1a02b799fd78161792df95901090b25d9f9c119d17991e7697284894e>
*/

int make_key(char *name, int length, char *out_key) {
	int i, j;

	// hex representation of one byte (2 chars + terminator)
	char key_char_byte[3];

	// needs better names
	char encoded_key_char;
	char xored_key_char;

	// make sure we don't try to divide by 0
	int sanity_check;

	// key prefix chars 1-8 (skip 0)
	unsigned int key_prefix_1_4, key_prefix_5_8;

	// 15 = the stuff before and after the key (112233445566778899<name>aabbccddeeff)
	int key_length = length + 15;

	char key[100] = {0};

	// the locations of the important parts
	char *key_license	= key + 1; // licensed features
	char *key_checksum	= key + 5; // not sure how this is calculated
	char *key_body		= key + 9; // encoded name
	char *key_trailer	= key + 9 + length; // extra stuff

	// the key prefix: one 1-byte int and two 4-byte ints (9 chars)
	key[0] = 0x70;
	key_prefix_1_4 = (0x03643050 & 0xfffdffff) | 0x109af;

	memcpy(key_license, &key_prefix_1_4, 4);

	// copy name to key body
	memcpy(key_body, name, length);

	sanity_check = (key_body[2] - key_body[3]) + 1;

	if (sanity_check == 0) {
		// we can't divide by 0
		fprintf(stderr, "Invalid name.\n");
		return 1;
	}

	sanity_check = (key_body[0] * key_body[1]) / sanity_check;

	key_trailer[1] = (((key_body[0] | key_body[1]) ^ ((key_body[2] | key_body[3]) + key_body[4])) & 0xf) << 4;
	key_trailer[1] |= (key_body[0] ^ key_body[1] ^ key_body[2] ^ key_body[3] ^ key_body[4]) & 0xf;
	key_trailer[2] = ((sanity_check - key_body[4]) & 0xf) << 4;
	key_trailer[2] |= (sanity_check * key_body[4]) & 0xf;
	key_trailer[3] = ((key_body[2] - key_body[3]) * (key_body[0] + key_body[1]) ^ key_body[4]) & 0xf;
	key_trailer[3] |= (((key_body[2] + key_body[3]) * (key_body[0] - key_body[1]) ^ ~key_body[4]) & 0xf) << 4;

	int uVar8 = 255;
	int bVar3 = uVar8;
	if ((((((key_body[0] + key_body[1] + key_body[2]) - (key_body[3] + key_body[4])) & 0xf) ^ uVar8) & 8) != 0) {
		bVar3 = bVar3 ^ 8;
	}

	key_trailer[4] = bVar3;
	key_trailer[4] = 114;

	key_trailer[5] = ((key_body[0] + key_body[1] - key_body[2]) - (key_body[3] + key_body[4])) & 0xf;
	key_trailer[5] |= 9 << 4;

	key_prefix_5_8 = 0;

	// needs fixing
	for (i = 0; i < key_length; i++) {
		key_prefix_5_8 = key[i] * 0x11121 + (key_prefix_5_8 << 3);
		key_prefix_5_8 += (key_prefix_5_8 >> 26);
	}

	memcpy(key_checksum, &key_prefix_5_8, 4);

	// encode the key
	for (i = 0; i < key_length; i++) {
		xored_key_char = key[i] ^ ((-1 - i) - (1 << (1 << (i & 0x1f) & 7)));
		encoded_key_char = 0;
		for (j = 0; j < 8; j++) {
			encoded_key_char = (encoded_key_char << 1) | (xored_key_char & 1);
			xored_key_char >>= 1;
		}
		key[i] = encoded_key_char;
	}

	out_key[0] = '<';

	for (i = 0; i < key_length; i++) {
#ifdef DEBUG
		if (i == 1) {
			strcat(out_key, "\x1b[1;35m");
		}
		if (i == 5) {
			strcat(out_key, "\x1b[1;36m");
		}
#endif
		sprintf(key_char_byte, "%.2x", key[i]);
#ifdef DEBUG
		if (i == 9) {
			strcat(out_key, "\x1b[0;0m");
		}
#endif
		strcat(out_key, key_char_byte);
	}

#ifdef DEBUG
	out_key[key_length*2+1+20] = '>';
#else
	out_key[key_length*2+1] = '>';
#endif

	return 0;
}

int main(int argc, char *argv[]) {
	int ret;

	char user_name[50] = {0};
	int user_name_length;

	// registration key
	char reg_key[200] = {0};

	if (argc < 2) {
		strcpy(user_name, DEFAULT_NAME);
		printf("Using default name \"%s\".\n", DEFAULT_NAME);
	} else {
		strncpy(user_name, argv[1], 50);
		printf("Using name \"%s\".\n", user_name);
	}

	user_name_length = strlen(user_name);

	if (user_name_length < 5) {
		fprintf(stderr, "Name must be at least 5 chars long.\n");
		return 1;
	}

	if (user_name_length > 32) {
		fprintf(stderr, "Name is too long.\n");
		return 1;
	}

	ret = make_key(user_name, user_name_length, reg_key);

	if (ret == 0) {
		if (strcmp(user_name, DEFAULT_NAME) == 0)
			printf("<b1a02b799fd78161792df95901090b25d9f9c119d17991e7697284894e>\n");
		printf("%s\n", reg_key);
	}

	return ret;
}
