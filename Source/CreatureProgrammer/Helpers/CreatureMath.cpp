#include "CreatureMath.h"

namespace
{
	// 36 x 10. normal cdf from -3.5 to 0
	TStaticArray<float, 36*10> ZTable = {
		.00017,.00022,.00017,.00022,.00018,.00021,.00019,.00020,.00019,.00023,
		.00024,.00032,.00025,.00031,.00026,.00030,.00027,.00029,.00028,.00034,
		.00035,.00047,.00036,.00045,.00038,.00043,.00039,.00042,.00040,.00048,
		.00050,.00066,.00052,.00064,.00054,.00062,.00056,.00060,.00058,.00069,
		.00071,.00094,.00074,.00090,.00076,.00087,.00079,.00084,.00082,.00097,
		.00100,.00131,.00104,.00126,.00107,.00122,.00111,.00118,.00114,.00135,
		.00139,.00181,.00144,.00175,.00149,.00169,.00154,.00164,.00159,.00187,
		.00193,.00248,.00199,.00240,.00205,.00233,.00212,.00226,.00219,.00256,
		.00264,.00336,.00272,.00326,.00280,.00317,.00289,.00307,.00298,.00347,
		.00357,.00453,.00368,.00440,.00379,.00427,.00391,.00415,.00402,.00466,
		.00480,.00604,.00494,.00587,.00508,.00570,.00523,.00554,.00539,.00621,
		.00639,.00798,.00657,.00776,.00676,.00755,.00695,.00734,.00714,.00820,
		.00842,.01044,.00866,.01017,.00889,.00990,.00914,.00964,.00939,.01072,
		.01101,.01355,.01130,.01321,.01160,.01287,.01191,.01255,.01222,.01390,
		.01426,.01743,.01463,.01700,.01500,.01659,.01539,.01618,.01578,.01786,
		.01831,.02222,.01876,.02169,.01923,.02118,.01970,.02068,.02018,.02275,
		.02330,.02807,.02385,.02743,.02442,.02680,.02500,.02619,.02559,.02872,
		.02938,.03515,.03005,.03438,.03074,.03362,.03144,.03288,.03216,.03593,
		.03673,.04363,.03754,.04272,.03836,.04182,.03920,.04093,.04006,.04457,
		.04551,.05370,.04648,.05262,.04746,.05155,.04846,.05050,.04947,.05480,
		.05592,.06552,.05705,.06426,.05821,.06301,.05938,.06178,.06057,.06681,
		.06811,.07927,.06944,.07780,.07078,.07636,.07215,.07493,.07353,.08076,
		.08226,.09510,.08379,.09342,.08534,.09176,.08691,.09012,.08851,.09680,
		.09853,.11314,.10027,.11123,.10204,.10935,.10383,.10749,.10565,.11507,
		.11702,.13350,.11900,.13136,.12100,.12924,.12302,.12714,.12507,.13567,
		.13786,.15625,.14007,.15386,.14231,.15151,.14457,.14917,.14686,.15866,
		.16109,.18141,.16354,.17879,.16602,.17619,.16853,.17361,.17106,.18406,
		.18673,.20897,.18943,.20611,.19215,.20327,.19489,.20045,.19766,.21186,
		.21476,.23885,.21770,.23576,.22065,.23270,.22363,.22965,.22663,.24196,
		.24510,.27093,.24825,.26763,.25143,.26435,.25463,.26109,.25785,.27425,
		.27760,.30503,.28096,.30153,.28434,.29806,.28774,.29460,.29116,.30854,
		.31207,.34090,.31561,.33724,.31918,.33360,.32276,.32997,.32636,.34458,
		.34827,.37828,.35197,.37448,.35569,.37070,.35942,.36693,.36317,.38209,
		.38591,.41683,.38974,.41294,.39358,.40905,.39743,.40517,.40129,.42074,
		.42465,.45620,.42858,.45224,.43251,.44828,.43644,.44433,.44038,.46017,
		.46414,.49601,.46812,.49202,.47210,.48803,.47608,.48405,.48006,.50000
	};
}

