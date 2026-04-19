#include "EGXWorkerUnit.h"
#include "EGXInventoryComponent.h"

AEGXWorkerUnit::AEGXWorkerUnit()
{
	Inventory = CreateDefaultSubobject<UEGXInventoryComponent>(TEXT("Inventory"));
}
