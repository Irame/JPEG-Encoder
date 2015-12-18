#include "stdafx.h"
#include "SamplingScheme.h"
#include <algorithm>
#include "ColorNames.h"

const SamplingScheme SamplingScheme::Scheme444(
	ChannelReductionOptions{ 1, Subsampling, 1, Subsampling },		// y
	ChannelReductionOptions{ 1, Subsampling, 1, Subsampling },		// cb
	ChannelReductionOptions{ 1, Subsampling, 1, Subsampling });		// br

const SamplingScheme SamplingScheme::Scheme422(
	ChannelReductionOptions{ 1, Subsampling, 1, Subsampling },		// y
	ChannelReductionOptions{ 2, Subsampling, 1, Subsampling },		// cb
	ChannelReductionOptions{ 2, Subsampling, 1, Subsampling });		// br

const SamplingScheme SamplingScheme::Scheme411(
	ChannelReductionOptions{ 1, Subsampling, 1, Subsampling },		// y
	ChannelReductionOptions{ 4, Subsampling, 1, Subsampling },		// cb
	ChannelReductionOptions{ 4, Subsampling, 1, Subsampling });		// br

const SamplingScheme SamplingScheme::Scheme420(
	ChannelReductionOptions{ 1, Subsampling, 1, Subsampling },		// y
	ChannelReductionOptions{ 2, Average, 2, Average },				// cb
	ChannelReductionOptions{ 2, Average, 2, Average });				// br


// For Testing
const SamplingScheme SamplingScheme::Scheme422Average(
	ChannelReductionOptions{ 1, Subsampling, 1, Subsampling },		// y
	ChannelReductionOptions{ 2, Average, 1, Subsampling },		// cb
	ChannelReductionOptions{ 2, Average, 1, Subsampling });		// br

const SamplingScheme SamplingScheme::Scheme422Height(
	ChannelReductionOptions{ 1, Subsampling, 1, Subsampling },		// y
	ChannelReductionOptions{ 1, Subsampling, 2, Subsampling },		// cb
	ChannelReductionOptions{ 1, Subsampling, 2, Subsampling });		// br

const SamplingScheme SamplingScheme::Scheme422HeightAverage(
	ChannelReductionOptions{ 1, Subsampling, 1, Subsampling },		// y
	ChannelReductionOptions{ 1, Subsampling, 2, Average },		// cb
	ChannelReductionOptions{ 1, Subsampling, 2, Average });		// br

const SamplingScheme SamplingScheme::Scheme311(
	ChannelReductionOptions{ 1, Subsampling, 1, Subsampling },		// y
	ChannelReductionOptions{ 3, Subsampling, 1, Subsampling },		// cb
	ChannelReductionOptions{ 3, Subsampling, 1, Subsampling });		// br

static int gcd(int a, int b)
{
	if (a == 0) return b;
	while (b != 0)
	{
		if (a > b)
			a = a - b;
		else
			b = b - a;
	}
	return a;
}

static int lcm(int a, int b)
{
	return (a * b) / gcd(a, b);
}

SamplingScheme::SamplingScheme(ChannelReductionOptions yReductionOptions, ChannelReductionOptions cbReductionOptions, ChannelReductionOptions crReductionOptions)
	: reductionOptions{ yReductionOptions, cbReductionOptions, crReductionOptions },
	stepSize(
		lcm(lcm(yReductionOptions.widthFactor, cbReductionOptions.widthFactor), crReductionOptions.widthFactor) * 8,
		lcm(lcm(yReductionOptions.heightFactor, cbReductionOptions.heightFactor), crReductionOptions.heightFactor) * 8),
	inverseFactor{
		Dimension2D(stepSize.width / yReductionOptions.widthFactor / 8, stepSize.height / yReductionOptions.heightFactor / 8),
		Dimension2D(stepSize.width / cbReductionOptions.widthFactor / 8, stepSize.height / cbReductionOptions.heightFactor / 8),
		Dimension2D(stepSize.width / crReductionOptions.widthFactor / 8, stepSize.height / crReductionOptions.heightFactor / 8),
	}
{}