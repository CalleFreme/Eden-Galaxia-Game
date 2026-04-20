#include "EGXPlanetActor.h"
#include "ProceduralMeshComponent.h"
#include "Materials/MaterialInterface.h"

AEGXPlanetActor::AEGXPlanetActor()
{
	PrimaryActorTick.bCanEverTick = false;
	
	SceneRootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = SceneRootComponent;


	PlanetMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("PlanetMesh"));
	PlanetMesh->SetupAttachment(RootComponent);
	PlanetMesh->bUseComplexAsSimpleCollision = true;
	PlanetMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	PlanetMesh->SetCollisionObjectType(ECC_WorldStatic);
	PlanetMesh->SetCollisionResponseToAllChannels(ECR_Block);
	PlanetMesh->SetMobility(EComponentMobility::Static);
	PlanetMesh->SetGenerateOverlapEvents(false);
}

void AEGXPlanetActor::BeginPlay()
{
	Super::BeginPlay();

	PlanetCenter = GetActorLocation();
	GeneratePlanet();
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
	/* TO DO:
	* better UV seam handling
	* reducing pole distortion
	* using noise/biome displacement
	* splitting into chunks/patches for scalability
	* collision/nav strategy for very large planets
	 */
	if (!PlanetMesh)
	{
		return;
	}

	PlanetMesh->ClearAllMeshSections();

	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UVs;
	TArray<FProcMeshTangent> Tangents;
	TArray<FColor> VertexColors;

	const int32 NumLat = FMath::Max(Resolution, 8);
	const int32 NumLon = FMath::Max(Resolution * 2, 16);

	for (int32 Lat = 0; Lat <= NumLat; ++Lat)
	{
		const float V = (float)Lat / (float)NumLat;
		const float Theta = V * PI;

		for (int32 Lon = 0; Lon <= NumLon; ++Lon)
		{
			const float U = (float)Lon / (float)NumLon;
			const float Phi = U * PI * 2.f;

			const float X = FMath::Sin(Theta) * FMath::Cos(Phi);
			const float Y = FMath::Sin(Theta) * FMath::Sin(Phi);
			const float Z = FMath::Cos(Theta);

			const FVector UnitNormal(X, Y, Z);
			const FVector Position = UnitNormal * PlanetRadius;

			Vertices.Add(Position);
			Normals.Add(UnitNormal);
			UVs.Add(FVector2D(U, V));
			VertexColors.Add(FColor::White);

			const FVector TangentDir(-FMath::Sin(Phi), FMath::Cos(Phi), 0.f);
			Tangents.Add(FProcMeshTangent(TangentDir.GetSafeNormal(), false));
		}
	}

	for (int32 Lat = 0; Lat < NumLat; ++Lat)
	{
		for (int32 Lon = 0; Lon < NumLon; ++Lon)
		{
			const int32 Current = Lat * (NumLon + 1) + Lon;
			const int32 Next = Current + NumLon + 1;

			Triangles.Add(Current);
			Triangles.Add(Current + 1);
			Triangles.Add(Next);

			Triangles.Add(Current + 1);
			Triangles.Add(Next + 1);
			Triangles.Add(Next);
		}
	}

	PlanetMesh->CreateMeshSection(
		0,
		Vertices,
		Triangles,
		Normals,
		UVs,
		VertexColors,
		Tangents,
		true);
	
	if (PlanetMaterial)
	{
		
		PlanetMesh->SetMaterial(0, PlanetMaterial);
	}

	PlanetMesh->ContainsPhysicsTriMeshData(true);
}

FVector AEGXPlanetActor::GetSurfacePointFromNormal(const FVector& UnitNormal, float HeightOffset) const
{
	return GetActorLocation() + UnitNormal.GetSafeNormal() * (PlanetRadius + HeightOffset);
}

FRotator AEGXPlanetActor::GetSurfaceRotationFromNormal(const FVector& UnitNormal) const
{
	const FVector Up = UnitNormal.GetSafeNormal();
	return FRotationMatrix::MakeFromZ(Up).Rotator();
}

FVector AEGXPlanetActor::GetSurfaceNormalAt(const FVector& WorldPoint) const
{
	return (WorldPoint - PlanetCenter).GetSafeNormal();
}

FVector AEGXPlanetActor::ProjectPointToSurface(const FVector& WorldPoint, float HeightOffset) const
{
	const FVector Normal = GetSurfaceNormalAt(WorldPoint);
	return PlanetCenter + Normal * (PlanetRadius + HeightOffset);
}

bool AEGXPlanetActor::RaycastToPlanet(const FVector& RayOrigin, const FVector& RayDirection, FVector& OutPoint, FVector& OutNormal) const
{
	const FVector Dir = RayDirection.GetSafeNormal();
	const FVector L = RayOrigin - PlanetCenter;

	const float A = FVector::DotProduct(Dir, Dir);
	const float B = 2.f * FVector::DotProduct(Dir, L);
	const float C = FVector::DotProduct(L, L) - FMath::Square(PlanetRadius);

	const float Discriminant = B * B - 4.f * A * C;
	if (Discriminant < 0.f)
	{
		return false;
	}

	const float SqrtD = FMath::Sqrt(Discriminant);
	const float T0 = (-B - SqrtD) / (2.f * A);
	const float T1 = (-B + SqrtD) / (2.f * A);

	float T = TNumericLimits<float>::Max();
	if (T0 > 0.f) T = T0;
	else if (T1 > 0.f) T = T1;

	if (!FMath::IsFinite(T) || T == TNumericLimits<float>::Max())
	{
		return false;
	}

	OutPoint = RayOrigin + Dir * T;
	OutNormal = GetSurfaceNormalAt(OutPoint);
	return true;
}
