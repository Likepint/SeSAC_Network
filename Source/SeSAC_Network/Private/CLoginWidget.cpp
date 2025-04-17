#include "CLoginWidget.h"
#include "CNetGameInstance.h"
#include "Components/Button.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Components/EditableText.h"
#include "Components/WidgetSwitcher.h"

void UCLoginWidget::NativeConstruct()
{
	Super::NativeConstruct();

	GameInstance = Cast<UCNetGameInstance>(GetWorld()->GetGameInstance());

	// Button 클릭 이벤트
	Button_CreateRoom->OnClicked.AddDynamic(this, &UCLoginWidget::CreateRoom);

	// Slider 이동 이벤트
	Slider_UserCount->OnValueChanged.AddDynamic(this, &UCLoginWidget::OnValueChanged);

	// Button 클릭 이벤트
	Button_CreateSession->OnClicked.AddDynamic(this, &UCLoginWidget::SwitchCreatePanel);
	Button_FindSession->OnClicked.AddDynamic(this, &UCLoginWidget::SwitchFindPanel);

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

void UCLoginWidget::SwitchCreatePanel()
{
	WidgetSwitcher->SetActiveWidgetIndex(1);

}

void UCLoginWidget::SwitchFindPanel()
{
	WidgetSwitcher->SetActiveWidgetIndex(2);

}
