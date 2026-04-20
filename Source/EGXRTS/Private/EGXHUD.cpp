#include "EGXHUD.h"
#include "EGXPlayerController.h"

void AEGXHUD::DrawHUD()
{
	Super::DrawHUD();

	AEGXPlayerController* PC = Cast<AEGXPlayerController>(GetOwningPlayerController());
	if (!PC || !PC->IsDraggingSelection())
	{
		return;
	}

	const FVector2D Start = PC->GetSelectionStartScreenPos();
	const FVector2D End = PC->GetSelectionEndScreenPos();

	const float X = FMath::Min(Start.X, End.X);
	const float Y = FMath::Min(Start.Y, End.Y);
	const float W = FMath::Abs(End.X - Start.X);
	const float H = FMath::Abs(End.Y - Start.Y);

	if (W <= KINDA_SMALL_NUMBER || H <= KINDA_SMALL_NUMBER)
	{
		return;
	}

	DrawRect(SelectionFillColor, X, Y, W, H);

	DrawLine(X, Y, X + W, Y, SelectionBorderColor, SelectionBorderThickness);
	DrawLine(X + W, Y, X + W, Y + H, SelectionBorderColor, SelectionBorderThickness);
	DrawLine(X + W, Y + H, X, Y + H, SelectionBorderColor, SelectionBorderThickness);
	DrawLine(X, Y + H, X, Y, SelectionBorderColor, SelectionBorderThickness);
}