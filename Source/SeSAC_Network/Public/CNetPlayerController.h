#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CNetPlayerController.generated.h"

UCLASS()
class SESAC_NETWORK_API ACNetPlayerController : public APlayerController
{
	GENERATED_BODY()
	
private:
	UPROPERTY()
	class ASeSAC_NetworkGameMode* GM;

public:
	virtual void BeginPlay() override;

	UFUNCTION(Reliable, Server)
	void ServerRPC_RespawnPlayer();
	void ServerRPC_RespawnPlayer_Implementation();

	// 사용할 위젯클래스
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class UCMainUI> MainUIWidget;

	// MainUIWidget으로부터 만들어진 인스턴스
	UPROPERTY()
	class UCMainUI* MainUI;

	// Spectator
	UFUNCTION(Reliable, Server)
	void ServerRPC_ChangeToSpectator();
	void ServerRPC_ChangeToSpectator_Implementation();

};
