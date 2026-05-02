#include "EGXPlanetActor.h"

#include "Materials/MaterialInterface.h"
#include "ProceduralMeshComponent.h"
#include "Kismet/GameplayStatics.h"

namespace
{
	constexpr int32 NumCubeFaces = 6;

	uint8 UnitToByte(float Value)
	{
		return static_cast<uint8>(FMath::RoundToInt(FMath::Clamp(Value, 0.f, 1.f) * 255.f));
	}

	FVector MakePerpendicular(const FVector& Normal)
	{
		FVector Tangent = FVector::CrossProduct(FVector::UpVector, Normal);
		if (Tangent.IsNearlyZero())
		{
			Tangent = FVector::CrossProduct(FVector::ForwardVector, Normal);
		}
		return Tangent.GetSafeNormal();
	}
}

AEGXPlanetActor::AEGXPlanetActor()
{
	PrimaryActorTick.bCanEverTick = true;

	SceneRootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = SceneRootComponent;

	PlanetMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("PlanetMesh"));
	PlanetMesh->SetupAttachment(RootComponent);
	PlanetMesh->bUseComplexAsSimpleCollision = false;
	PlanetMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PlanetMesh->SetCollisionObjectType(ECC_WorldStatic);
	PlanetMesh->SetCollisionResponseToAllChannels(ECR_Block);
	PlanetMesh->SetMobility(EComponentMobility::Static);
	PlanetMesh->SetGenerateOverlapEvents(false);
	PlanetMesh->SetCastShadow(false);
}

void AEGXPlanetActor::BeginPlay()
{
	Super::BeginPlay();

	PlanetCenter = GetActorLocation();
	GeneratePlanet();
}

void AEGXPlanetActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!bEnableRuntimeLOD || !PlanetMesh)
	{
		return;
	}

	TimeSinceLastLODUpdate += DeltaSeconds;
	if (TimeSinceLastLODUpdate < LODUpdateIntervalSeconds)
	{
		return;
	}

	const FVector ViewLocation = GetLODViewLocation();
	if (!LastLODViewLocation.ContainsNaN() &&
		FVector::DistSquared(ViewLocation, LastLODViewLocation) < FMath::Square(LODCameraMoveThreshold))
	{
		TimeSinceLastLODUpdate = 0.f;
		return;
	}

	LastLODViewLocation = ViewLocation;
	TimeSinceLastLODUpdate = 0.f;
	RebuildPlanetMesh(false);
}

#if WITH_EDITOR
void AEGXPlanetActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	PlanetCenter = GetActorLocation();
	GeneratePlanet();
}
#endif

