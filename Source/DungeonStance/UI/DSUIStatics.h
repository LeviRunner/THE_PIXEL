#pragma once

#include "CoreMinimal.h"
#include "Components/SlateWrapperTypes.h"
#include "UObject/Class.h"

class UDSUIDataAsset;
class UImage;
class UTexture2D;
class UUserWidget;
class UWidget;

/** Helpers de apresentação compartilhados pelos widgets DS (sem regra de jogo). */
namespace DSUI
{
	/** Data Asset de UI configurado no ADSHUD do jogador dono do widget. */
	UDSUIDataAsset* GetUIData(const UUserWidget* Widget);

	/** Aplica a textura (async) ou esconde a imagem quando a textura não estiver definida. */
	void SetImageFromSoftTexture(UImage* Image, const TSoftObjectPtr<UTexture2D>& Texture);

	/** Mostra/esconde um widget opcional (BindWidgetOptional pode ser nulo). */
	void SetWidgetShown(UWidget* Widget, bool bShown, ESlateVisibility ShownVisibility = ESlateVisibility::SelfHitTestInvisible);

	/** Razão segura 0..1. */
	float SafeRatio(float Value, float MaxValue);

	/** "12" acima de 10 s; "3,5" abaixo (formato de recarga). */
	FText FormatCooldownSeconds(float Seconds);

	/** "m:ss" para tempos de fase. */
	FText FormatClockSeconds(float Seconds);

	/** Lista os valores de um UENUM (sem o _MAX), opcionalmente pulando o valor 0 (None). */
	template <typename TEnum>
	TArray<TEnum> GetEnumValues(bool bSkipZero)
	{
		TArray<TEnum> Values;
		const UEnum* Enum = StaticEnum<TEnum>();
		if (!Enum)
		{
			return Values;
		}
		// NumEnums() inclui o _MAX gerado pela UHT.
		for (int32 Index = 0; Index < Enum->NumEnums() - 1; ++Index)
		{
			const int64 Value = Enum->GetValueByIndex(Index);
			if (bSkipZero && Value == 0)
			{
				continue;
			}
			Values.Add(static_cast<TEnum>(Value));
		}
		return Values;
	}

	/** Avança/volta circularmente numa lista de valores. */
	template <typename TEnum>
	TEnum CycleValue(const TArray<TEnum>& Values, TEnum Current, int32 Direction)
	{
		if (Values.Num() == 0)
		{
			return Current;
		}
		const int32 CurrentIndex = FMath::Max(0, Values.IndexOfByKey(Current));
		const int32 NextIndex = (CurrentIndex + Direction + Values.Num()) % Values.Num();
		return Values[NextIndex];
	}
}
