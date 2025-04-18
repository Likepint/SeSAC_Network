#include "CSessionSlotWidget.h"
#include "CNetGameInstance.h"
#include "Components/TextBlock.h"

void UCSessionSlotWidget::Set(const FSessionInfo& InSessionInfo)
{
	Text_RoomName->SetText(FText::FromString(InSessionInfo.RoomName));
	Text_HostName->SetText(FText::FromString(InSessionInfo.HostName));
	Text_UserCount->SetText(FText::FromString(InSessionInfo.UserCount));
	Text_PingSpeed->SetText(FText::FromString(FString::Printf(TEXT("%dms"), InSessionInfo.PingSpeed)));

	SessionNumber = InSessionInfo.Index;

}
