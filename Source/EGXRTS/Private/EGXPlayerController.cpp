#include "EGXPlayerController.h"
#include "EGXCameraPawn.h"
#include "EGXPlanetActor.h"
#include "EGXPlanetTypes.h"
#include "EGXOrderTypes.h"
#include "EGXUnitBase.h"
#include "EGXBuildingBase.h"
#include "EGXMoveMarkerActor.h"
#include "EGXSurfacePlacementLibrary.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EngineUtils.h"
#include "InputActionValue.h"

AEGXPlayerController::AEGXPlayerController()
{
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
	DefaultMouseCursor = EMouseCursor::Crosshairs;
}

void AEGXPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	UE_LOG(LogTemp, Warning, TEXT("EGXPlayerController::BeginPlay - Controller class: %s"), *GetClass()->GetName());
	
	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* InputSubsystem =
			LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			if (DefaultMappingContext)
			{
				InputSubsystem->AddMappingContext(DefaultMappingContext, 0);
			}
		}
	}

	CachedCameraPawn = Cast<AEGXCameraPawn>(GetPawn());
}

void AEGXPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	CachedCameraPawn = Cast<AEGXCameraPawn>(InPawn);

	UE_LOG(LogTemp, Warning, TEXT("EGXPlayerController::OnPossess - Pawn: %s, Class: %s"),
		*GetNameSafe(InPawn),
		InPawn ? *InPawn->GetClass()->GetName() : TEXT("None"));
}

void AEGXPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	if (bIsDraggingSelection)
	{
		GetMousePosition(SelectionEndScreenPos.X, SelectionEndScreenPos.Y);
	}
}

void AEGXPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UE_LOG(LogTemp, Warning, TEXT("EGXPlayerController::SetupInputComponent"));
	
	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (SelectAction)
		{
			EIC->BindAction(SelectAction, ETriggerEvent::Started, this, &AEGXPlayerController::Input_SelectStarted);
			EIC->BindAction(SelectAction, ETriggerEvent::Completed, this, &AEGXPlayerController::Input_SelectCompleted);
		}

		if (CommandAction)
		{
			EIC->BindAction(CommandAction, ETriggerEvent::Started, this, &AEGXPlayerController::Input_CommandStarted);
		}

		if (CancelAction)
		{
			EIC->BindAction(CancelAction, ETriggerEvent::Started, this, &AEGXPlayerController::Input_CancelStarted);
		}
		
		if (CameraPanAction)
		{
			EIC->BindAction(CameraPanAction, ETriggerEvent::Triggered, this, &AEGXPlayerController::Input_CameraPan);
		}

		if (CameraZoomAction)
		{
			EIC->BindAction(CameraZoomAction, ETriggerEvent::Triggered, this, &AEGXPlayerController::Input_CameraZoom);
		}

		if (CameraRotateAction)
		{
			EIC->BindAction(CameraRotateAction, ETriggerEvent::Triggered, this, &AEGXPlayerController::Input_CameraRotate);
		}
	}
}

void AEGXPlayerController::Input_SelectStarted(const FInputActionValue& Value)
{
	GetMousePosition(SelectionStartScreenPos.X, SelectionStartScreenPos.Y);
	SelectionEndScreenPos = SelectionStartScreenPos;
	bIsDraggingSelection = true;
}

void AEGXPlayerController::Input_SelectCompleted(const FInputActionValue& Value)
{
	GetMousePosition(SelectionEndScreenPos.X, SelectionEndScreenPos.Y);

	const bool bAppend = IsShiftSelectionActive();
	const float DragDistance = FVector2D::Distance(SelectionStartScreenPos, SelectionEndScreenPos);

	if (DragDistance <= ClickSelectionThreshold)
	{
		HandleSingleClickSelection(bAppend);
	}
	else
	{
		HandleBoxSelection(bAppend);
	}

	bIsDraggingSelection = false;
}

