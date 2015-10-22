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

	const ChannelReductionOptions yReductionOptions, cbReductionOptions, crReductionOptions;
};

