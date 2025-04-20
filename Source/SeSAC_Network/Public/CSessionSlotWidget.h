#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CSessionSlotWidget.generated.h"

UCLASS()
class SESAC_NETWORK_API UCSessionSlotWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* Text_RoomName;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* Text_HostName;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* Text_UserCount;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* Text_PingSpeed;

	// 세션조인
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* Button_Join;

public:
	virtual void NativeConstruct() override;

public:
	void Set(const struct FSessionInfo& InSessionInfo);

	UFUNCTION()
	void JoinSession();

public:
	int32 SessionNumber;

};
