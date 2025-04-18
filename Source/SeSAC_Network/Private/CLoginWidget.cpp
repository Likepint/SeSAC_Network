#include "CLoginWidget.h"
#include "CNetGameInstance.h"
#include "Components/Button.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Components/EditableText.h"
#include "Components/WidgetSwitcher.h"
#include "CSessionSlotWidget.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"

void UCLoginWidget::NativeConstruct()
{
	Super::NativeConstruct();

	GameInstance = Cast<UCNetGameInstance>(GetWorld()->GetGameInstance());
	GameInstance->OnSearchCompleted.AddDynamic(this, &UCLoginWidget::AddSlotWidget);
	GameInstance->OnSearchState.AddDynamic(this, &UCLoginWidget::OnChangeButtonEnable);

	// Button 클릭 이벤트
	Button_CreateRoom->OnClicked.AddDynamic(this, &UCLoginWidget::CreateRoom);

	// Slider 이동 이벤트
	Slider_UserCount->OnValueChanged.AddDynamic(this, &UCLoginWidget::OnValueChanged);

	// Button 클릭 이벤트
	Button_CreateSession->OnClicked.AddDynamic(this, &UCLoginWidget::SwitchCreatePanel);
	Button_FindSession->OnClicked.AddDynamic(this, &UCLoginWidget::SwitchFindPanel);

	// Button 클릭 이벤트
	Button_CreateRoomToMain->OnClicked.AddDynamic(this, &UCLoginWidget::BackToMain);
	Button_FindRoomToMain->OnClicked.AddDynamic(this, &UCLoginWidget::BackToMain);

	// Button 클릭 이벤트
	Button_Find->OnClicked.AddDynamic(this, &UCLoginWidget::OnClickedFindSession);

}

void UCLoginWidget::CreateRoom()
{
	if (GameInstance and Edit_RoomName->GetText().IsEmpty() == false)
	{
		FString name = Edit_RoomName->GetText().ToString();
		int32 count = Slider_UserCount->GetValue();

		GameInstance->CreateMySession(name, count);
	}

}

void UCLoginWidget::OnValueChanged(float InVal)
{
	Text_UserCount->SetText(FText::AsNumber(InVal));

}

void UCLoginWidget::BackToMain()
{
	WidgetSwitcher->SetActiveWidget(0);

}

void UCLoginWidget::OnClickedFindSession()
{
	// 기존 슬롯이 있다면 모두 제거
	Scroll_RoomList->ClearChildren();

	if (GameInstance) // 세션 검색 요청
		GameInstance->FindOtherSession();

}

void UCLoginWidget::OnChangeButtonEnable(bool bIsSearching)
{
	Button_Find->SetIsEnabled(!bIsSearching);

	if (bIsSearching)
		Text_SearchingMsg->SetVisibility(ESlateVisibility::Visible);
	else Text_SearchingMsg->SetVisibility(ESlateVisibility::Hidden);

}

void UCLoginWidget::SwitchCreatePanel()
{
	WidgetSwitcher->SetActiveWidgetIndex(1);

}

void UCLoginWidget::SwitchFindPanel()
{
	WidgetSwitcher->SetActiveWidgetIndex(2);

	OnClickedFindSession();

}

void UCLoginWidget::AddSlotWidget(const FSessionInfo& InSessionInfo)
{
	auto slot = CreateWidget<UCSessionSlotWidget>(this, SessionInfoWidget);
	slot->Set(InSessionInfo);

	Scroll_RoomList->AddChild(slot);

}
