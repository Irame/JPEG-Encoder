#pragma once
#include "Dimension2D.h"

enum ReductionMethod
{
	Subsampling, Average
};

struct ChannelReductionOptions
{
	const int widthFactor;
	const ReductionMethod widthMethod;
	const int heightFactor;
	const ReductionMethod heightMethod;

	constexpr
	ChannelReductionOptions(const int w, const ReductionMethod wm, const int h, const ReductionMethod hm)
		: widthFactor(w), widthMethod(wm), heightFactor(h), heightMethod(hm)
	{}
};

constexpr int gcd(int a, int b) { return b == 0 ? a : gcd(b, a % b); }
constexpr int lcm(int a, int b) { return (a * b) / gcd(a, b); }

class SamplingDefinition
{
public:
	const ChannelReductionOptions reductionOptions[3];
	const Dimension2D stepSize;
	const Dimension2D inverseFactor[3];

	constexpr
		SamplingDefinition(ChannelReductionOptions yReductionOptions, ChannelReductionOptions cbReductionOptions, ChannelReductionOptions crReductionOptions)
		: 
			reductionOptions{ yReductionOptions, cbReductionOptions, crReductionOptions },
			stepSize(
				lcm(lcm(yReductionOptions.widthFactor, cbReductionOptions.widthFactor), crReductionOptions.widthFactor) * 8,
				lcm(lcm(yReductionOptions.heightFactor, cbReductionOptions.heightFactor), crReductionOptions.heightFactor) * 8),
			inverseFactor{
				Dimension2D(stepSize.width / yReductionOptions.widthFactor / 8, stepSize.height / yReductionOptions.heightFactor / 8),
				Dimension2D(stepSize.width / cbReductionOptions.widthFactor / 8, stepSize.height / cbReductionOptions.heightFactor / 8),
				Dimension2D(stepSize.width / crReductionOptions.widthFactor / 8, stepSize.height / crReductionOptions.heightFactor / 8) }
		{};
};

namespace Sampling
{
	const SamplingDefinition Scheme444(
		{ 1, Subsampling, 1, Subsampling },		// Y
		{ 1, Subsampling, 1, Subsampling },		// Cb
		{ 1, Subsampling, 1, Subsampling });	// Cr

	const SamplingDefinition Scheme422(
		{ 1, Subsampling, 1, Subsampling },		// Y
		{ 2, Subsampling, 1, Subsampling },		// Cb
		{ 2, Subsampling, 1, Subsampling });	// Cr

	const SamplingDefinition Scheme411(
		{ 1, Subsampling, 1, Subsampling },		// Y
		{ 4, Subsampling, 1, Subsampling },		// Cb
		{ 4, Subsampling, 1, Subsampling });	// Cr

	const SamplingDefinition Scheme420(
		{ 1, Subsampling, 1, Subsampling },		// Y
		{ 2, Average, 2, Average },				// Cb
		{ 2, Average, 2, Average });			// Cr


// For Testing
	const SamplingDefinition Scheme422Average(
		{ 1, Subsampling, 1, Subsampling },		// Y
		{ 2, Average, 1, Subsampling },			// Cb
		{ 2, Average, 1, Subsampling });		// Cr

	const SamplingDefinition Scheme422Height(
		{ 1, Subsampling, 1, Subsampling },		// Y
		{ 1, Subsampling, 2, Subsampling },		// Cb
		{ 1, Subsampling, 2, Subsampling });	// Cr

	const SamplingDefinition Scheme422HeightAverage(
		{ 1, Subsampling, 1, Subsampling },		// Y
		{ 1, Subsampling, 2, Average },			// Cb
		{ 1, Subsampling, 2, Average });		// Cr

	const SamplingDefinition Scheme311(
		{ 1, Subsampling, 1, Subsampling },		// Y
		{ 3, Subsampling, 1, Subsampling },		// Cb
		{ 3, Subsampling, 1, Subsampling });	// Cr

	const SamplingDefinition Scheme321(
		{ 1, Subsampling, 1, Subsampling },		// Y
		{ 3, Subsampling, 2, Subsampling },		// Cb
		{ 3, Subsampling, 2, Subsampling });	// Cr
}