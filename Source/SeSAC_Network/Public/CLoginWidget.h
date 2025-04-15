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

};
