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

	// 총알 위젯이 추가될 패널
	UPROPERTY(BlueprintReadWrite, Category = "UI", meta = (BindWidget))
	class UUniformGridPanel* BulletPanel;

	// 총알 위젯 클래스
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Bullet")
	TSubclassOf<class UUserWidget> BulletUIFactory;

	// 총알 위젯 추가 함수
	void AddBullet();

	// 총알 제거
	void PopBullet(int32 index);

	// 모든 총알UI 제거
	void RemoveAllAmmo();

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "HP")
	float HP = 1;

	// DamageUI 애니메이션
	UPROPERTY(EditDefaultsOnly, Transient, Category = "MySettings", meta = (BindWidgetAnim))
	class UWidgetAnimation* DamageAnimation;

	// 피격처리 애니메이션
	void PlayDamageAnimation();

	// 게임 종료 버튼
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UHorizontalBox* GameOverUI;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* Button_Retry;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* Button_Exit;

public:
	// 사용자 목록
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* Text_Users;

public:
	// 리스폰
	virtual void NativeConstruct() override;

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UFUNCTION()
	void OnRetry();

	UFUNCTION()
	void OnExit();

};
