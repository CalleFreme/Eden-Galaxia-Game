#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "EGXOrderTypes.h"
#include "EGXPlayerController.generated.h"

class UEGXSelectionSubsystem;
class UInputAction;
class UInputMappingContext;
class AEGXCameraPawn;
class AEGXUnitBase;
class AEGXPlanetActor;
class AEGXBuildingBase;
class AHUD;
class AEGXMoveMarkerActor;
struct FInputActionValue;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEGXSelectionChangedSignature, int32, NumSelected);

UCLASS()
class EGXRTS_API AEGXPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AEGXPlayerController();
	
	virtual void BeginPlay() override;
	virtual void PlayerTick( float DeltaSeconds ) override;
	virtual void SetupInputComponent() override;
	virtual void OnPossess(APawn* Pawn) override;
	
	UFUNCTION(BlueprintCallable, Category="EGX RTS")
	bool GetMouseSurfaceHit(FEGXSurfaceHit& OutHit) const;
	
	bool IsDraggingSelection() const { return bIsDraggingSelection; }
	FVector2D GetSelectionStartScreenPos() const { return SelectionStartScreenPos; }
	FVector2D GetSelectionEndScreenPos() const { return SelectionEndScreenPos; }

protected:
	void Input_SelectStarted(const FInputActionValue& Value);
	void Input_SelectCompleted(const FInputActionValue& Value);
	void Input_CommandStarted(const FInputActionValue& Value);
	void Input_CancelStarted(const FInputActionValue& Value);
	void Input_CameraPan(const FInputActionValue& Value);
	void Input_CameraZoom(const FInputActionValue& Value);
	void Input_CameraRotate(const FInputActionValue& Value);

	void HandleSingleClickSelection(bool bAppendSelection);
	void HandleBoxSelection(bool bAppendSelection);

	void ClearSelection();
	void AddUnitToSelection(AEGXUnitBase* Unit);
	bool CanSelectUnit(AEGXUnitBase* Unit) const;
	bool IsShiftSelectionActive() const;
	
	void AddBuildingToSelection(AEGXBuildingBase* Building);
	bool CanSelectBuilding(AEGXBuildingBase* Building) const;
	void ClearBuildingSelection();
	
	bool IsPointInsideSelectionRect(const FVector2D& ScreenPosition) const;
	bool ProjectActorBoundsToScreenRect(const AActor* Actor, FVector2D& OutMin, FVector2D& OutMax) const;
	bool DoesScreenRectIntersectSelection(const FVector2D& RectMin, const FVector2D& RectMax) const;

	void IssueOrderToSelection(const FEGXOrder& Order);
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Selection")
	TArray<TObjectPtr<AEGXUnitBase>> SelectedUnits;

	UPROPERTY()
	TArray<TObjectPtr<AEGXBuildingBase>> SelectedBuildings;
	
	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputAction> SelectAction;

	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputAction> CommandAction;

	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputAction> CancelAction;
	
	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputAction> CameraPanAction = nullptr;

	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputAction> CameraZoomAction = nullptr;

	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputAction> CameraRotateAction = nullptr;

	UPROPERTY(BlueprintAssignable, Category="Selection")
	FEGXSelectionChangedSignature OnSelectionChanged;

	UPROPERTY(BlueprintReadOnly, Category="Selection")
	bool bIsDraggingSelection = false;

	UPROPERTY(BlueprintReadOnly, Category="Selection")
	FVector2D SelectionStartScreenPos = FVector2D::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category="Selection")
	FVector2D SelectionEndScreenPos = FVector2D::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Selection")
	float ClickSelectionThreshold = 8.f;

	UPROPERTY()
	TObjectPtr<AEGXCameraPawn> CachedCameraPawn;
	
	UPROPERTY(EditDefaultsOnly, Category="Selection")
	TSubclassOf<AEGXMoveMarkerActor> MoveMarkerClass;
};
