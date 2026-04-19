#include "EGXJobSubsystem.h"

void UEGXJobSubsystem::EnqueueJob(const FEGXJobRecord& Job)
{
	PendingJobs.Add(Job);
	PendingJobs.Sort([](const FEGXJobRecord& A, const FEGXJobRecord& B)
	{
		return A.Priority > B.Priority;
	});
}

bool UEGXJobSubsystem::TryClaimBestJob(FEGXJobRecord& OutJob)
{
	if (PendingJobs.IsEmpty())
	{
		return false;
	}

	OutJob = PendingJobs[0];
	PendingJobs.RemoveAt(0);
	return true;
}
