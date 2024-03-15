#ifndef tld_aes
#define tld_aes

namespace aes256 {
	unsigned int W[60];

	unsigned char sbox[256];
	unsigned char rsbox[256];

	template <typename T>
	T rotate(T a, unsigned int b) {
		return (a << (sizeof(T)*8 - b))|(a >> b);
	}

	template <typename T>
	T unrotate(T a, unsigned int b) {
		return (a >> (sizeof(T)*8 - b))|(a << b);
	}

	unsigned char rmult(unsigned char a, unsigned char b) {
		unsigned char p = 0; // ab+p=product
		while (a != 0 && b != 0) { // if a or b is zero, then 0*(a or b)+p=product so p(+0)=product (we can stop)
			if (b & 0b00000001) { // if polynomial for b has constant term
				p ^= a; // addition in rijndaels finite field
			}

			if (a & 0b10000000) { // if nonzero term x^7, then must be reduced before x^8
				a <<= 1; // *x
				a ^= 0x11b; // subtract irreducible primitive polynomial x^8 + x^4 + x^3 + x + 1
			}
			else {
				a <<= 1; // *x
			}

			b >>= 1; // /x
		}
		return p;
	}

	void initSbox() {
		unsigned char p = 1, q = 1;

		do {
			p = p ^ (p << 1) ^ (p & 0x80 ? 0x1B : 0);
			q = q ^ (q << 1);
			q = q ^ (q << 2);
			q = q ^ (q << 4);
			q = q ^ (q & 0x80 ? 0x09 : 0);
			sbox[p] = (q ^ rotate(q, 1) ^ rotate(q, 2) ^ rotate(q, 3) ^ rotate(q, 4)) ^ 0x63;
		} while (p != 1);

		sbox[0] = 0x63;

		for (int i = 0; i < 256; i++) {
			rsbox[sbox[i]] = i;
		}
	}

	unsigned char rc[10] = {
		0x01, 0x02, 0x04, 0x08, 0x10,
		0x20, 0x40, 0x80, 0x1B, 0x36
	}; // rc[i]=rc[i-1]*2 up to 128 then rc[i]=(rc[i-1]*2)) ^ 0x11B (rijndaels galois finite field)

	unsigned int rcon(unsigned int i) {
		unsigned int _out = 0;
		*(unsigned char*)&_out = rc[i];
		return _out;
	}

	unsigned int rotWord(unsigned int a) {
		return rotate(a, 8);
	}

	unsigned int subWord(unsigned int a) {
		unsigned int _out = a;
		for (int i = 0; i < 4; i++) {
			((unsigned char*)&_out)[i] = sbox[((unsigned char*)&_out)[i]];
		}
		return _out;
	}

	void initKeySchedule(void* _key) {
		int key[8];
		memcpy(key, _key, 32);

		for (int i = 0; i < 60; i++) {
			if (i < 8) {
				W[i] = key[i];
			}
			else if (i % 8 == 0) {
				W[i] = W[i-8] ^ subWord(rotWord(W[i-1])) ^ rcon(i/8);
			}
			else if (i % 8 == 4) {
				W[i] = W[i-8] ^ subWord(W[i-1]);
			}
			else {
				W[i] = W[i-8] ^ W[i-1];
			}
		}
	}