void AEGXPlanetActor::GeneratePlanet()
{
	PlanetCenter = GetActorLocation();
	PlanetMesh->bUseComplexAsSimpleCollision = bGenerateTerrainCollision;
	PlanetMesh->SetCollisionEnabled(bGenerateTerrainCollision ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
	PlanetMesh->SetCastShadow(bCastTerrainShadow);
	LastLODViewLocation = GetLODViewLocation();
	RebuildPlanetMesh(true);
}

FEGXPlanetSurfaceSample AEGXPlanetActor::SampleSurfaceFromNormal(const FVector& UnitNormal) const
{
	FEGXPlanetSurfaceSample Sample;
	Sample.SurfaceNormal = UnitNormal.GetSafeNormal();
	Sample.Coord.Normal = Sample.SurfaceNormal;
	Sample.BaseHeight = SampleBaseHeight(Sample.SurfaceNormal);
	Sample.DeformationHeight = SampleDeformationHeight(Sample.SurfaceNormal);
	Sample.FinalHeight = Sample.BaseHeight + Sample.DeformationHeight;
	Sample.Coord.Altitude = Sample.FinalHeight;
	Sample.SurfaceNormal = EstimateSurfaceNormal(Sample.Coord.Normal);
	Sample.WaterDepth = SampleWaterDepth(Sample.Coord.Normal, Sample.FinalHeight);
	Sample.Ecology = SampleEcology(Sample.Coord.Normal);
	Sample.Temperature = SampleTemperature(Sample.Coord.Normal, Sample.FinalHeight);
	Sample.Moisture = SampleMoisture(Sample.Coord.Normal, Sample.FinalHeight, Sample.WaterDepth);
	Sample.SlopeDegrees = EstimateSlopeDegrees(Sample.Coord.Normal, Sample.FinalHeight);
	Sample.Biome = ResolveBiome(Sample.Coord.Normal, Sample.FinalHeight, Sample.WaterDepth, Sample.Ecology, Sample.Temperature, Sample.Moisture);
	Sample.OreDensity = GetResourceDensityAt(Sample.Coord.Normal, EEGXResourceType::Ore);
	Sample.IceDensity = GetResourceDensityAt(Sample.Coord.Normal, EEGXResourceType::WaterIce);
	Sample.BiomassDensity = GetResourceDensityAt(Sample.Coord.Normal, EEGXResourceType::Biomass);
	Sample.WorldLocation = PlanetCenter + Sample.Coord.Normal * (PlanetRadius + Sample.FinalHeight);
	return Sample;
}

FEGXPlanetSurfaceSample AEGXPlanetActor::SampleSurfaceAtWorldPoint(const FVector& WorldPoint) const
{
	return SampleSurfaceFromNormal(GetSurfaceNormalAt(WorldPoint));
}

float AEGXPlanetActor::GetResourceDensityAt(const FVector& UnitNormal, EEGXResourceType ResourceType) const
{
	const FVector Normal = UnitNormal.GetSafeNormal();
	const float Height = SampleBaseHeight(Normal) + SampleDeformationHeight(Normal);
	const float WaterDepth = SampleWaterDepth(Normal, Height);
	const float Ecology = SampleEcology(Normal);

	switch (ResourceType)
	{
	case EEGXResourceType::Ore:
		{
			const float Geology = SampleNoise01(Normal, GenerationSettings.OreScale, 7100.f);
			const float MountainBias = FMath::Clamp((Height - GenerationSettings.ContinentHeight * 0.15f) / FMath::Max(GenerationSettings.MountainHeight, 1.f), 0.f, 1.f);
			return FMath::Clamp(Geology * 0.75f + MountainBias * 0.35f, 0.f, 1.f);
		}

	case EEGXResourceType::WaterIce:
	case EEGXResourceType::Ice:
		{
			const float LatitudeCold = 1.f - SampleTemperature(Normal, Height);
			const float AltitudeCold = FMath::Clamp(Height / FMath::Max(GenerationSettings.MaxTerrainHeight, 1.f), 0.f, 1.f) * 0.35f;
			return FMath::Clamp(LatitudeCold * 0.8f + AltitudeCold - WaterDepth * 0.01f, 0.f, 1.f);
		}

	case EEGXResourceType::Biomass:
		{
			const float Moisture = SampleMoisture(Normal, Height, WaterDepth);
			const float Temperature = SampleTemperature(Normal, Height);
			const float LandBias = WaterDepth > 0.f ? 0.35f : 1.f;
			return FMath::Clamp(Moisture * Temperature * Ecology * LandBias, 0.f, 1.f);
		}

	default:
		return 0.f;
	}
}

void AEGXPlanetActor::ApplyRadialHeightEdit(const FVector& WorldCenter, float Radius, float HeightDelta)
{
	if (Radius <= 0.f || FMath::IsNearlyZero(HeightDelta))
	{
		return;
	}

	const FVector CenterNormal = GetSurfaceNormalAt(WorldCenter);
	const float CosAngularRadius = FMath::Cos(Radius / FMath::Max(PlanetRadius, 1.f));
	const int32 CellCount = FMath::Max(MutableCellResolution, 8);

	for (int32 Face = 0; Face < NumCubeFaces; ++Face)
	{
		for (int32 Y = 0; Y < CellCount; ++Y)
		{
			for (int32 X = 0; X < CellCount; ++X)
			{
				const FVector2D UV((X + 0.5f) / CellCount, (Y + 0.5f) / CellCount);
				const FVector CellNormal = FaceUVToNormal(Face, UV);
				const float Dot = FVector::DotProduct(CenterNormal, CellNormal);
				if (Dot < CosAngularRadius)
				{
					continue;
				}

				const float AngularDistance = FMath::Acos(FMath::Clamp(Dot, -1.f, 1.f));
				const float SurfaceDistance = AngularDistance * PlanetRadius;
				const float FalloffAlpha = FMath::Clamp(SurfaceDistance / Radius, 0.f, 1.f);
				const float Falloff = 1.f - (FalloffAlpha * FalloffAlpha * (3.f - 2.f * FalloffAlpha));
				FEGXPlanetMutableCell& Cell = MutableCells.FindOrAdd(MakeMutableCellKey(Face, X, Y));
				Cell.HeightDelta += HeightDelta * Falloff;
			}
		}
	}

	RebuildPlanetMesh(true);
}

void AEGXPlanetActor::ApplyRadialEcologyEdit(const FVector& WorldCenter, float Radius, float EcologyDelta)
{
	if (Radius <= 0.f || FMath::IsNearlyZero(EcologyDelta))
	{
		return;
	}

	const FVector CenterNormal = GetSurfaceNormalAt(WorldCenter);
	const float CosAngularRadius = FMath::Cos(Radius / FMath::Max(PlanetRadius, 1.f));
	const int32 CellCount = FMath::Max(MutableCellResolution, 8);

	for (int32 Face = 0; Face < NumCubeFaces; ++Face)
	{
		for (int32 Y = 0; Y < CellCount; ++Y)
		{
			for (int32 X = 0; X < CellCount; ++X)
			{
				const FVector2D UV((X + 0.5f) / CellCount, (Y + 0.5f) / CellCount);
				const FVector CellNormal = FaceUVToNormal(Face, UV);
				const float Dot = FVector::DotProduct(CenterNormal, CellNormal);
				if (Dot < CosAngularRadius)
				{
					continue;
				}

				const float AngularDistance = FMath::Acos(FMath::Clamp(Dot, -1.f, 1.f));
				const float SurfaceDistance = AngularDistance * PlanetRadius;
				const float FalloffAlpha = FMath::Clamp(SurfaceDistance / Radius, 0.f, 1.f);
				const float Falloff = 1.f - (FalloffAlpha * FalloffAlpha * (3.f - 2.f * FalloffAlpha));
				FEGXPlanetMutableCell& Cell = MutableCells.FindOrAdd(MakeMutableCellKey(Face, X, Y));
				Cell.EcologyDelta = FMath::Clamp(Cell.EcologyDelta + EcologyDelta * Falloff, -1.f, 1.f);
			}
		}
	}

	RebuildPlanetMesh(true);
}

void AEGXPlanetActor::ClearMutableTerrain()
{
	MutableCells.Empty();
	RebuildPlanetMesh(true);
}

FVector AEGXPlanetActor::GetSurfacePointFromNormal(const FVector& UnitNormal, float HeightOffset) const
{
	const FEGXPlanetSurfaceSample Sample = SampleSurfaceFromNormal(UnitNormal);
	return Sample.WorldLocation + Sample.SurfaceNormal * HeightOffset;
}

FRotator AEGXPlanetActor::GetSurfaceRotationFromNormal(const FVector& UnitNormal) const
{
	const FVector Up = EstimateSurfaceNormal(UnitNormal);
	return FRotationMatrix::MakeFromZ(Up).Rotator();
}

FVector AEGXPlanetActor::GetSurfaceNormalAt(const FVector& WorldPoint) const
{
	return (WorldPoint - PlanetCenter).GetSafeNormal();
}

FVector AEGXPlanetActor::ProjectPointToSurface(const FVector& WorldPoint, float HeightOffset) const
{
	const FEGXPlanetSurfaceSample Sample = SampleSurfaceAtWorldPoint(WorldPoint);
	return Sample.WorldLocation + Sample.SurfaceNormal * HeightOffset;
}

bool AEGXPlanetActor::RaycastToPlanet(const FVector& RayOrigin, const FVector& RayDirection, FVector& OutPoint, FVector& OutNormal) const
{
	const FVector Dir = RayDirection.GetSafeNormal();
	if (Dir.IsNearlyZero())
	{
		return false;
	}

	const float BoundsRadius = PlanetRadius + GenerationSettings.MaxTerrainHeight + MaxExpectedDeformation + FMath::Abs(GenerationSettings.SeaLevel);
	const FVector L = RayOrigin - PlanetCenter;
	const float B = 2.f * FVector::DotProduct(Dir, L);
	const float C = FVector::DotProduct(L, L) - FMath::Square(BoundsRadius);
	const float Discriminant = B * B - 4.f * C;
	if (Discriminant < 0.f)
	{
		return false;
	}

	const float SqrtD = FMath::Sqrt(Discriminant);
	float NearT = (-B - SqrtD) * 0.5f;
	const float FarT = (-B + SqrtD) * 0.5f;
	NearT = FMath::Max(NearT, 0.f);

	if (FarT < NearT)
	{
		return false;
	}

	auto SignedDistanceToSurface = [this](const FVector& Point)
	{
		const FVector Normal = GetSurfaceNormalAt(Point);
		const FEGXPlanetSurfaceSample Sample = SampleSurfaceFromNormal(Normal);
		return FVector::Distance(Point, PlanetCenter) - (PlanetRadius + Sample.FinalHeight);
	};

	const int32 StepCount = 96;
	float PrevT = NearT;
	float PrevSigned = SignedDistanceToSurface(RayOrigin + Dir * PrevT);

	for (int32 Step = 1; Step <= StepCount; ++Step)
	{
		const float Alpha = static_cast<float>(Step) / static_cast<float>(StepCount);
		const float T = FMath::Lerp(NearT, FarT, Alpha);
		const float Signed = SignedDistanceToSurface(RayOrigin + Dir * T);

		if (Signed <= 0.f && PrevSigned >= 0.f)
		{
			float Lo = PrevT;
			float Hi = T;
			for (int32 Iter = 0; Iter < 10; ++Iter)
			{
				const float Mid = (Lo + Hi) * 0.5f;
				const float MidSigned = SignedDistanceToSurface(RayOrigin + Dir * Mid);
				if (MidSigned > 0.f)
				{
					Lo = Mid;
				}
				else
				{
					Hi = Mid;
				}
			}

			OutPoint = RayOrigin + Dir * Hi;
			OutNormal = EstimateSurfaceNormal(GetSurfaceNormalAt(OutPoint));
			return true;
		}

		PrevT = T;
		PrevSigned = Signed;
	}

	return false;
}

float AEGXPlanetActor::SampleBaseHeight(const FVector& UnitNormal) const
{
	const FVector Normal = UnitNormal.GetSafeNormal();
	const float ContinentRaw = SampleNoise01(Normal, GenerationSettings.ContinentScale, GenerationSettings.Seed * 0.37f);
	const float ContinentMask = FMath::Clamp(ContinentRaw + GenerationSettings.LandBias, 0.f, 1.f);
	const float ContinentShape = FMath::Pow(ContinentMask, GenerationSettings.ContinentSharpness);

	const float PlateMask = SampleRidgedNoise01(Normal, GenerationSettings.MountainScale * 0.42f, GenerationSettings.Seed * 1.19f);
	const float MountainGate = FMath::Clamp((PlateMask - (1.f - GenerationSettings.MountainCoverage)) / FMath::Max(GenerationSettings.MountainCoverage, 0.01f), 0.f, 1.f);
	const float MountainRidge = FMath::Pow(SampleRidgedNoise01(Normal, GenerationSettings.MountainScale, GenerationSettings.Seed * 1.73f), GenerationSettings.MountainSharpness);
	const float MountainMask = MountainGate * MountainRidge * ContinentShape;

	const float Hills = (SampleNoise01(Normal, GenerationSettings.HillScale, GenerationSettings.Seed * 2.11f) - 0.5f) * 2.f;
	const float Roughness = (SampleNoise01(Normal, GenerationSettings.RoughnessScale, GenerationSettings.Seed * 2.91f) - 0.5f) * 2.f;

	const float Shelf = (ContinentShape * 2.f - 0.85f) * GenerationSettings.ContinentHeight;
	const float PlainsMask = FMath::Clamp(1.f - MountainGate, 0.f, 1.f) * GenerationSettings.PlainsStrength;

	float Height = Shelf;
	Height += MountainMask * GenerationSettings.MountainHeight;
	Height += Hills * GenerationSettings.HillHeight * FMath::Lerp(1.f, 0.35f, PlainsMask);
	Height += Roughness * GenerationSettings.RoughnessHeight * FMath::Lerp(0.4f, 1.f, MountainGate);
	Height = SmoothTerrainByErosion(Height, ContinentShape, MountainMask);

	return FMath::Clamp(Height, -GenerationSettings.MaxTerrainHeight, GenerationSettings.MaxTerrainHeight);
}

float AEGXPlanetActor::SampleDeformationHeight(const FVector& UnitNormal) const
{
	const FEGXPlanetMutableCell* Cell = MutableCells.Find(MakeMutableCellKeyFromNormal(UnitNormal));
	return Cell ? Cell->HeightDelta : 0.f;
}

float AEGXPlanetActor::SampleEcology(const FVector& UnitNormal) const
{
	const FEGXPlanetMutableCell* Cell = MutableCells.Find(MakeMutableCellKeyFromNormal(UnitNormal));
	const float MutableDelta = Cell ? Cell->EcologyDelta : 0.f;
	const float BaseMoisture = SampleNoise01(UnitNormal, GenerationSettings.MoistureScale, GenerationSettings.Seed * 2.3f);
	const float LatitudeHealth = FMath::Clamp(1.f - FMath::Abs(UnitNormal.GetSafeNormal().Z) * 0.35f, 0.f, 1.f);
	return FMath::Clamp(GenerationSettings.BaseEcology * BaseMoisture * LatitudeHealth + MutableDelta, 0.f, 1.f);
}

float AEGXPlanetActor::SampleWaterDepth(const FVector& UnitNormal, float FinalHeight) const
{
	if (!GenerationSettings.bHasWater)
	{
		return 0.f;
	}

	return FMath::Max(GenerationSettings.SeaLevel - FinalHeight, 0.f);
}

float AEGXPlanetActor::SampleTemperature(const FVector& UnitNormal, float FinalHeight) const
{
	const float EquatorWarmth = 1.f - FMath::Pow(FMath::Abs(UnitNormal.GetSafeNormal().Z), GenerationSettings.PolarFalloff);
	const float AltitudeCooling = FMath::Clamp(FinalHeight / FMath::Max(GenerationSettings.MaxTerrainHeight, 1.f), 0.f, 1.f) * 0.28f;
	return FMath::Clamp(EquatorWarmth + GenerationSettings.TemperatureBias - AltitudeCooling, 0.f, 1.f);
}

float AEGXPlanetActor::SampleMoisture(const FVector& UnitNormal, float FinalHeight, float WaterDepth) const
{
	const float MoistureNoise = SampleNoise01(UnitNormal, GenerationSettings.MoistureScale, GenerationSettings.Seed * 3.17f);
	const float CoastalMoisture = WaterDepth > 0.f ? 0.35f : FMath::Clamp((GenerationSettings.SeaLevel + 450.f - FinalHeight) / 900.f, 0.f, 0.25f);
	const float MountainDrying = FMath::Clamp(FinalHeight / FMath::Max(GenerationSettings.MaxTerrainHeight, 1.f), 0.f, 1.f) * 0.25f;
	return FMath::Clamp(MoistureNoise + GenerationSettings.MoistureBias + CoastalMoisture - MountainDrying, 0.f, 1.f);
}

float AEGXPlanetActor::SampleNoise01(const FVector& UnitNormal, float Scale, float SeedOffset) const
{
	const FVector P = UnitNormal.GetSafeNormal() * PlanetRadius * Scale + FVector(SeedOffset, SeedOffset * 0.37f, SeedOffset * 0.73f);
	return FMath::Clamp(FMath::PerlinNoise3D(P) * 0.5f + 0.5f, 0.f, 1.f);
}

float AEGXPlanetActor::SampleRidgedNoise01(const FVector& UnitNormal, float Scale, float SeedOffset) const
{
	const float Noise = SampleNoise01(UnitNormal, Scale, SeedOffset);
	return FMath::Clamp(1.f - FMath::Abs(Noise * 2.f - 1.f), 0.f, 1.f);
}

float AEGXPlanetActor::SmoothTerrainByErosion(float Height, float ContinentMask, float MountainMask) const
{
	const float Erosion = FMath::Clamp(GenerationSettings.ErosionStrength, 0.f, 1.f);
	const float PlainMask = FMath::Clamp(GenerationSettings.PlainsStrength * (1.f - MountainMask) * ContinentMask, 0.f, 1.f);
	const float ErosionMask = Erosion * FMath::Lerp(1.f, 0.25f, MountainMask);
	const float ErodedHeight = FMath::Lerp(Height, Height * 0.72f, ErosionMask);
	return FMath::Lerp(ErodedHeight, ErodedHeight * 0.82f, PlainMask);
}

float AEGXPlanetActor::EstimateSlopeDegrees(const FVector& UnitNormal, float FinalHeight) const
{
	const FVector SurfaceNormal = EstimateSurfaceNormal(UnitNormal);
	return FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(FVector::DotProduct(SurfaceNormal, UnitNormal.GetSafeNormal()), -1.f, 1.f)));
}

