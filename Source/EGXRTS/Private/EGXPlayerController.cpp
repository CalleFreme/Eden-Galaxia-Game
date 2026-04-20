#include "EGXPlayerController.h"
#include "EGXCameraPawn.h"
#include "EGXPlanetActor.h"
#include "EGXPlanetTypes.h"
#include "EGXOrderTypes.h"
#include "EGXUnitBase.h"

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
	}

	IssueOrderToSelection(Order);
}

void AEGXPlayerController::Input_CancelStarted(const FInputActionValue& Value)
{
	// Later: cancel build mode, queued placement, etc.
}

bool AEGXPlayerController::GetMouseSurfaceHit(FEGXSurfaceHit& OutHit) const
{
	OutHit = FEGXSurfaceHit{};

	// First try regular collision.
	FHitResult HitResult;
	if (GetHitResultUnderCursor(ECC_Visibility, true, HitResult))
	{
		OutHit.bHit = true;
		OutHit.WorldLocation = HitResult.ImpactPoint;
		OutHit.SurfaceNormal = HitResult.ImpactNormal;
		OutHit.HitActor = HitResult.GetActor();
		OutHit.Planet = Cast<AEGXPlanetActor>(HitResult.GetActor());

		// If we hit terrain that is not the planet actor directly, infer active planet from camera if possible.
		if (!OutHit.Planet && CachedCameraPawn && CachedCameraPawn->GetWorld())
		{
			// Leave null if unknown for now.
		}

		return true;
	}

	// Fallback: raycast against current planet sphere if nothing has collision yet.
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

	AEGXPlanetActor* Planet = nullptr;
	for (TActorIterator<AEGXPlanetActor> It(GetWorld()); It; ++It)
	{
		Planet = *It;
		break;
	}

	if (!Planet)
	{
		return false;
	}

	FVector HitPoint;
	FVector HitNormal;
	if (!Planet->RaycastToPlanet(RayOrigin, RayDirection, HitPoint, HitNormal))
	{
		return false;
	}

	OutHit.bHit = true;
	OutHit.WorldLocation = HitPoint;
	OutHit.SurfaceNormal = HitNormal;
	OutHit.Planet = Planet;
	return true;
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
	if (!HitUnit || !CanSelectUnit(HitUnit))
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

	AddUnitToSelection(HitUnit);
	OnSelectionChanged.Broadcast(SelectedUnits.Num());
}

void AEGXPlayerController::HandleBoxSelection(bool bAppendSelection)
{
	if (!bAppendSelection)
	{
		ClearSelection();
	}

	const float MinX = FMath::Min(SelectionStartScreenPos.X, SelectionEndScreenPos.X);
	const float MaxX = FMath::Max(SelectionStartScreenPos.X, SelectionEndScreenPos.X);
	const float MinY = FMath::Min(SelectionStartScreenPos.Y, SelectionEndScreenPos.Y);
	const float MaxY = FMath::Max(SelectionStartScreenPos.Y, SelectionEndScreenPos.Y);

	for (TActorIterator<AEGXUnitBase> It(GetWorld()); It; ++It)
	{
		AEGXUnitBase* Unit = *It;
		if (!CanSelectUnit(Unit))
		{
			continue;
		}

		FVector2D ScreenPosition;
		if (!ProjectWorldLocationToScreen(Unit->GetActorLocation(), ScreenPosition, true))
		{
			continue;
		}

		const bool bInside =
			ScreenPosition.X >= MinX &&
			ScreenPosition.X <= MaxX &&
			ScreenPosition.Y >= MinY &&
			ScreenPosition.Y <= MaxY;

		if (bInside)
		{
			AddUnitToSelection(Unit);
		}
	}

	OnSelectionChanged.Broadcast(SelectedUnits.Num());
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

	SelectedUnits.Empty();
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
	if (!IsValid(Unit))
	{
		return false;
	}

	return Unit->IsOwnedBy(this);
}

bool AEGXPlayerController::IsShiftSelectionActive() const
{
	return IsInputKeyDown(EKeys::LeftShift) || IsInputKeyDown(EKeys::RightShift);
}

void AEGXPlayerController::IssueOrderToSelection(const FEGXOrder& Order)
{
	for (AEGXUnitBase* Unit : SelectedUnits)
	{
		if (!IsValid(Unit))
		{
			continue;
		}

		Unit->IssueOrder(Order);
	}
}
