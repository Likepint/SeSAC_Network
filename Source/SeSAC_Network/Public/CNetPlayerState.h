#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "CNetPlayerState.generated.h"

UCLASS()
class SESAC_NETWORK_API ACNetPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
	virtual void BeginPlay() override;

	UFUNCTION(Reliable, Server)
	void ServerRPC_SetUserName(const FString& InName);
	void ServerRPC_SetUserName_Implementation(const FString& InName);

};
