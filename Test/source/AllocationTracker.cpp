#include <atomic>
#include <cstdlib>
#include <new>

#include "AllocationTracker.h"

namespace {

std::atomic<long> live_blocks{ 0 };

}

void* operator new(size_t size) {
	if (void* p = std::malloc(size ? size : 1)) {
		live_blocks.fetch_add(1);
		return p;
	}
	throw std::bad_alloc();
}

void* operator new[](size_t size) {
	if (void* p = std::malloc(size ? size : 1)) {
		live_blocks.fetch_add(1);
		return p;
	}
	throw std::bad_alloc();
}

void operator delete(void* p) noexcept {
	if (p) {
		live_blocks.fetch_sub(1);
		std::free(p);
	}
}

void operator delete[](void* p) noexcept {
	if (p) {
		live_blocks.fetch_sub(1);
		std::free(p);
	}
}

namespace alloc {

long balance() {
	return live_blocks.load();
}

}
