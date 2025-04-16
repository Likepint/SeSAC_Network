#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "OnlineSubsystem.h"
#include "CNetGameInstance.generated.h"

UCLASS()
class SESAC_NETWORK_API UCNetGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;
	
public:
	void CreateMySession(FString InRoomName, int32 InUserCount);

	UFUNCTION()
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);

public:
	IOnlineSessionPtr SessionInterface;

	// Session(Host) Name
	FString MySessionName = "likepint";

};
