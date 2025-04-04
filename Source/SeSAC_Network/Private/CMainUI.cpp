#include "CMainUI.h"
#include "Components/Image.h"

void UCMainUI::ShowCrossHair(bool bShow)
{
	if (bShow)
		 IMG_CrossHair->SetVisibility(ESlateVisibility::Visible);
	else IMG_CrossHair->SetVisibility(ESlateVisibility::Hidden);

}
