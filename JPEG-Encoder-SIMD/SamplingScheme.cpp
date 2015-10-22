#include "stdafx.h"
#include "SamplingScheme.h"

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
	ChannelReductionOptions{ 2, Average, 2, Average },		// cb
	ChannelReductionOptions{ 2, Average, 2, Average });		// br

SamplingScheme::SamplingScheme(ChannelReductionOptions yReductionOptions, ChannelReductionOptions cbReductionOptions, ChannelReductionOptions crReductionOptions)
	: yReductionOptions(yReductionOptions),cbReductionOptions(cbReductionOptions),crReductionOptions(crReductionOptions)

{}