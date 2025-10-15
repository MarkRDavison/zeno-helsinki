#include <cstdlib>
#include <iostream>

#include <helsinki/Example/ExampleConfiguration.hpp>

int main(int, char**) {

	auto example = ex::ExampleConfiguration();

	std::cout << "Hello World!" << std::endl;
	return EXIT_SUCCESS;
}