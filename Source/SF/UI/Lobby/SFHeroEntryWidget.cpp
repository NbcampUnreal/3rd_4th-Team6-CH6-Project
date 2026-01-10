#include "SFHeroEntryWidget.h"

#include "Character/Hero/SFHeroDefinition.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void USFHeroEntryWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);

	HeroDefinition = Cast<USFHeroDefinition>(ListItemObject);
	if (HeroDefinition)
	{
		HeroIcon->GetDynamicMaterial()->SetTextureParameterValue(IconTextureMatParamName, HeroDefinition->LoadIcon());
		HeroNameText->SetText(FText::FromString(HeroDefinition->GetHeroDisplayName().ToUpper()));
	}
}

void USFHeroEntryWidget::SetSelected(bool bIsSelected)
{
	FLinearColor TargetColor = bIsSelected ? FLinearColor(1.0f, 0.8f, 0.0f, 0.7f) : FLinearColor::White;

	// 2. 이미지 위젯 자체의 색조(Tint)를 변경
	if (HeroIcon)
	{
		HeroIcon->SetColorAndOpacity(TargetColor);
	}
}
