#pragma once

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

public:
	virtual void NativeInitializeAnimation() override;

	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

};
