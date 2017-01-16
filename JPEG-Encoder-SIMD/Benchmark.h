#pragma once
#include <chrono>
#include <string>

template <typename F>
static void benchmark(std::string name, std::chrono::milliseconds dur, F&& lambda)
{
	auto start = std::chrono::high_resolution_clock::now();

	int count = 0;
	do
	{
		lambda();
		count++;
	}
	while (std::chrono::high_resolution_clock::now() - start < dur);

	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start);
	long long durationAVG = duration.count() / count;

	std::cout << "Function " << name << " took avg. " << durationAVG << "us (" << durationAVG / 1000.0f << "ms) over " << count << " runs." << std::endl;
}


struct StopWatch
{
	typedef std::chrono::high_resolution_clock clock;
	typedef clock::time_point time_point;

	time_point startTime; // Point in time when start() was called
	time_point lapTime;   // Point in time when operator() was called

public:
	StopWatch()
	{
		reset();
	}

	void reset()
	{
		startTime = clock::now();
		lapTime = startTime;
	}

	void stop()
	{
		auto duration = std::chrono::duration_cast<std::chrono::microseconds>(clock::now() - startTime);

		std::cout << "Total time: " << duration.count() << "us (" << duration.count() / 1000.0f << "ms)" << std::endl;
	}

	void operator()(const std::string& str = "")
	{
		auto duration = std::chrono::duration_cast<std::chrono::microseconds>(clock::now() - lapTime);

		std::cout << str << (str.empty() ? "" : " ") << "took " << duration.count() << "us (" << duration.count() / 1000.0f << "ms)" << std::endl;

		lapTime = clock::now();
	}
};