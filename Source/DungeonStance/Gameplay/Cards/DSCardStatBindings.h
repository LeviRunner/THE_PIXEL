#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"

/**
 * Liga um "stat" de carta do protótipo (cards_*.ts, ex.: "crit_chance") a um atributo GAS.
 * Stats de gatilho (crit_heal_pct, burn_death_explode...) NÃO têm atributo: são lidos com
 * UDSStanceComponent::GetCardStat no ponto do gatilho, como no protótipo.
 */
struct FDSCardStatBinding
{
	/** Nome do stat no protótipo (FDSCardModifier::Stat). */
	FName CardStat;
	/** Nome curto do atributo (usado nas tags Data.Card.<Nome>.Add/.Mult). */
	FName AttributeName;
	FGameplayAttribute Attribute;
};

namespace DSCardStats
{
	DUNGEONSTANCE_API const TArray<FDSCardStatBinding>& GetBindings();
	DUNGEONSTANCE_API const FDSCardStatBinding* FindByCardStat(FName CardStat);
}
