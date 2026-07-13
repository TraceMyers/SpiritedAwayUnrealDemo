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
	// GeneratorFuncType takes a normalized float and returns a float
	template<typename GeneratorFuncType>
	void GenerateSamples(int SampleCount, GeneratorFuncType Generator)
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

FORCEINLINE FVector NoZ(const FVector& V)
{
	return FVector(V.X, V.Y, 0);
}

// template specialization helpers
template <typename T>
concept IsTVector = requires(T V) {
	{ V.X } -> std::convertible_to<double>;
	{ V.Y } -> std::convertible_to<double>;
};
template <typename T>
concept IsScalar = std::is_arithmetic_v<T> && !IsTVector<T>;

namespace CreatureMath
{
	// linearly interpolate between points with a constant step, and without overshooting.
	// mirrors FMath::VInterpConstantTo, which only processes 3D vectors
	FVector2D VInterpConstantTo(const FVector2D& Current, const FVector2D& Target, float DeltaTime, float InterpSpeed);
	
	// imagine a line intersecting a circle. what is the half-distance of the line segment inside
	// the circle and what is the length of the arc from point to point?
	// OutRadius is the approximate radius of the circle and OutTheta is the angle of the intersected arc.
	// returns true if there is an arc, false if it's a straight line (OutRadius and OutTheta not set).
	bool SolveCircleArc(float InHalfWidth, float InArcLength, float& OutRadius, float& OutTheta);
	
	bool SolveCatenary(float HalfWidth, float ArcLength, float& OutScale);

	FVector2f SampleCatenary(float Scale, float ArcDistance);
	
	// StepPhase 0..1 over a SINGLE step (0 at heel-strike/double-support,
	// 0.5 at mid-stance, back to 1 at the next double-support).
	// One full speed oscillation per step — reuse verbatim for left and right.
	float CalculateImmediateWalkSpeed(float StepPhase, float AvgSpeed, float Amplitude);
	
	float SampleNormal(float Mean, float StandardDeviation, float X);
	
	// truncated normal from -3.5 to 3.5 standard deviations
	float SampleNormalCDF(float Alpha);
	
	// Distance and speed are signed, so -speed is in the same direction as -distance.
	// MaxDeceleration > 0, is always accelerating opposite the destination.
	float CalculateDeceleration(float Distance, float Speed, float MaxDeceleration);
	
	// signed. - displacement -> + accel
	double CalculateSpringAcceleration(double Stiffness, double Damping, double Displacement, double Speed, double Mass);
	
	void AngularDampen(FRotator& AngularVelocity, double Deceleration, float DeltaTime);
	
	template<IsTVector T>
	void Step(T& X, T TargetX, double XSpeed, float DeltaTime)
	{
		const T XDiff = TargetX - X;
		const float Delta = XSpeed * DeltaTime;
		const double Size = XDiff.Size();
		if (FMath::Abs(Delta) > Size)
		{
			X = TargetX;
		}
		else
		{
			X += XDiff * (Delta / Size);
		}
	}
	
	template<IsScalar T>
	void Step (T& X, T TargetX, float XSpeed, float DeltaTime)
	{
		const T XDiff = TargetX - X;
		const T Delta = FMath::Sign(XDiff) * XSpeed * DeltaTime;
		if (FMath::Abs(Delta) > FMath::Abs(XDiff))
		{
			X = TargetX;
		}
		else
		{
			X += Delta;
		}
	}
	
	void AccelerateAndDecelerate(double& SignedSpeed, double SignedDistance, double Acceleration, double Deceleration, double AbsMaxSpeed, float DeltaTime);
	
	FQuadraticSolution SolveQuadratic(float A, float B, float C, FSolveQuadraticParams Params = {false, 0});;
	
	template<typename T>
	T LinearSampleArrayUnsafe(const T* Array, int32 ArrayNum, float Alpha)
	{
		check(ArrayNum > 0)
		check(Alpha >= 0 && Alpha <= 1)
	
		const int32 IndexA = Alpha * (ArrayNum - 1);
		const int32 IndexB = FMath::Min(IndexA + 1, ArrayNum - 1);
	
		const float InvStepSize = (float)(ArrayNum - 1);
		const float StepDist = FMath::Max(Alpha - (float)IndexA / (ArrayNum - 1), 0);
		const float AB_Alpha = FMath::Clamp(StepDist * InvStepSize, 0, 1);
		
		return Array[IndexA] * (1 - AB_Alpha) + Array[IndexB] * AB_Alpha;
	}
	
	template<typename T>
	T LinearSampleArray(const TArray<T>& Array, float Alpha)
	{
		check(Array.Num() > 0)
		return LinearSampleArrayUnsafe(Array.GetData(), Array.Num(), Alpha);
	}
	
	// line trace helper, barely more convenient at this point. maybe shouldn't exist
	// this shouldn't be in math but given the project's size it'd be in a file by itself and there's enough of that already
	bool Raycast(const UWorld* World, FVector& OutLocation, const FVector& TrBegin, const FVector& TrEnd, ECollisionChannel=ECC_WorldStatic, double NormalOffset=0, FHitResult* OutResult=nullptr);
	
	void MakeBasis(const FVector& InX, FVector &OutY, FVector& OutZ);
}
