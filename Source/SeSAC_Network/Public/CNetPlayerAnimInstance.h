﻿#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "CNetPlayerAnimInstance.generated.h"

UCLASS()
class SESAC_NETWORK_API UCNetPlayerAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "MyAnimSettings")
	bool bHasPistol = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "MyAnimSettings")
	float Direction;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "MyAnimSettings")
	float Speed;

	UPROPERTY()
	class ASeSAC_NetworkCharacter* player;

	// 총쏘기에 사용할 몽타주
	UPROPERTY(EditDefaultsOnly, Category = "Anim")
	class UAnimMontage* FireMontage;

	// 총쏘기 애니메이션 재생
	void PlayFireAnimation();

	// 회전값 기억할 변수
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "MyAnimSettings")
	float PitchAngle;

public:
	virtual void NativeInitializeAnimation() override;

	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

};
