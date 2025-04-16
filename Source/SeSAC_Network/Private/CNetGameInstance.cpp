#include "CNetGameInstance.h"
#include "OnlineSessionSettings.h"
#include "OnlineSessionClient.h"
#include "SeSAC_Network.h"

void UCNetGameInstance::Init()
{
	Super::Init();

	if (auto subsystem = IOnlineSubsystem::Get())
	{
		// 서브시스템으로부터 세션인터페이스 가져오기
		SessionInterface = subsystem->GetSessionInterface();

		SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UCNetGameInstance::OnCreateSessionComplete);

		//FTimerHandle handle;

		//auto lambda = [&]()
		//	{
		//		CreateMySession(MySessionName, 10);
		//	};

		//GetWorld()->GetTimerManager().SetTimer(handle, lambda, 2, false);
	}

}

void UCNetGameInstance::CreateMySession(FString InRoomName, int32 InUserCount)
{
	// 세션 설정 변수
	FOnlineSessionSettings settings;

	// 1. Dedicated Server 접속 여부
	settings.bIsDedicated = false;

	// 2. 랜선(로컬)매칭을 할지 Steam 매칭을 할지 여부
	// Steam On/Off 유무로 로컬로 매칭할지를 결정
	// IOnlineSubsystem::Get()->GetSubsystemName()
	// GetSubsystem() 함수의 경우 Steam이 연결되어 있다면 Steam을 리턴
	// 아니라면 "NULL" 리턴
	FName name = IOnlineSubsystem::Get()->GetSubsystemName();
	settings.bIsLANMatch = (name == "NULL");

	// 3. 매칭이 온라인을 통해 노출될지 여부
	// false라면 초대를 통해서만 입장이 가능 (비공개세션)
	// SendSessionInviteToFriend() 함수를 통해 친구초대 가능
	settings.bShouldAdvertise = true;

	// 4. 온라인 상태(Presence) 정보를 활용할지 여부
	settings.bUsesPresence = true;

	// 5. 게임 진행중에 난입 가능 여부
	settings.bAllowJoinViaPresence = true;
	settings.bAllowJoinInProgress = true;

	// 6. 세션에 참여할 수 있는 공개(public) 연결의 최대 허용 수
	settings.NumPublicConnections = InUserCount;

	// 7. 커스텀 방 이름 설정
	settings.Set(FName("ROOM_NAME"), InRoomName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

	// 8. 호스트이름 설정
	settings.Set(FName("HOST_NAME"), MySessionName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

	PRINTLOG(TEXT("Create Session Start : %s"), *MySessionName);

	PRINTLOG(TEXT("Room Name : %s"), *InRoomName);

	// NetID
	// 플레이어는 각자 고유의 ID를 소유하므로, 해당 NetID를 로드
	FUniqueNetIdPtr net = GetWorld()->GetFirstLocalPlayerFromController()->GetUniqueNetIdForPlatformUser().GetUniqueNetId();

	SessionInterface->CreateSession(*net, FName(MySessionName), settings);

}

void UCNetGameInstance::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	PRINTLOG(TEXT("SessionName : %s, bWasSuccessful : %d"), *SessionName.ToString(), bWasSuccessful);

}
