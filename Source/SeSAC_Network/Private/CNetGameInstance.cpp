#include "CNetGameInstance.h"
#include "OnlineSessionSettings.h"
#include "OnlineSessionClient.h"
#include "Online/OnlineSessionNames.h"
#include "SeSAC_Network.h"

void UCNetGameInstance::Init()
{
	Super::Init();

	if (auto subsystem = IOnlineSubsystem::Get())
	{
		// 서브시스템으로부터 세션인터페이스 가져오기
		SessionInterface = subsystem->GetSessionInterface();

		SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UCNetGameInstance::OnCreateSessionComplete);

		SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &UCNetGameInstance::OnFindSessionComplete);

		// Find 버튼 생성으로 주석
		//FTimerHandle handle;
		//GetWorld()->GetTimerManager().SetTimer(handle, FTimerDelegate::CreateLambda([&]()
		//																			{
		//																				FindOtherSession();
		//																			}), 2, false);
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
	settings.bUseLobbiesIfAvailable = true;
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

void UCNetGameInstance::FindOtherSession()
{
	// 델리게이트 Broadcast
	OnSearchState.Broadcast(true);

	// TSharedPtr 사용법 MakeShareable(new DataType())
	SessionSearch = MakeShareable(new FOnlineSessionSearch());

	// 1. 세션 검색 조건 설정
	SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

	// 2. LAN 여부 (CreateMySession() 함수에서 Steam인지 아닌지 비교했던 것과 동일)
	SessionSearch->bIsLanQuery = IOnlineSubsystem::Get()->GetSubsystemName() == FName("NULL");

	// 3. 최대 검색 세션 수
	SessionSearch->MaxSearchResults = 10;

	// 4. 세션 검색
	SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());

}

// 주어진 세션 검색 결과로부터 FSessionInfo 구조체를 생성하는 함수
FSessionInfo UCNetGameInstance::BuildSessionInfoFromResult(int32 index, const FOnlineSessionSearchResult& sr)
{
	FSessionInfo sessionInfo;
	sessionInfo.Index = index;

	// 세션 설정에서 방 이름(Room Name)과 호스트 이름(Host Name) 가져오기
	sr.Session.SessionSettings.Get(FName("ROOM_NAME"), sessionInfo.RoomName);
	sr.Session.SessionSettings.Get(FName("HOST_NAME"), sessionInfo.HostName);

	// 다국어 인코딩 과정
	sessionInfo.RoomName = StringBase64Decode(sessionInfo.RoomName);
	sessionInfo.HostName = StringBase64Decode(sessionInfo.HostName);

	// 최대 유저 수
	int32 maxUserCount = sr.Session.SessionSettings.NumPublicConnections;

	// 현재 입장한 유저 수 계산 (최대 인원 - 남은 슬롯 수)
	// NumOpenPublicConnections 값은 Steam에서는 정상 작동
	int32 currentUserCount = maxUserCount - sr.Session.NumOpenPublicConnections;

	// 사용자 수 포맷 문자열로 저장 (예: "(2/4)")
	sessionInfo.UserCount = FString::Printf(TEXT("(%d/%d)"), currentUserCount, maxUserCount);

	// 핑 정보 (Steam에서는 종종 9999로 출력됨)
	sessionInfo.PingSpeed = sr.PingInMs;

	return sessionInfo;

}

// 세션 검색 완료 시 호출되는 함수
void UCNetGameInstance::OnFindSessionComplete(bool bWasSuccessful)
{
	// 검색 실패 처리
	if (!bWasSuccessful)
	{
		// 델리게이트 Broadcast
		OnSearchState.Broadcast(false);

		PRINTLOG(TEXT("Session Search Failed ..."));

		return;
	}

	// 세션 검색 결과 리스트 가져오기
	const auto& results = SessionSearch->SearchResults;
	PRINTLOG(TEXT("Search Result Count : %d"), results.Num());

	// 검색된 세션 각각에 대해 정보 출력
	for (int32 i = 0; i < results.Num(); ++i)
	{
		const auto& sr = results[i];

		// 세션 정보가 유효한지 확인
		if (!sr.IsValid())
			continue;

		// 세션 정보를 구조체로 정리
		FSessionInfo sessionInfo = BuildSessionInfoFromResult(i, sr);

		// 세션 정보 출력
		PRINTLOG(TEXT("%s"), *sessionInfo.ToString());

		// 델리게이트로 위젯에 알려주기
		OnSearchCompleted.Broadcast(sessionInfo);

	}

	// 델리게이트 Broadcast
	OnSearchState.Broadcast(false);

}

// UTF-16 기반 FString을 Steam 서버에서도 안전하게 전달하기 위해 Base64 인코딩/디코딩을 사용
// Steam 서버는 내부적으로 UTF-8 기반 처리 추정 → 직접 문자열 전달 시 한글/특수문자 깨짐 발생 가능
// Base64는 ASCII 안전 문자만 사용하여 플랫폼 간 호환성이 뛰어남

// 문자열을 Base64 형식으로 인코딩 (FString → Base64 문자열)
FString UCNetGameInstance::StringBase64Encode(const FString& InStr)
{
	// 1. TCHAR 기반 FString을 UTF-8 문자열(std::string)로 변환
	std::string utf8Str = TCHAR_TO_UTF8(*InStr);

	// 2. UTF-8 문자열을 uint8 배열(TArray<uint8>)로 변환
	TArray<uint8> byteArray(reinterpret_cast<const uint8*>(utf8Str.c_str()), utf8Str.length());

	// 3. byte 배열을 Base64 문자열로 인코딩
	return FBase64::Encode(byteArray);

}

// Base64 문자열을 디코딩하여 FString으로 복원 (Base64 문자열 → FString)
FString UCNetGameInstance::StringBase64Decode(const FString& InStr)
{
	// 1. Base64 문자열을 디코딩하여 byte 배열로 변환
	TArray<uint8> byteArray;
	FBase64::Decode(InStr, byteArray);

	// 2. byte 배열을 UTF-8 문자열(std::string)로 해석
	std::string utf8Str(reinterpret_cast<const char*>(byteArray.GetData()), byteArray.Num());

	// 3. UTF-8 문자열을 TCHAR 기반 FString으로 변환
	return UTF8_TO_TCHAR(utf8Str.c_str());

}