void AEGXPlayerController::Input_CommandStarted(const FInputActionValue& Value)
{
	if (SelectedUnits.IsEmpty())
	{
		return;
	}

	FEGXSurfaceHit SurfaceHit;
	if (!GetMouseSurfaceHit(SurfaceHit))
	{
		return;
	}

	FEGXOrder Order;

	if (AEGXUnitBase* HitUnit = Cast<AEGXUnitBase>(SurfaceHit.HitActor))
		// CUrrently treats any clicked unit as attack-targetable
	{
		// Replace this with faction logic later.
		Order.Type = EEGXOrderType::Attack;
		Order.TargetActor = HitUnit;
		Order.TargetLocation = SurfaceHit.WorldLocation;
	}
	else
	{
		Order.Type = EEGXOrderType::Move;
		Order.TargetLocation = SurfaceHit.WorldLocation;

		if (MoveMarkerClass)
		{
			FTransform MarkerTransform;

			if (SurfaceHit.Planet)
			{
				MarkerTransform = UEGXSurfacePlacementLibrary::MakePlanetPlacementTransform(
					SurfaceHit.Planet,
					SurfaceHit.WorldLocation,
					8.f,
					0.f);
			}
			else
			{
				MarkerTransform = FTransform(FRotator::ZeroRotator, SurfaceHit.WorldLocation);
			}

			GetWorld()->SpawnActor<AEGXMoveMarkerActor>(
				MoveMarkerClass,
				MarkerTransform.GetLocation(),
				MarkerTransform.Rotator());
		}
	}

	IssueOrderToSelection(Order);
}

void AEGXPlayerController::Input_CancelStarted(const FInputActionValue& Value)
{
	// Later: cancel build mode, queued placement, etc.
	UE_LOG(LogTemp, Warning, TEXT("Input_CancelStarted"));
}

void AEGXPlayerController::Input_CameraPan(const FInputActionValue& Value)
{
	if (CachedCameraPawn)
	{
		CachedCameraPawn->Input_CameraPan(Value);
	}
}

void AEGXPlayerController::Input_CameraZoom(const FInputActionValue& Value)
{
	if (CachedCameraPawn)
	{
		CachedCameraPawn->Input_CameraZoom(Value);
	}
}

void AEGXPlayerController::Input_CameraRotate(const FInputActionValue& Value)
{
	const float Axis = Value.Get<float>();

	UE_LOG(LogTemp, Warning, TEXT("Input_CameraRotate Axis=%f Pawn=%s"),
		Axis,
		*GetNameSafe(CachedCameraPawn));

	if (CachedCameraPawn)
	{
		CachedCameraPawn->Input_CameraRotate(Value);
	}
}

bool AEGXPlayerController::GetMouseSurfaceHit(FEGXSurfaceHit& OutHit) const
{
	OutHit = FEGXSurfaceHit{};

	FHitResult HitResult;
	if (GetHitResultUnderCursor(ECC_Visibility, true, HitResult))
	{
		OutHit.bHit = true;
		OutHit.WorldLocation = HitResult.ImpactPoint;
		OutHit.SurfaceNormal = HitResult.ImpactNormal.GetSafeNormal();
		OutHit.HitActor = HitResult.GetActor();
		OutHit.Planet = Cast<AEGXPlanetActor>(HitResult.GetActor());
		return true;
	}

	if (!CachedCameraPawn)
	{
		return false;
	}

	FVector RayOrigin;
	FVector RayDirection;
	if (!DeprojectMousePositionToWorld(RayOrigin, RayDirection))
	{
		return false;
	}

	// Prefer active camera planet if available.
	if (AEGXPlanetActor* ActivePlanet = CachedCameraPawn->GetActivePlanet())
	{
		FVector HitPoint;
		FVector HitNormal;
		if (ActivePlanet->RaycastToPlanet(RayOrigin, RayDirection, HitPoint, HitNormal))
		{
			OutHit.bHit = true;
			OutHit.WorldLocation = HitPoint;
			OutHit.SurfaceNormal = HitNormal.GetSafeNormal();
			OutHit.Planet = ActivePlanet;
			return true;
		}
	}

	// Flat fallback: intersect the world Z=0 plane.
	const FPlane GroundPlane(FVector::ZeroVector, FVector::UpVector);
	const float PlaneHitT = FMath::RayPlaneIntersectionParam(RayOrigin, RayDirection, GroundPlane);

	if (PlaneHitT > 0.f && FMath::IsFinite(PlaneHitT))
	{
		OutHit.bHit = true;
		OutHit.WorldLocation = RayOrigin + RayDirection * PlaneHitT;
		OutHit.SurfaceNormal = FVector::UpVector;
		OutHit.Planet = nullptr;
		return true;
	}

	return false;
}