FVector AEGXPlanetActor::EstimateSurfaceNormal(const FVector& UnitNormal) const
{
	const FVector N = UnitNormal.GetSafeNormal();
	const FVector T = MakePerpendicular(N);
	const FVector B = FVector::CrossProduct(N, T).GetSafeNormal();
	const float Epsilon = FMath::Max(PlanetRadius * 0.00015f, 10.f);
	const float Angle = Epsilon / FMath::Max(PlanetRadius, 1.f);

	const FVector N1 = FQuat(B, Angle).RotateVector(N).GetSafeNormal();
	const FVector N2 = FQuat(B, -Angle).RotateVector(N).GetSafeNormal();
	const FVector N3 = FQuat(T, Angle).RotateVector(N).GetSafeNormal();
	const FVector N4 = FQuat(T, -Angle).RotateVector(N).GetSafeNormal();

	const FVector P1 = N1 * (PlanetRadius + SampleBaseHeight(N1) + SampleDeformationHeight(N1));
	const FVector P2 = N2 * (PlanetRadius + SampleBaseHeight(N2) + SampleDeformationHeight(N2));
	const FVector P3 = N3 * (PlanetRadius + SampleBaseHeight(N3) + SampleDeformationHeight(N3));
	const FVector P4 = N4 * (PlanetRadius + SampleBaseHeight(N4) + SampleDeformationHeight(N4));

	const FVector Dx = P1 - P2;
	const FVector Dy = P3 - P4;
	FVector SurfaceNormal = FVector::CrossProduct(Dy, Dx).GetSafeNormal();
	if (FVector::DotProduct(SurfaceNormal, N) < 0.f)
	{
		SurfaceNormal *= -1.f;
	}
	return SurfaceNormal.IsNearlyZero() ? N : SurfaceNormal;
}

