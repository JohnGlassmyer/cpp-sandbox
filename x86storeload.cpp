#include <atomic>
#include <iostream>
#include <thread>

// based on the oft-referenced example from Intel's x86 guide.

// switch this to demonstrate the effect
constexpr bool shouldFence = false;

int x;
int y;
// not registers but w/e
int r1;
int r2;

std::atomic<bool> shouldProceed;

void storeAndLoad(int &storeTo, int &loadFrom, int &dest) {
	while(!shouldProceed);

	storeTo = 1;

	// prevent compile-time reordering by the compiler
	asm ("" ::: "memory");

	if (shouldFence) {
		// also prevent run-time reordering by the processor
		asm ("mfence" ::: "memory");
	}

	dest = loadFrom;
}

int main() {
	for (int i = 0; i < 10000; i++) {
		x = 0;
		y = 0;
		r1 = 0;
		r2 = 0;

		shouldProceed = false;
		std::thread left(storeAndLoad, std::ref(x), std::ref(y), std::ref(r1));
		std::thread right(storeAndLoad, std::ref(y), std::ref(x), std::ref(r2));
		shouldProceed = true;

		left.join();
		right.join();

		std::cout << r1 << r2;
		if (r1 == 0 && r2 == 0) {
			// store-load reordered
			std::cout << "*****";
		}
		std::cout << std::endl;
	}
}