void AEGXPlayerController::HandleSingleClickSelection(bool bAppendSelection)
{
	FHitResult Hit;
	if (!GetHitResultUnderCursor(ECC_Visibility, true, Hit))
	{
		if (!bAppendSelection)
		{
			ClearSelection();
		}
		return;
	}

	AEGXUnitBase* HitUnit = Cast<AEGXUnitBase>(Hit.GetActor());
	AEGXBuildingBase* HitBuilding = Cast<AEGXBuildingBase>(Hit.GetActor());
	UE_LOG(LogTemp, Warning, TEXT("SingleClick Hit Actor: %s"), *GetNameSafe(Hit.GetActor()));
	if (!HitUnit && !HitBuilding)
	{
		if (!bAppendSelection)
		{
			ClearSelection();
		}
		return;
	}
	
	if (!bAppendSelection)
	{
		ClearSelection();
	}

	if (HitUnit && CanSelectUnit(HitUnit))
	{
		AddUnitToSelection(HitUnit);
	}
	else if (HitBuilding && CanSelectBuilding(HitBuilding))
	{
		AddBuildingToSelection(HitBuilding);
	}

	OnSelectionChanged.Broadcast(SelectedUnits.Num() + SelectedBuildings.Num());
}

void AEGXPlayerController::HandleBoxSelection(bool bAppendSelection)
{
	if (!bAppendSelection)
	{
		ClearSelection();
	}

	for (TActorIterator<AEGXUnitBase> It(GetWorld()); It; ++It)
	{
		AEGXUnitBase* Unit = *It;
		if (!CanSelectUnit(Unit))
		{
			continue;
		}

		FVector2D RectMin;
		FVector2D RectMax;
		if (!ProjectActorBoundsToScreenRect(Unit, RectMin, RectMax))
		{
			continue;
		}

		if (DoesScreenRectIntersectSelection(RectMin, RectMax))
		{
			AddUnitToSelection(Unit);
		}
	}

	for (TActorIterator<AEGXBuildingBase> It(GetWorld()); It; ++It)
	{
		AEGXBuildingBase* Building = *It;
		if (!CanSelectBuilding(Building))
		{
			continue;
		}

		FVector2D RectMin;
		FVector2D RectMax;
		if (!ProjectActorBoundsToScreenRect(Building, RectMin, RectMax))
		{
			continue;
		}

		if (DoesScreenRectIntersectSelection(RectMin, RectMax))
		{
			AddBuildingToSelection(Building);
		}
	}

	OnSelectionChanged.Broadcast(SelectedUnits.Num() + SelectedBuildings.Num());
}

void AEGXPlayerController::ClearSelection()
{
	for (AEGXUnitBase* Unit : SelectedUnits)
	{
		if (IsValid(Unit))
		{
			Unit->SetSelectedLocal(false);
		}
	}

	for (AEGXBuildingBase* Building : SelectedBuildings)
	{
		if (IsValid(Building))
		{
			Building->SetSelectedLocal(false);
		}
	}

	SelectedUnits.Empty();
	SelectedBuildings.Empty();

	OnSelectionChanged.Broadcast(0);
}

void AEGXPlayerController::AddUnitToSelection(AEGXUnitBase* Unit)
{
	if (!IsValid(Unit) || SelectedUnits.Contains(Unit))
	{
		return;
	}

	SelectedUnits.Add(Unit);
	Unit->SetSelectedLocal(true);
}

bool AEGXPlayerController::CanSelectUnit(AEGXUnitBase* Unit) const
{
	return IsValid(Unit);
	// if (!IsValid(Unit))
	// {
	// 	return false;
	// }
	//
	// return Unit->IsOwnedBy(this);
}

void AEGXPlayerController::AddBuildingToSelection(AEGXBuildingBase* Building)
{
	if (!IsValid(Building) || SelectedBuildings.Contains(Building))
	{
		return;
	}

	SelectedBuildings.Add(Building);
	Building->SetSelectedLocal(true);
}

bool AEGXPlayerController::CanSelectBuilding(AEGXBuildingBase* Building) const
{
	return IsValid(Building);
}

