#include "foo.h"

#include <iostream>

int main(const char** argc, int argv)
{
	std::cout << "Magic value: " << foo() << std::endl;
	return 0;
}