	char* encrypt(void* _text, unsigned int length, void* _key) {
		unsigned int blocksLen = std::ceil(length / 16.0);
		unsigned char blocks[blocksLen][16];
		for (int i = 0; i < 16; i++) blocks[blocksLen-1][i] = 0;
		memcpy(blocks, _text, length);

		initSbox();

		initKeySchedule(_key);

		for (unsigned int block = 0; block < blocksLen; block++) {
			// initial round key addition
			for (int i = 0; i < 4; i++) {
				((unsigned int*)blocks[block])[i] ^= W[i];
			}
			for (int round = 1; round <= 13; round++) {
				// sub bytes from sbox
				for (int i = 0; i < 16; i++) {
					blocks[block][i] = sbox[blocks[block][i]];
				}

				// shift rows
				for (int i = 0; i < 4; i++) {
					((unsigned int*)blocks[block])[i] = rotate(((unsigned int*)blocks[block])[i], i);
				}

				// mix columns
				for (int i = 0; i < 4; i++) {
					unsigned char a[4];
					memcpy(a, &blocks[block][i*4], 4);
					blocks[block][i*4+0] = rmult(a[0], 2) ^ rmult(a[1], 3) ^ a[2] ^ a[3];
					blocks[block][i*4+1] = a[0] ^ rmult(a[1], 2) ^ rmult(a[2], 3) ^ a[3];
					blocks[block][i*4+2] = a[0] ^ a[1] ^ rmult(a[2], 2) ^ rmult(a[3], 3);
					blocks[block][i*4+3] = rmult(a[0], 3) ^ a[1] ^ a[2] ^ rmult(a[3], 2);
				}

				// add round key
				for (int i = 0; i < 4; i++) {
					((unsigned int*)blocks[block])[i] ^= W[round*4+i];
				}
			}

			// final sub bytes from sbox
			for (int i = 0; i < 16; i++) {
				blocks[block][i] = sbox[blocks[block][i]];
			}

			// final shift rows
			for (int i = 0; i < 4; i++) {
				((unsigned int*)blocks[block])[i] = rotate(((unsigned int*)blocks[block])[i], i);
			}

			// final add round key
			for (int i = 0; i < 4; i++) {
				((unsigned int*)blocks[block])[i] ^= W[14*4+i];
			}
		}

		char* _out = (char*)malloc(blocksLen * 16);
		memcpy(_out, blocks, blocksLen * 16);
		return _out;
	}

	char* decrypt(void* _text, unsigned int length, void* _key) {
		unsigned int blocksLen = std::ceil(length / 16.0);
		unsigned char blocks[blocksLen][16];
		for (int i = 0; i < 16; i++) blocks[blocksLen-1][i] = 0;
		memcpy(blocks, _text, length);

		initSbox();

		initKeySchedule(_key);

		for (unsigned int block = 0; block < blocksLen; block++) {
			// initial remove round key
			for (int i = 0; i < 4; i++) {
				((unsigned int*)blocks[block])[i] ^= W[14*4+i];
			}

			// initial unshift rows
			for (int i = 0; i < 4; i++) {
				((unsigned int*)blocks[block])[i] = unrotate(((unsigned int*)blocks[block])[i], i);
			}

			// initial sub bytes from reverse sbox
			for (int i = 0; i < 16; i++) {
				blocks[block][i] = rsbox[blocks[block][i]];
			}

			for (int round = 13; round >= 1; round--) {
				// remove round key
				for (int i = 0; i < 4; i++) {
					((unsigned int*)blocks[block])[i] ^= W[round*4+i];
				}

				// unmix columns
				for (int i = 0; i < 4; i++) {
					unsigned char a[4];
					memcpy(a, &blocks[block][i*4], 4);
					blocks[block][i*4+0] = rmult(a[0], 14) ^ rmult(a[1], 11) ^ rmult(a[2], 13) ^ rmult(a[3], 9);
					blocks[block][i*4+1] = rmult(a[0], 9) ^ rmult(a[1], 14) ^ rmult(a[2], 11) ^ rmult(a[3], 13);
					blocks[block][i*4+2] = rmult(a[0], 13) ^ rmult(a[1], 9) ^ rmult(a[2], 14) ^ rmult(a[3], 11);
					blocks[block][i*4+3] = rmult(a[0], 11) ^ rmult(a[1], 13) ^ rmult(a[2], 9) ^ rmult(a[3], 14);
				}

				// unshift rows
				for (int i = 0; i < 4; i++) {
					((unsigned int*)blocks[block])[i] = unrotate(((unsigned int*)blocks[block])[i], i);
				}

				// sub bytes from reverse sbox
				for (int i = 0; i < 16; i++) {
					blocks[block][i] = rsbox[blocks[block][i]];
				}
			}

			// final round key removal
			for (int i = 0; i < 4; i++) {
				((unsigned int*)blocks[block])[i] ^= W[i];
			}
		}

		char* _out = (char*)malloc(blocksLen * 16);
		memcpy(_out, blocks, length);
		return _out;
	}
}

#endif