FVector2D CreatureMath::VInterpConstantTo(const FVector2D& Current, const FVector2D& Target, float DeltaTime,
	float InterpSpeed)
{
	const FVector2D Delta = Target - Current;
	const FVector2D::FReal DeltaMSq = Delta.SizeSquared();
	const FVector2D::FReal MaxStep = InterpSpeed * DeltaTime;

	if (DeltaMSq > FMath::Square(MaxStep))
	{
		if (MaxStep > 0.0)
		{
			const FVector2D DeltaN = Delta / FMath::Sqrt(DeltaMSq);
			return Current + DeltaN * MaxStep;
		}
		else
		{
			return Current;
		}
	}

	return Target;
}

bool CreatureMath::SolveCircleArc(float HalfWidth, float ArcLength, float& OutRadius, float& OutTheta)
{
	const float Rho = 2.0f * HalfWidth / ArcLength;
	if (Rho >= 1.0f)
	{
		// straight segment
		return false;
	}
		
	// Newton-Raphson method, searching for the origin / tip of the arc
	constexpr int32 ITER_CT = 3;
	float T = FMath::Sqrt(6.0f * (1.0f - Rho));
	for (int i = 0; i < ITER_CT; i++)
	{
		float SinT, CosT;
		FMath::SinCos(&SinT, &CosT, T);
		T -= (SinT - Rho * T) / (CosT - Rho);
	}
		
	OutTheta = T;
	OutRadius = HalfWidth / FMath::Sin(T);
		
	return true;
}
bool CreatureMath::SolveCatenary(float HalfWidth, float ArcLength, float& OutScale)
{
	const float Sigma = ArcLength / (2.0f * HalfWidth);
	if (Sigma <= 1.0f)
	{
		// straight segment
		return false;
	}

	// Newton-Raphson method, solving sinh(U)/U = Sigma for U = HalfWidth / Scale
	// seed: small-U series for shallow bends, log asymptote (sinh U ~ e^U / 2) for deep bends
	constexpr int32 ITER_CT = 4;
	float U = (Sigma < 3.0f)
		? FMath::Sqrt(6.0f * (Sigma - 1.0f))
		: FMath::Loge(2.0f * Sigma * FMath::Loge(2.0f * Sigma));
	for (int32 i = 0; i < ITER_CT; i++)
	{
		// sinh and cosh share one exp
		const float E    = FMath::Exp(U);
		const float EInv = 1.0f / E;
		const float SinhU = 0.5f * (E - EInv);
		const float CoshU = 0.5f * (E + EInv);
		U -= (SinhU - Sigma * U) / (CoshU - Sigma);
	}

	OutScale = HalfWidth / U;

	return true;
}

FVector2f CreatureMath::SampleCatenary(float Scale, float ArcDistance)
{
	// ArcDistance measured from the apex, in [-ArcLength/2, ArcLength/2]
	// closed-form inverse of catenary arc length: s(x) = Scale * sinh(x / Scale)
	const float Hyp = FMath::Sqrt(Scale * Scale + ArcDistance * ArcDistance);
	const float X = Scale * asinh(ArcDistance / Scale);
	const float Y = Hyp - Scale;
	return FVector2f(X, Y);
}

float CreatureMath::CalculateImmediateWalkSpeed(float StepPhase, float AvgSpeed, float Amplitude)
{
	return AvgSpeed * (1.0f + Amplitude * FMath::Cos(2.0f * PI * StepPhase));
}

float CreatureMath::SampleNormal(float Mean, float StandardDeviation, float X)
{
	return 1.0f / (FMath::Sqrt(2*PI) * StandardDeviation) * FMath::Exp(-FMath::Square(X - Mean) / (2 * FMath::Square(StandardDeviation)));
}

float CreatureMath::SampleNormalCDF(float Alpha)
{
	if (Alpha <= 0.5)
	{
		return LinearSampleArrayUnsafe(ZTable.GetData(), ZTable.Num(), Alpha * 2);
	}
	else
	{
		return 1 - LinearSampleArrayUnsafe(ZTable.GetData(), ZTable.Num(), (1 - Alpha) * 2);
	}
}

float CreatureMath::CalculateDeceleration(float Distance, float Speed, float MaxDeceleration)
{
	check(MaxDeceleration >= 0)
	if (Speed == 0 || MaxDeceleration == 0)
	{
		return 0;
	}
	
	float DestinationTime = Distance / Speed;
	if (DestinationTime < 0) // opposite directions
	{
		return 0;		
	}
	
	float TimeUntilZeroSpeed = FMath::Abs(Speed) / MaxDeceleration;
	
	float NormDecel;
	if (FMath::Abs(DestinationTime) == 0)
	{
		NormDecel = TimeUntilZeroSpeed == 0 ? 0 : 1;
	}
	else
	{
		NormDecel = FMath::Min(TimeUntilZeroSpeed / DestinationTime, 1);
	}
	
	return NormDecel * MaxDeceleration;
}

