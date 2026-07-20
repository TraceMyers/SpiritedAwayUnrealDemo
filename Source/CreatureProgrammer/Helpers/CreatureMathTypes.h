#pragma once

// math helper file to fill in the gaps of FMath

enum class EQuadSide : uint8
{
	Top,
	Right,
	Bottom,
	Left
};

enum class EHorizontalSide : uint8
{
	Right=0,
	Left=1
};

struct FAlphaSampler
{
	// Generator takes a normalized float and returns a float
	void GenerateSamples(int SampleCount, float (*Generator)(float))
	{
		check(SampleCount > 1)
		Samples.SetNumUninitialized(SampleCount);
		for (int i = 0; i < SampleCount; i++)
		{
			const float Alpha = (float)i / (SampleCount - 1);
			Samples[i] = Generator(Alpha);
		}
	}
	
	float LinearSample(float Alpha) const;
	
	TArray<float> Samples;
};

struct FQuadraticSolution
{
	float A;
	float B;
};

struct FSolveQuadraticParams
{
	bool bReplaceNan;
	float NanReplacement;
};

// template specialization helpers
template <typename T>
concept IsTVector = requires(T V) {
	{ V.X } -> std::convertible_to<double>;
	{ V.Y } -> std::convertible_to<double>;
};
template <typename T>
concept IsScalar = std::is_arithmetic_v<T> && !IsTVector<T>;