EEGXPlanetBiome AEGXPlanetActor::ResolveBiome(const FVector& UnitNormal, float FinalHeight, float WaterDepth, float Ecology, float Temperature, float Moisture) const
{
	if (WaterDepth > 25.f)
	{
		return Moisture > 0.55f && Ecology > 0.45f ? EEGXPlanetBiome::Wetlands : EEGXPlanetBiome::Barren;
	}

	if (Temperature < 0.18f)
	{
		return EEGXPlanetBiome::Ice;
	}

	if (FinalHeight > GenerationSettings.MountainHeight * 0.35f)
	{
		return EEGXPlanetBiome::Mountain;
	}

	if (Temperature < 0.32f)
	{
		return EEGXPlanetBiome::Tundra;
	}

	if (Moisture < 0.25f || Ecology < 0.2f)
	{
		return EEGXPlanetBiome::Desert;
	}

	return Ecology > 0.52f && Moisture > 0.38f ? EEGXPlanetBiome::Temperate : EEGXPlanetBiome::Barren;
}

FColor AEGXPlanetActor::MakeVertexColor(const FEGXPlanetSurfaceSample& Sample) const
{
	return VertexColorMode == EEGXPlanetVertexColorMode::DebugBiome
		? MakeDebugBiomeColor(Sample)
		: MakeMaterialMaskColor(Sample);
}

