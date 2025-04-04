#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CMainUI.generated.h"

UCLASS()
class SESAC_NETWORK_API UCMainUI : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadWrite, Category = "UI", meta = (BindWidget))
	class UImage* IMG_CrossHair;

	// 크로스헤어 on/off 처리함수
	void ShowCrossHair(bool bShow);

};
