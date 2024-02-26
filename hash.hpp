#ifndef tld_hash
#define tld_hash

#include <cmath>
#include <cstring>

namespace sha256 {
	const unsigned int k[64] = {
		0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
		0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
		0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
		0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
		0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
		0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
		0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
		0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
	};

	unsigned int rotate(unsigned int a, unsigned int b) {
		return (a << (32 - b))|(a >> b);
	}

	char* hash(void* data, unsigned int size) {
		unsigned int h0 = 0x6a09e667;
		unsigned int h1 = 0xbb67ae85;
		unsigned int h2 = 0x3c6ef372;
		unsigned int h3 = 0xa54ff53a;
		unsigned int h4 = 0x510e527f;
		unsigned int h5 = 0x9b05688c;
		unsigned int h6 = 0x1f83d9ab;
		unsigned int h7 = 0x5be0cd19;

		unsigned int chunks = (unsigned int)std::ceil((size + 9) / 64.0);
		unsigned int arrSize = chunks * 64;

		char msg[arrSize];
		for (unsigned int i = 0; i < arrSize; i++)
			msg[i] = 0x0;
		std::memcpy(msg, data, size);
		msg[size] = 0b10000000;
		msg[arrSize - 1] = size * 8;

		for (unsigned int cNum = 0; cNum < chunks; cNum++) {
			char* chunk = msg + cNum * 64;

			unsigned int w[64];
			std::memcpy(w, chunk, 64);

			for (unsigned int i = 16; i < 64; i++) {
				unsigned int s0 = rotate(w[i-15], 7) ^ rotate(w[i-15], 18) ^ rotate(w[i-15], 3);
				unsigned int s1 = rotate(w[i-2], 17) ^ rotate(w[i-2], 19) ^ rotate(w[i-2], 10);
				w[i] = w[i-16] + s0 + w[i-7] + s1;
			}

			unsigned int a = h0;
			unsigned int b = h1;
			unsigned int c = h2;
			unsigned int d = h3;
			unsigned int e = h4;
			unsigned int f = h5;
			unsigned int g = h6;
			unsigned int h = h7;

			for (unsigned int i = 0; i < 64; i++) {
				unsigned int S1 = rotate(e, 6) ^ rotate(e, 11) ^ rotate(e, 25);
				unsigned int ch = (e & f) ^ ((~e) & g);
				unsigned int temp1 = h + S1 + ch + k[i] + w[i];
				unsigned int S0 = rotate(a, 2) ^ rotate(a, 13) ^ rotate(a, 22);
				unsigned int maj = (a & b) ^ (a & c) ^ (b & c);
				unsigned int temp2 = S0 + maj;

				h = g;
				g = f;
				f = e;
				e = d + temp1;
				d = c;
				c = b;
				b = a;
				a = temp1 + temp2;
			}

			h0 = h0 + a;
			h1 = h1 + b;
			h2 = h2 + c;
			h3 = h3 + d;
			h4 = h4 + e;
			h5 = h5 + f;
			h6 = h6 + g;
			h7 = h7 + h;
		}

		char* out = (char*)malloc(256);
		memcpy(out + 0x00, &h0, 4);
		memcpy(out + 0x04, &h1, 4);
		memcpy(out + 0x08, &h2, 4);
		memcpy(out + 0x0C, &h3, 4);
		memcpy(out + 0x10, &h4, 4);
		memcpy(out + 0x14, &h5, 4);
		memcpy(out + 0x18, &h6, 4);
		memcpy(out + 0x1C, &h7, 4);

		return (char*)out;
	}
	char* hash(char* msg) {
		return sha256::hash((void*)msg, strlen(msg));
	}
	char* hash(const char* msg) {
		return sha256::hash((void*)msg, strlen(msg));
	}
}

#endif
