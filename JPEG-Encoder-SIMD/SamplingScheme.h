#pragma once
#include "Dimension2D.h"

enum ReductionMethod
{
	Subsampling, Average
};

struct ChannelReductionOptions
{
	int widthFactor;
	ReductionMethod widthMethod;
	int heightFactor;
	ReductionMethod heightMethod;
};

class SamplingScheme
{
private:
	SamplingScheme(ChannelReductionOptions yReductionOptions, ChannelReductionOptions cbReductionOptions, ChannelReductionOptions crReductionOptions);

public:
	static const SamplingScheme Scheme444, Scheme422, Scheme411, Scheme420;

	// For Testing
	static const SamplingScheme Scheme422Average, Scheme422Height, Scheme422HeightAverage, Scheme311, Scheme321;


	const ChannelReductionOptions reductionOptions[3];
	const Dimension2D stepSize;
	const Dimension2D inverseFactor[3];
};