bool AEGXPlayerController::IsShiftSelectionActive() const
{
	return IsInputKeyDown(EKeys::LeftShift) || IsInputKeyDown(EKeys::RightShift);
}

void AEGXPlayerController::IssueOrderToSelection(const FEGXOrder& Order)
{
	UE_LOG(LogTemp, Warning, TEXT("IssueOrderToSelection: Type=%d Units=%d Buildings=%d Target=%s"),
		static_cast<int32>(Order.Type),
		SelectedUnits.Num(),
		SelectedBuildings.Num(),
		*Order.TargetLocation.ToString());

	for (AEGXUnitBase* Unit : SelectedUnits)
	{
		if (!IsValid(Unit))
		{
			continue;
		}

		Unit->IssueOrder(Order);
	}
}

bool AEGXPlayerController::IsPointInsideSelectionRect(const FVector2D& ScreenPosition) const
{
	const float MinX = FMath::Min(SelectionStartScreenPos.X, SelectionEndScreenPos.X);
	const float MaxX = FMath::Max(SelectionStartScreenPos.X, SelectionEndScreenPos.X);
	const float MinY = FMath::Min(SelectionStartScreenPos.Y, SelectionEndScreenPos.Y);
	const float MaxY = FMath::Max(SelectionStartScreenPos.Y, SelectionEndScreenPos.Y);

	return ScreenPosition.X >= MinX &&
		ScreenPosition.X <= MaxX &&
		ScreenPosition.Y >= MinY &&
		ScreenPosition.Y <= MaxY;
}

bool AEGXPlayerController::DoesScreenRectIntersectSelection(const FVector2D& RectMin, const FVector2D& RectMax) const
{
	const FVector2D SelMin(
		FMath::Min(SelectionStartScreenPos.X, SelectionEndScreenPos.X),
		FMath::Min(SelectionStartScreenPos.Y, SelectionEndScreenPos.Y));

	const FVector2D SelMax(
		FMath::Max(SelectionStartScreenPos.X, SelectionEndScreenPos.X),
		FMath::Max(SelectionStartScreenPos.Y, SelectionEndScreenPos.Y));

	const bool bNoOverlap =
		RectMax.X < SelMin.X ||
		RectMin.X > SelMax.X ||
		RectMax.Y < SelMin.Y ||
		RectMin.Y > SelMax.Y;

	return !bNoOverlap;
}

bool AEGXPlayerController::ProjectActorBoundsToScreenRect(const AActor* Actor, FVector2D& OutMin, FVector2D& OutMax) const
{
	if (!IsValid(Actor))
	{
		return false;
	}

	FVector Origin;
	FVector Extent;
	Actor->GetActorBounds(true, Origin, Extent);

	const FVector Corners[8] =
	{
		Origin + FVector(-Extent.X, -Extent.Y, -Extent.Z),
		Origin + FVector(-Extent.X, -Extent.Y,  Extent.Z),
		Origin + FVector(-Extent.X,  Extent.Y, -Extent.Z),
		Origin + FVector(-Extent.X,  Extent.Y,  Extent.Z),
		Origin + FVector( Extent.X, -Extent.Y, -Extent.Z),
		Origin + FVector( Extent.X, -Extent.Y,  Extent.Z),
		Origin + FVector( Extent.X,  Extent.Y, -Extent.Z),
		Origin + FVector( Extent.X,  Extent.Y,  Extent.Z)
	};

	bool bAnyProjected = false;
	FVector2D Min2D(FLT_MAX, FLT_MAX);
	FVector2D Max2D(-FLT_MAX, -FLT_MAX);

	for (const FVector& Corner : Corners)
	{
		FVector2D ScreenPos;
		if (ProjectWorldLocationToScreen(Corner, ScreenPos, true))
		{
			bAnyProjected = true;
			Min2D.X = FMath::Min(Min2D.X, ScreenPos.X);
			Min2D.Y = FMath::Min(Min2D.Y, ScreenPos.Y);
			Max2D.X = FMath::Max(Max2D.X, ScreenPos.X);
			Max2D.Y = FMath::Max(Max2D.Y, ScreenPos.Y);
		}
	}

	if (!bAnyProjected)
	{
		return false;
	}

	OutMin = Min2D;
	OutMax = Max2D;
	return true;
}