FColor AEGXPlanetActor::MakeMaterialMaskColor(const FEGXPlanetSurfaceSample& Sample) const
{
	const float HeightRange = FMath::Max(GenerationSettings.MaxTerrainHeight * 2.f, 1.f);
	const float NormalizedHeight = (Sample.FinalHeight + GenerationSettings.MaxTerrainHeight) / HeightRange;
	const float SlopeMask = FMath::Clamp(Sample.SlopeDegrees / 55.f, 0.f, 1.f);
	const float WetnessMask = FMath::Max(FMath::Clamp(Sample.WaterDepth / 700.f, 0.f, 1.f), Sample.Moisture * 0.35f);
	const float VegetationMask = FMath::Clamp(Sample.BiomassDensity * Sample.Ecology, 0.f, 1.f);

	// R: vegetation/biomass, G: slope/rock, B: water/wetness, A: normalized height.
	return FColor(
		UnitToByte(VegetationMask),
		UnitToByte(SlopeMask),
		UnitToByte(WetnessMask),
		UnitToByte(NormalizedHeight));
}

FColor AEGXPlanetActor::MakeDebugBiomeColor(const FEGXPlanetSurfaceSample& Sample) const
{
	if (Sample.WaterDepth > 0.f)
	{
		const uint8 Depth = static_cast<uint8>(FMath::Clamp(Sample.WaterDepth / 500.f, 0.f, 1.f) * 120.f + 80.f);
		return FColor(25, 70, Depth, 255);
	}

	switch (Sample.Biome)
	{
	case EEGXPlanetBiome::Ice:
		return FColor(210, 230, 235, 255);
	case EEGXPlanetBiome::Mountain:
		return FColor(115, 110, 100, 255);
	case EEGXPlanetBiome::Temperate:
		return FColor(65, 130, 70, 255);
	case EEGXPlanetBiome::Wetlands:
		return FColor(55, 105, 85, 255);
	case EEGXPlanetBiome::Desert:
		return FColor(165, 135, 82, 255);
	case EEGXPlanetBiome::Tundra:
		return FColor(135, 150, 145, 255);
	default:
		return FColor(95, 85, 72, 255);
	}
}

