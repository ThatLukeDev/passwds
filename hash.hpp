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

	template <typename T>
	T swap(T a) {
		T _out = a;
		for (unsigned int i = 0; i < sizeof(T); i++) {
			*((char*)(&_out) + i) = *((char*)(&a) + sizeof(T) - 1 - i);
		}
		return _out;
	}

	bool wrongEndian() {
		unsigned int a = 1;
		return *(char*)&a == '\0' ? false : true;
	}

	bool cacheSwp = false;
	bool doSwp = false;

	template <typename T>
	T swp(T a) {
		T _out = a;
		if (cacheSwp) {
			if (doSwp) {
				_out = swap(_out);
			}
		}
		else {
			doSwp = wrongEndian();
			cacheSwp = true;
			if (doSwp) {
				_out = swap(_out);
			}
		}
		return _out;
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

		char* msg = (char*)malloc(arrSize);
		for (unsigned int i = 0; i < arrSize; i++)
			msg[i] = 0x0;
		std::memcpy(msg, data, size);
		msg[size] = 0b10000000;
		*(unsigned long long*)(msg + arrSize - 8) = (unsigned long long)size * 8;
		*(unsigned long long*)(msg + arrSize - 8) = swp(*(unsigned long long*)(msg + arrSize - 8));

		for (unsigned int cNum = 0; cNum < chunks; cNum++) {
			char* chunk = msg + cNum * 64;

			//unsigned int w[64];
			unsigned int* w = (unsigned int*)malloc(256);
			std::memcpy(w, chunk, 64);

			for (unsigned int i = 16; i < 64; i++) {
				unsigned int s0 = rotate(swp(w[i-15]), 7) ^ rotate(swp(w[i-15]), 18) ^ (swp(w[i-15]) >> 3);
				unsigned int s1 = rotate(swp(w[i-2]), 17) ^ rotate(swp(w[i-2]), 19) ^ (swp(w[i-2]) >> 10);
				if (doSwp)
					w[i] = swp(swp(w[i-16]) + swp(w[i-7]) + s0 + s1);
				else
					w[i] = w[i-16] + s0 + w[i-7] + s1;
			}

			unsigned int a = swp(h0);
			unsigned int b = swp(h1);
			unsigned int c = swp(h2);
			unsigned int d = swp(h3);
			unsigned int e = swp(h4);
			unsigned int f = swp(h5);
			unsigned int g = swp(h6);
			unsigned int h = swp(h7);

			for (unsigned int i = 0; i < 64; i++) {
				unsigned int S1 = rotate(swp(e), 6) ^ rotate(swp(e), 11) ^ rotate(swp(e), 25);
				unsigned int ch = (swp(e) & swp(f)) ^ (swp(~e) & swp(g));
				unsigned int temp1 = swp(h) + S1 + ch + k[i] + swp(w[i]);
				unsigned int S0 = rotate(swp(a), 2) ^ rotate(swp(a), 13) ^ rotate(swp(a), 22);
				unsigned int maj = swp((a & b) ^ (a & c) ^ (b & c));
				unsigned int temp2 = S0 + maj;

				h = g;
				g = f;
				f = e;
				e = swp(swp(d) + temp1);
				d = c;
				c = b;
				b = a;
				a = swp(temp1 + temp2);
			}

			h0 = swp(h0 + swp(a));
			h1 = swp(h1 + swp(b));
			h2 = swp(h2 + swp(c));
			h3 = swp(h3 + swp(d));
			h4 = swp(h4 + swp(e));
			h5 = swp(h5 + swp(f));
			h6 = swp(h6 + swp(g));
			h7 = swp(h7 + swp(h));
		}

		char* out = (char*)malloc(32);
		memcpy(out + 0x00, &h0, 4);
		memcpy(out + 0x04, &h1, 4);
		memcpy(out + 0x08, &h2, 4);
		memcpy(out + 0x0C, &h3, 4);
		memcpy(out + 0x10, &h4, 4);
		memcpy(out + 0x14, &h5, 4);
		memcpy(out + 0x18, &h6, 4);
		memcpy(out + 0x1C, &h7, 4);

		free(msg);

		return (char*)out;
	}
	char* hash(char* msg) {
		return sha256::hash((void*)msg, strlen(msg));
	}
	char* hash(const char* msg) {
		return sha256::hash((void*)msg, strlen(msg));
	}

	char* hashX1000(void* data, unsigned int size) {
		char* buffer = (char*)malloc(32);
		buffer = sha256::hash(data, size);
		for (int i = 0; i < 999; i++) {
			buffer = sha256::hash((void*)buffer, 32);
		}
		return buffer;
	}
	char* hashX1000(char* msg) {
		return sha256::hashX1000((void*)msg, strlen(msg));
	}
	char* hashX1000(const char* msg) {
		return sha256::hashX1000((void*)msg, strlen(msg));
	}
}

#endif
