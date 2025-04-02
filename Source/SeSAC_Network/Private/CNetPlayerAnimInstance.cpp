#include "CNetPlayerAnimInstance.h"
#include "SeSAC_NetworkCharacter.h"

void UCNetPlayerAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	player = Cast<ASeSAC_NetworkCharacter>(TryGetPawnOwner());

}

void UCNetPlayerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (player)
	{
		Direction = FVector::DotProduct(player->GetVelocity(), player->GetActorRightVector());

		Speed = FVector::DotProduct(player->GetVelocity(), player->GetActorForwardVector());

	}

}
