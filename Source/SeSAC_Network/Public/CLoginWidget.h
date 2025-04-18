#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CLoginWidget.generated.h"

UCLASS()
class SESAC_NETWORK_API UCLoginWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta = (BindWidget))
	class UEditableText* Edit_RoomName;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta = (BindWidget))
	class USlider* Slider_UserCount;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta = (BindWidget))
	class UTextBlock* Text_UserCount;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta = (BindWidget))
	class UButton* Button_CreateRoom;

public:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UWidgetSwitcher* WidgetSwitcher;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UButton* Button_CreateSession;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UButton* Button_FindSession;

	UFUNCTION()
	void SwitchCreatePanel();

	UFUNCTION()
	void SwitchFindPanel();

public:
	UPROPERTY()
	class UCNetGameInstance* GameInstance;

public:
	virtual void NativeConstruct() override;

public:
	UFUNCTION()
	void CreateRoom();

	// Slider Callback
	UFUNCTION()
	void OnValueChanged(float InVal);

public:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UButton* Button_CreateRoomToMain;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UButton* Button_FindRoomToMain;

	UFUNCTION()
	void BackToMain();

public:
	// 방 검색 버튼
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UButton* Button_Find;

	// 검색중 메시지
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UTextBlock* Text_SearchingMsg;

	// 방 찾기 버튼 클릭시 호출될 콜백
	UFUNCTION()
	void OnClickedFindSession();

	// 방 찾기 상태 이벤트 콜백
	UFUNCTION()
	void OnChangeButtonEnable(bool bIsSearching);

public: // Session Slot
	// Canvas_FindRoom의 스크롤 박스 위젯
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	class UScrollBox* Scroll_RoomList;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class UCSessionSlotWidget> SessionInfoWidget;

	UFUNCTION()
	void AddSlotWidget(const struct FSessionInfo& InSessionInfo);

};
