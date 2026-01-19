//
// Created by viceda-s on 16/01/26.
//

#include "PmergeMe.hpp"
#include <iostream>

int main(int ac, char **av) {
	PmergeMe pm;

	if (!pm.parseInput(ac, av)) {
		std::cerr << "Error" << std::endl;
		return 1;
	}

	pm.sort();
	pm.displayResults();

	return 0;
}