void AEGXPlanetActor::ApplyPlanetMaterialParameters()
{
	if (!PlanetMesh)
	{
		return;
	}

	PlanetMesh->SetScalarParameterValueOnMaterials(TEXT("EGX_PlanetRadius"), PlanetRadius);
	PlanetMesh->SetScalarParameterValueOnMaterials(TEXT("EGX_SeaLevel"), GenerationSettings.SeaLevel);
	PlanetMesh->SetScalarParameterValueOnMaterials(TEXT("EGX_MaxTerrainHeight"), GenerationSettings.MaxTerrainHeight);
	PlanetMesh->SetScalarParameterValueOnMaterials(TEXT("EGX_TerrainCollisionEnabled"), bGenerateTerrainCollision ? 1.f : 0.f);
	PlanetMesh->SetScalarParameterValueOnMaterials(TEXT("EGX_TerrainShadowEnabled"), bCastTerrainShadow ? 1.f : 0.f);
	PlanetMesh->SetScalarParameterValueOnMaterials(TEXT("EGX_VertexColorMode"), VertexColorMode == EEGXPlanetVertexColorMode::MaterialMasks ? 1.f : 0.f);
}

int64 AEGXPlanetActor::MakeMutableCellKey(int32 FaceIndex, int32 X, int32 Y) const
{
	return (static_cast<int64>(FaceIndex & 0xff) << 56) |
		(static_cast<int64>(X & 0x0fffffff) << 28) |
		static_cast<int64>(Y & 0x0fffffff);
}

