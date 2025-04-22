#include "CNetPlayerState.h"
#include "CNetGameInstance.h"

void ACNetPlayerState::BeginPlay()
{
	Super::BeginPlay();

	if (GetPlayerController() and GetPlayerController()->IsLocalController())
	{
		auto instance = Cast<UCNetGameInstance>(GetWorld()->GetGameInstance());

		ServerRPC_SetUserName(instance->MySessionName);
	}

}

void ACNetPlayerState::ServerRPC_SetUserName_Implementation(const FString& InName)
{
	SetPlayerName(InName);

}