double CreatureMath::CalculateSpringAcceleration(double Stiffness, double Damping, double Displacement, double Speed, double Mass)
{
	const double SpringForce = -Stiffness * Displacement; // F = -kx
	const double SpringDampingConstant = FMath::Sqrt(Stiffness * Mass) * Damping;
	const double SpringDampingForce = -Speed * SpringDampingConstant;
	return (SpringForce + SpringDampingForce) / Mass;
}

void CreatureMath::AngularDampen(FRotator& AngularVelocity, double Deceleration, float DeltaTime)
{
	LinearStep(AngularVelocity.Pitch, 0.0, Deceleration * DeltaTime);
	LinearStep(AngularVelocity.Yaw, 0.0, Deceleration * DeltaTime);
	LinearStep(AngularVelocity.Roll, 0.0, Deceleration * DeltaTime);
}

void CreatureMath::AccelerateAndDecelerate(double& SignedSpeed, double SignedDistance, double Acceleration,
                                           double Deceleration, double AbsMaxSpeed, float DeltaTime)
{
	const float Direction = FMath::Sign(SignedDistance);
	SignedSpeed = FMath::Clamp(SignedSpeed + Direction * Acceleration * DeltaTime, -AbsMaxSpeed, AbsMaxSpeed);
	const float DecelerationMoment = CalculateDeceleration(SignedDistance, SignedSpeed, Deceleration);
	SignedSpeed = FMath::Clamp(SignedSpeed - Direction * DecelerationMoment * DeltaTime, -AbsMaxSpeed, AbsMaxSpeed);
}

FQuadraticSolution CreatureMath::SolveQuadratic(float A, float B, float C, FSolveQuadraticParams Params)
{
	const float SqrtPart = FMath::Sqrt(FMath::Square(B) - 4.0f * A * C);
	const float InvDivisor = 1.0f / (2.0f * A);
	FQuadraticSolution Solution = {(-B - SqrtPart) * InvDivisor, (-B + SqrtPart) * InvDivisor};
	if (Params.bReplaceNan)
	{
		// todo: pretty sure if one is, the other is, and if one isn't the other isn't. test and remove one.
		if (FMath::IsNaN(Solution.A))
		{
			Solution.A = Params.NanReplacement;
		}
		if (FMath::IsNaN(Solution.B))
		{
			Solution.B = Params.NanReplacement;
		}
	}	
	return Solution;
}

bool CreatureMath::Raycast(const UWorld* World, FVector& OutLocation, const FVector& TrBegin, const FVector& TrEnd, ECollisionChannel CollisionChannel, double NormalOffset, FHitResult* OutResult)
{
	FHitResult LocalHitResult;
	FHitResult* HitResult = OutResult ? OutResult : &LocalHitResult;
	if (World->LineTraceSingleByChannel(*HitResult, TrBegin, TrEnd, CollisionChannel))
	{
		OutLocation = HitResult->ImpactPoint + HitResult->ImpactNormal * NormalOffset;
		return true;
	}
	else
	{
		OutLocation = TrEnd;
		return false;
	}
}

void CreatureMath::MakeBasis(const FVector& InX, FVector& OutY, FVector& OutZ)
{
	// I normalize and check for normality because cross products generate a little error, and multiple chained cross products
	// of normalized orthogonal vectors will accrue enough error to deviate from normality. not sure how many, but I've experienced it. 
	// probably double precision floats don't experience this problem so much, but it's a safe habit.
	check(InX.IsNormalized())
	const FVector XYPlaneVec = ((InX | FVector::XAxisVector) > 0.99) ? FVector::YAxisVector : FVector::XAxisVector;
	OutZ = InX.Cross(XYPlaneVec).GetSafeNormal();
	if (OutZ.Z < 0)
	{
		OutZ *= -1;
	}
	OutY = InX.Cross(OutZ).GetSafeNormal();
}

float FAlphaSampler::LinearSample(float Alpha) const
{
	return CreatureMath::LinearSampleArray(Samples, Alpha);
}