int64 AEGXPlanetActor::MakeMutableCellKeyFromNormal(const FVector& UnitNormal) const
{
	int32 FaceIndex = 0;
	FVector2D UV;
	NormalToFaceUV(UnitNormal, FaceIndex, UV);

	const int32 CellCount = FMath::Max(MutableCellResolution, 8);
	const int32 X = FMath::Clamp(FMath::FloorToInt(UV.X * CellCount), 0, CellCount - 1);
	const int32 Y = FMath::Clamp(FMath::FloorToInt(UV.Y * CellCount), 0, CellCount - 1);
	return MakeMutableCellKey(FaceIndex, X, Y);
}

void AEGXPlanetActor::NormalToFaceUV(const FVector& UnitNormal, int32& OutFaceIndex, FVector2D& OutUV) const
{
	const FVector N = UnitNormal.GetSafeNormal();
	const FVector Abs(FMath::Abs(N.X), FMath::Abs(N.Y), FMath::Abs(N.Z));

	float U = 0.f;
	float V = 0.f;
	if (Abs.X >= Abs.Y && Abs.X >= Abs.Z)
	{
		if (N.X >= 0.f)
		{
			OutFaceIndex = 0;
			U = -N.Y / Abs.X;
			V = N.Z / Abs.X;
		}
		else
		{
			OutFaceIndex = 1;
			U = N.Y / Abs.X;
			V = N.Z / Abs.X;
		}
	}
	else if (Abs.Y >= Abs.X && Abs.Y >= Abs.Z)
	{
		if (N.Y >= 0.f)
		{
			OutFaceIndex = 2;
			U = N.X / Abs.Y;
			V = N.Z / Abs.Y;
		}
		else
		{
			OutFaceIndex = 3;
			U = -N.X / Abs.Y;
			V = N.Z / Abs.Y;
		}
	}
	else
	{
		if (N.Z >= 0.f)
		{
			OutFaceIndex = 4;
			U = N.X / Abs.Z;
			V = -N.Y / Abs.Z;
		}
		else
		{
			OutFaceIndex = 5;
			U = N.X / Abs.Z;
			V = N.Y / Abs.Z;
		}
	}

	OutUV = FVector2D(U * 0.5f + 0.5f, V * 0.5f + 0.5f);
}

FVector AEGXPlanetActor::FaceUVToNormal(int32 FaceIndex, const FVector2D& UV) const
{
	const float U = UV.X * 2.f - 1.f;
	const float V = UV.Y * 2.f - 1.f;

	switch (FaceIndex)
	{
	case 0:
		return FVector(1.f, -U, V).GetSafeNormal();
	case 1:
		return FVector(-1.f, U, V).GetSafeNormal();
	case 2:
		return FVector(U, 1.f, V).GetSafeNormal();
	case 3:
		return FVector(-U, -1.f, V).GetSafeNormal();
	case 4:
		return FVector(U, -V, 1.f).GetSafeNormal();
	default:
		return FVector(U, V, -1.f).GetSafeNormal();
	}
}

int32 AEGXPlanetActor::GetPatchResolutionForLOD(const FVector& PatchCenterNormal, const FVector& ViewLocation) const
{
	if (!bEnableRuntimeLOD)
	{
		return FMath::Max(Resolution, 2);
	}

	const FVector PatchWorld = PlanetCenter + PatchCenterNormal.GetSafeNormal() * PlanetRadius;
	const float DistanceRatio = FVector::Distance(ViewLocation, PatchWorld) / FMath::Max(PlanetRadius, 1.f);

	if (DistanceRatio <= LOD0DistanceRatio)
	{
		return FMath::Max(LOD0Resolution, 2);
	}
	if (DistanceRatio <= LOD1DistanceRatio)
	{
		return FMath::Max(LOD1Resolution, 2);
	}
	if (DistanceRatio <= LOD2DistanceRatio)
	{
		return FMath::Max(LOD2Resolution, 2);
	}
	return FMath::Max(LOD3Resolution, 2);
}

