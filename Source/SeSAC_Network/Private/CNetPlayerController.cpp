#include "CNetPlayerController.h"
#include "SeSAC_NetworkGameMode.h"
#include "GameFramework/SpectatorPawn.h"

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

void ACNetPlayerController::ServerRPC_ChangeToSpectator_Implementation()
{
	// 관전자가 플레이어의 위치에 생성될 수 있도록 플레이어 정보를 가져온다.
	if (APawn* player = GetPawn())
	{
		// 관전자 생성
		FActorSpawnParameters params;
		params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		auto spectator = GetWorld()->SpawnActor<ASpectatorPawn>(GM->SpectatorClass, player->GetActorTransform(), params);

		// 빙의(Possess)
		Possess(spectator);

		// 이전 플레이어 제거
		player->Destroy();

		// 5초 후에 리스폰											// 현재 서버에서 호출하는 부분이므로
		FTimerHandle handle;										// 서버 RPC를 부를 필요가 없어 바로 Implementation 호출
		GetWorldTimerManager().SetTimer(handle, this, &ACNetPlayerController::ServerRPC_RespawnPlayer_Implementation, 5, false);
	}

}
