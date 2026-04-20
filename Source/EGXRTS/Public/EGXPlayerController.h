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
	
	UFUNCTION(BlueprintCallable, Category="EGX RTS")
	bool GetMouseSurfaceHit(FEGXSurfaceHit& OutHit) const;

protected:

	void Input_SelectStarted(const FInputActionValue& Value);
	void Input_SelectCompleted(const FInputActionValue& Value);
	void Input_CommandStarted(const FInputActionValue& Value);
	void Input_CancelStarted(const FInputActionValue& Value);

	void HandleSingleClickSelection(bool bAppendSelection);
	void HandleBoxSelection(bool bAppendSelection);

	void ClearSelection();
	void AddUnitToSelection(AEGXUnitBase* Unit);
	bool CanSelectUnit(AEGXUnitBase* Unit) const;
	bool IsShiftSelectionActive() const;

	void IssueOrderToSelection(const FEGXOrder& Order);
	
	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputAction> SelectAction;

	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputAction> CommandAction;

	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputAction> CancelAction;

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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Selection")
	TArray<TObjectPtr<AEGXUnitBase>> SelectedUnits;

	UPROPERTY()
	TObjectPtr<AEGXCameraPawn> CachedCameraPawn;
};
