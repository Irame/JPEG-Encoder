#pragma once

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
	static const SamplingScheme Scheme422Average, Scheme422Height, Scheme422HeightAverage;


	const ChannelReductionOptions yReductionOptions, cbReductionOptions, crReductionOptions;

	int calcWidthStepSize();
	int calcHeightStepSize();
};