FVector AEGXPlanetActor::GetLODViewLocation() const
{
	if (const APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(this, 0))
	{
		return CameraManager->GetCameraLocation();
	}

	return PlanetCenter + FVector::UpVector * (PlanetRadius + 10000.f);
}

void AEGXPlanetActor::RebuildPlanetMesh(bool bForceFullRebuild)
{
	if (!PlanetMesh)
	{
		return;
	}

	const int32 PatchCount = FMath::Max(FacePatchCount, 1);
	const int32 TotalPatchCount = NumCubeFaces * PatchCount * PatchCount;
	if (bForceFullRebuild || LastPatchResolutions.Num() != TotalPatchCount)
	{
		PlanetMesh->ClearAllMeshSections();
		LastPatchResolutions.Init(INDEX_NONE, TotalPatchCount);
	}

	const FVector ViewLocation = GetLODViewLocation();
	int32 SectionIndex = 0;

	for (int32 Face = 0; Face < NumCubeFaces; ++Face)
	{
		for (int32 PatchY = 0; PatchY < PatchCount; ++PatchY)
		{
			for (int32 PatchX = 0; PatchX < PatchCount; ++PatchX)
			{
				TArray<FVector> Vertices;
				TArray<int32> Triangles;
				TArray<FVector> Normals;
				TArray<FVector2D> UVs;
				TArray<FProcMeshTangent> Tangents;
				TArray<FColor> VertexColors;

				const FVector2D PatchCenterUV(
					(static_cast<float>(PatchX) + 0.5f) / static_cast<float>(PatchCount),
					(static_cast<float>(PatchY) + 0.5f) / static_cast<float>(PatchCount));
				const FVector PatchCenterNormal = FaceUVToNormal(Face, PatchCenterUV);
				const int32 PatchResolution = GetPatchResolutionForLOD(PatchCenterNormal, ViewLocation);

				if (!bForceFullRebuild &&
					LastPatchResolutions.IsValidIndex(SectionIndex) &&
					LastPatchResolutions[SectionIndex] == PatchResolution)
				{
					++SectionIndex;
					continue;
				}

				if (LastPatchResolutions.IsValidIndex(SectionIndex))
				{
					LastPatchResolutions[SectionIndex] = PatchResolution;
				}

				const int32 VertexCount = (PatchResolution + 1) * (PatchResolution + 1);
				Vertices.Reserve(VertexCount);
				Normals.Reserve(VertexCount);
				UVs.Reserve(VertexCount);
				Tangents.Reserve(VertexCount);
				VertexColors.Reserve(VertexCount);

				for (int32 Y = 0; Y <= PatchResolution; ++Y)
				{
					for (int32 X = 0; X <= PatchResolution; ++X)
					{
						const float LocalU = static_cast<float>(X) / static_cast<float>(PatchResolution);
						const float LocalV = static_cast<float>(Y) / static_cast<float>(PatchResolution);
						const FVector2D FaceUV(
							(static_cast<float>(PatchX) + LocalU) / static_cast<float>(PatchCount),
							(static_cast<float>(PatchY) + LocalV) / static_cast<float>(PatchCount));

						const FVector UnitNormal = FaceUVToNormal(Face, FaceUV);
						const FEGXPlanetSurfaceSample Sample = SampleSurfaceFromNormal(UnitNormal);
						Vertices.Add(UnitNormal * (PlanetRadius + Sample.FinalHeight));
						Normals.Add(Sample.SurfaceNormal);
						UVs.Add(FaceUV);
						VertexColors.Add(MakeVertexColor(Sample));

						const FVector TangentDir = MakePerpendicular(UnitNormal);
						Tangents.Add(FProcMeshTangent(TangentDir, false));
					}
				}

				for (int32 Y = 0; Y < PatchResolution; ++Y)
				{
					for (int32 X = 0; X < PatchResolution; ++X)
					{
						const int32 Current = Y * (PatchResolution + 1) + X;
						const int32 Next = Current + PatchResolution + 1;

						Triangles.Add(Current);
						Triangles.Add(Current + 1);
						Triangles.Add(Next);

						Triangles.Add(Current + 1);
						Triangles.Add(Next + 1);
						Triangles.Add(Next);
					}
				}

				PlanetMesh->CreateMeshSection(
					SectionIndex,
					Vertices,
					Triangles,
					Normals,
					UVs,
					VertexColors,
					Tangents,
					bGenerateTerrainCollision);

				if (PlanetMaterial)
				{
					PlanetMesh->SetMaterial(SectionIndex, PlanetMaterial);
				}

				++SectionIndex;
			}
		}
	}

	ApplyPlanetMaterialParameters();
	PlanetMesh->ContainsPhysicsTriMeshData(true);
}
