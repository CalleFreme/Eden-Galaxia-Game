#include "EGXInventoryComponent.h"

int32 UEGXInventoryComponent::GetAmount(EEGXResourceType Type) const
{
	if (const int32* Found = Resources.Find(Type))
	{
		return *Found;
	}
	return 0;
}

void UEGXInventoryComponent::AddResource(EEGXResourceType Type, int32 Delta)
{
	Resources.FindOrAdd(Type) += Delta;
}
