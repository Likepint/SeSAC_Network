﻿#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CNetActor.generated.h"

UCLASS()
class SESAC_NETWORK_API ACNetActor : public AActor
{
	GENERATED_BODY()
	
public:	
	ACNetActor();

protected:
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	
	virtual void Tick(float DeltaTime) override;

public:
	UPROPERTY(VisibleAnywhere)
	class UStaticMeshComponent* MeshComp;

	void PrintLog();

	// Owner 검출 영역
	UPROPERTY(EditAnywhere)
	float SearchDist = 200;

	// Owner 설정
	void FindOwner();

	// 회전 값 동기화 변수
	//UPROPERTY(Replicated)
	UPROPERTY(ReplicatedUsing = "OnRep_RotYaw")
	float RotYaw = 0;

	UFUNCTION()
	void OnRep_RotYaw();

	float CurrentTime = 0;
	float LastTime = 0;

	UPROPERTY()
	class UMaterialInstanceDynamic* Mat;
	
	// 재질에 동기화될 색상
	UPROPERTY(ReplicatedUsing = "OnRep_ChangeMatColor")
	FLinearColor MatColor;

	UFUNCTION()
	void OnRep_ChangeMatColor();

	UFUNCTION(Reliable, Server)
	void ServerRPC_ChangeColor(const FLinearColor InColor);
	void ServerRPC_ChangeColor_Implementation(const FLinearColor InColor);

	UFUNCTION(Client, Unreliable)
	void ClientRPC_ChangeColor(const FLinearColor InColor);
	void ClientRPC_ChangeColor_Implementation(const FLinearColor InColor);

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastRPC_ChangeColor(const FLinearColor InColor);
	void MulticastRPC_ChangeColor_Implementation(const FLinearColor InColor);

private:
	FTimerHandle handle;

};
