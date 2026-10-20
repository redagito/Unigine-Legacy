#pragma once

namespace alloc {

// Net number of live heap blocks allocated through the global operators.
long balance();

class Scope {
public:
	Scope() : start(balance()) {}
	long delta() const { return balance() - start; }

private:
	long start;
};

}
