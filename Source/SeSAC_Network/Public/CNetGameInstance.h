#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "CNetGameInstance.generated.h"

// 세션 정보를 담는 구조체
USTRUCT(BlueprintType)
struct FSessionInfo
{
	GENERATED_BODY()

public:
	// 구조체 내용을 문자열로 출력 (예: [0] MyRoom : Alice - (2/4), 53ms)
	FORCEINLINE FString ToString() const
	{
		return FString::Printf(
			TEXT("[%d] %s : %s - %s, %dms"),
			Index, *RoomName, *HostName, *UserCount, PingSpeed
		);
	}

public:
	// 방 이름 (예: "My Cool Room")
	UPROPERTY(BlueprintReadOnly)
	FString RoomName;

	// 호스트 이름 (예: "Alice")
	UPROPERTY(BlueprintReadOnly)
	FString HostName;

	// 사용자 수 표현 (예: "(2/4)")
	UPROPERTY(BlueprintReadOnly)
	FString UserCount;

	// 핑 속도 (단위: ms)
	UPROPERTY(BlueprintReadOnly)
	int32 PingSpeed;

	// 검색된 세션의 인덱스 (예: 0, 1, 2, ...)
	UPROPERTY(BlueprintReadOnly)
	int32 Index;

};

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

public:
	// 방 검색
	void FindOtherSession();

	// 방 구조체 생성
	FSessionInfo BuildSessionInfoFromResult(int32 index, const FOnlineSessionSearchResult& sr);

	// 방 검색 결과
	void OnFindSessionComplete(bool bWasSuccessful);

	// 방 검색
	TSharedPtr<class FOnlineSessionSearch> SessionSearch;

public:
	// 다국어 인코딩
	FString StringBase64Encode(const FString& InStr);
	FString StringBase64Decode(const FString& InStr);

};
