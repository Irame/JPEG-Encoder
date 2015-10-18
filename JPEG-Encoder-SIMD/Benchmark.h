#pragma once

#include <chrono>
#include <functional>
#include <iostream>
#include <string>

using namespace std; 

template <typename F>
static void benchmark(string name, size_t count, F& lambda)
{
	auto start = std::chrono::high_resolution_clock::now();

	for (size_t i = 0; i < count; i++)
	{
		lambda();
	}

	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start);
	long long durationAVG = duration.count() / count;

	std::cout << "Function "<< name << " took avg. " << durationAVG << "us (" << durationAVG / 1000.0f << "ms) over " << count << " runs." << std::endl;
}
