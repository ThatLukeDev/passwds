#include <cstdlib>
#include <cstring>
#include <chrono>

void populateRand(char* chars, int len) {
	chars[len - 1] = 0;

	int seed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	srand(seed);

	// ascii 33 to 126
	for (int i = 0; i < len - 1; i++) {
		chars[i] = rand() % 94 + 33;
	}
}
