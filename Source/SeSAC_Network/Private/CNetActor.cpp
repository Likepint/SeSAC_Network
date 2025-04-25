#include "CNetActor.h"
#include "Components/StaticMeshComponent.h"
#include "SeSAC_Network.h"
#include "SeSAC_NetworkCharacter.h"
#include "EngineUtils.h"
#include "Net/UnrealNetwork.h"
#include "CNetGameInstance.h"

ACNetActor::ACNetActor()
{
	PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	SetRootComponent(MeshComp);
	MeshComp->SetRelativeScale3D(FVector(0.5));

	bReplicates = true;

	// 대역폭 조정
	NetUpdateFrequency = 100;

}

void ACNetActor::BeginPlay()
{
	Super::BeginPlay();
	
	Mat = MeshComp->CreateDynamicMaterialInstance(0);

	if (HasAuthority())
	{
		auto instance = GetGameInstance<UCNetGameInstance>();

		GetWorld()->GetTimerManager().SetTimer(handle, [&, instance]()
											   {
												   if (instance->IsInRoom())
												   {
													   ServerRPC_ChangeColor(FLinearColor::MakeRandomColor());
												   }
											   }, 1, true);

		//FTimerHandle handle;
		// 
		//auto lambda = [&]()
		//	{
		//		FLinearColor MatColor = FLinearColor::MakeRandomColor();

		//		//OnRep_ChangeMatColor();
		//		ServerRPC_ChangeColor(MatColor);
		//	};

		//GetWorld()->GetTimerManager().SetTimer(handle, lambda, 1, true);
	}

}

void ACNetActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorld()->GetTimerManager().ClearTimer(handle);

	Super::EndPlay(EndPlayReason);

}

void ACNetActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	FindOwner();

	PrintLog();

	// Server일 경우
	if (HasAuthority())
	{
		// 서버 영역
		AddActorLocalRotation(FRotator(0, 50 * DeltaTime, 0));
		RotYaw = GetActorRotation().Yaw;
	}
	else
	{
		//// 클라이언트 영역
		//FRotator newRot = GetActorRotation();
		//newRot.Yaw = RotYaw;
		//SetActorRotation(newRot);

		// 클라이언트 자체 보간
		// 경과시간 증가
		CurrentTime += DeltaTime;

		// 0으로 나눠지지 않도록 LastTime 값 체크
		if (LastTime < KINDA_SMALL_NUMBER)
			return;

		// 이전 경과시간과 현재 경과시간의 비율계산
		float lerpRatio = CurrentTime / LastTime;
		
		// 이전 경과시간만큼 회전할 것으로 새로운 회전값 계산
		float newYaw = RotYaw + 50 * LastTime;

		// 예측되는 값으로 진행된 시간만큼 보간 처리
		float lerpYaw = FMath::Lerp(RotYaw, newYaw, lerpRatio);

		// 최종 적용
		FRotator curRot = GetActorRotation();
		curRot.Yaw = lerpYaw;
		SetActorRotation(curRot);
	}

}

void ACNetActor::PrintLog()
{
	const FString validConnection = GetNetConnection() != nullptr ? TEXT("Valid Connection") : TEXT("Invalid Connection");

	const FString ownerName = GetOwner() != nullptr ? GetOwner()->GetName() : TEXT("No Owner");

	const FString logStr = FString::Printf(TEXT("Connection : %s\n Owner Name : %s\n Local Role : %s\n Remote Role : %s"), *validConnection, *ownerName, *LOCAL_ROLE, *REMOTE_ROLE);

	DrawDebugString(GetWorld(), GetActorLocation() + GetActorUpVector() * 100, *logStr, nullptr, FColor::Red, 0, true, 1);

}

void ACNetActor::FindOwner()
{
	if (HasAuthority())
	{
		AActor* newOwner = nullptr;

		float minDist = SearchDist;

		for (TActorIterator<ASeSAC_NetworkCharacter> it(GetWorld()); it; ++it)
		{
			AActor* other = *it;
			float dist = GetDistanceTo(other);

			if (dist < minDist)
			{
				minDist = dist;
				newOwner = other;
			}
		}

		// Owner 설정
		if (GetOwner() != newOwner)
			SetOwner(newOwner);
	} // if HasAuthority

	DrawDebugSphere(GetWorld(), GetActorLocation(), SearchDist, 30, FColor::Yellow, false, 0, 0, 1);

}

void ACNetActor::OnRep_RotYaw()
{
	// 클라이언트 영역
	FRotator newRot = GetActorRotation();
	newRot.Yaw = RotYaw;
	SetActorRotation(newRot);

	// 업데이트된 경과 시간 저장
	LastTime = CurrentTime;

	// 경과 시간 초기화
	CurrentTime = 0;

}

// 색상동기화
void ACNetActor::OnRep_ChangeMatColor()
{
	if (Mat)
		Mat->SetVectorParameterValue(TEXT("FloorColor"), MatColor);

}

void ACNetActor::ServerRPC_ChangeColor_Implementation(const FLinearColor InColor)
{
	//if (Mat)
	//	Mat->SetVectorParameterValue(TEXT("FloorColor"), InColor);

	//ClientRPC_ChangeColor(InColor);
	MulticastRPC_ChangeColor(InColor);

}

void ACNetActor::ClientRPC_ChangeColor_Implementation(const FLinearColor InColor)
{
	if (Mat)
		Mat->SetVectorParameterValue(TEXT("FloorColor"), InColor);

}

void ACNetActor::MulticastRPC_ChangeColor_Implementation(const FLinearColor InColor)
{
	if (Mat)
		Mat->SetVectorParameterValue(TEXT("FloorColor"), InColor);

}

void ACNetActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACNetActor, RotYaw);

	DOREPLIFETIME(ACNetActor, MatColor);

}
