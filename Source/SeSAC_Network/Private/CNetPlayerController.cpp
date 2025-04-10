#include "CNetPlayerController.h"
#include "SeSAC_NetworkGameMode.h"

void ACNetPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
		GM = Cast<ASeSAC_NetworkGameMode>(GetWorld()->GetAuthGameMode());

}

void ACNetPlayerController::ServerRPC_RespawnPlayer_Implementation()
{
	auto player = GetPawn();
	UnPossess();

	player->Destroy();

	GM->RestartPlayer(this);

}
