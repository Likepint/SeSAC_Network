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

		// 회전값 적용
		PitchAngle = -player->GetBaseAimRotation().GetNormalized().Pitch;
		PitchAngle = FMath::Clamp(PitchAngle, -60, 60);

		// 총소유 여부 적용
		bHasPistol = player->bHasPistol;

	}

}

void UCNetPlayerAnimInstance::PlayFireAnimation()
{
	if (bHasPistol and FireMontage)
		Montage_Play(FireMontage);

}
