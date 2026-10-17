# Gameplay — Dungeon Stance & Cast Framework

Sistema central de combate em grupo (1 a 5 heróis), construído sobre o **Gameplay Ability System (GAS)**, com servidor autoritativo.
Fonte de verdade das regras: `Nakama_Legacy_Reference/src/` (ver [`ESPEC_REGRAS_DUNGEON_STANCE.md`](../../../docs/ESPEC_REGRAS_DUNGEON_STANCE.md)). Unidades: **cm** (1 px do protótipo = 2,5 cm), segundos.

## Mapa de pastas

| Pasta | Conteúdo |
| --- | --- |
| `DSGameplayTagsGameplay.h/.cpp` | Tags só da Gameplay (`Character.*`, `Cooldown.Support.*`, `Data.Buff.*`, `Data.Card.<Attr>.Add/Mult`, `Event.Crit`…) |
| `Abilities/` | `UDSAbilitySystemComponent` (input por tag, sets), `UDSAbilitySet`, `UDSGameplayAbility` (base), `UDSGA_BasicAttack`, `UDSGA_Dodge`, `UDSGA_Sprint`, `UDSGA_AreaStrike` + `UDSGA_Whirlwind`, `UDSGA_SupportAura` + `UDSSupportAbilityData` + `DSSupportResolver` |
| `Attributes/` | `UDSAttributeSet` |
| `Effects/` | `UDSDamageExecution`, GEs genéricos (`DSGameplayEffects.h`), GEs de status (`DSStatusEffects.h`), `UDSStatusLibrary`, `DSBuffHelpers` |
| `Combat/` | `DSDamageMath.h` e `DSEnemyRules.h` (fórmulas puras), `UDSTargetingLibrary`, `ADSProjectile`, `ADSSupportZone` |
| `Characters/` | `ADSCharacterBase`, `ADSHeroCharacter`, `ADSEnemyCharacter`, `UDSTargetLockComponent`, `DSPlayerStateBridge` |
| `Data/` | `UDSClassData`, `UDSEnemyData`, `UDSCardData` + `FDSCardDefinition` + `UDSCardRegistry`, `UDSGameplaySettings`; `Import/DT_Cards.json` (192 cartas) e `Import/SupportAbilities.json` |
| `Cards/` | `UDSCardLibrary` (cards.ts), `DSCardStatBindings` (stat de carta → atributo) |
| `Stance/` | `UDSStanceComponent` |
| `AI/` | `ADSEnemyAIController` |
| `Tests/` | Automation Tests: `DungeonStance.Gameplay.Cards.*`, `DungeonStance.Gameplay.Damage.*` |

## API usada por outras áreas

### Core → Gameplay

| Chamada | Observações |
| --- | --- |
| `ADSEnemyCharacter::InitializeFromData(const UDSEnemyData*, int32 Level, float HealthMultiplier, float DamageMultiplier, bool bElite)` | Servidor, logo após o spawn. `MaxHealth = vida(L) × HealthMultiplier`. |
| `ADSEnemyCharacter::OnEnemyDied` (`FDSOnEnemyDied(ADSEnemyCharacter*, AController* Killer)`) | Nativo, servidor. Killer = PlayerController do herói (via PlayerState dono do ASC). |
| `ADSCharacterBase::OnDeath`, `IsAlive()` | `IsAlive()` = sem `State.Dead` e sem `State.Downed`. |
| `ADSHeroCharacter` → `ADSGameMode::NotifyHeroDowned(this)` | A Gameplay chama quando a vida do herói zera (herói fica `State.Downed`, não morre). |
| `ADSHeroCharacter::Revive(float HealthRatio)` | Servidor. O `ReviveHero` atual do Core (mexe só na vida) também funciona: vida > 0 limpa o estado abatido. |
| `UDSCardLibrary::RollCardChoices(EDSStance, const TArray<FDSOwnedCard>&, int32 NumChoices, FRandomStream&) -> TArray<FName>` | Sorteio ponderado sem reposição, só cartas sorteáveis (máx. de acúmulos + pai possuído). |
| `UDSCardLibrary::ValidateCardPick(FName, EDSStance, const TArray<FDSOwnedCard>&, int32 PendingPicks, FString& OutReason) -> bool` | Mensagens iguais às do protótipo (`"parent not owned"`…). |
| `UDSCardLibrary::ApplyCardToPlayer(ADSPlayerState*, FName CardId) -> bool` | **Chamado antes de `AddCardStack`**: conta +1 acúmulo. Reaplica `UDSGE_CardStats` e concede a habilidade de apoio no 1º slot livre. |
| `UDSAbilitySystemComponent::GrantAbilitySet`, `AbilityInputTagPressed/Released`, `ProcessAbilityInput` | O herói local chama `ProcessAbilityInput` no `Tick` (o PlayerController não precisa). |

Overloads com `const UDSCardRegistry*` explícito existem para testes/ferramentas. Sem registro explícito, a biblioteca usa `UDSGameplaySettings::CardRegistry` (`DefaultGame.ini`, seção `[/Script/DungeonStance.DSGameplaySettings]`, padrão `/Game/DungeonStance/Data/Cards/DA_CardRegistry`) ou `UDSCardLibrary::SetCardRegistryOverride`.

### UI → Gameplay (leitura)

Atributos via `UDSAttributeSet` (replicados), `UDSTargetLockComponent::OnTargetLockChanged`, `UDSStanceComponent::GetCardStat/GetCardStacks`, tags `State.*`/`Status.*` no ASC, recargas pelas tags `Cooldown.*` (`Cooldown.Dodge`, `Cooldown.ClassSkill`, `Cooldown.Support.<Habilidade>`).

## Como funciona

- **Atributos** (`UDSAttributeSet`): os 15 do contrato + `Lifesteal`, `DamageReduction`, `DodgeChance`, `HealthRegen`, `ResourceRegen`; meta `IncomingDamage`/`IncomingHeal`. Clamp em `PreAttributeChange`/`PreAttributeBaseChange` (CDR máx. 60%, esquiva máx. 75%). Dano: `IncomingDamage → Shield → Health`; `State.Invulnerable` zera o dano; vida 0 → `OnOutOfHealth` (herói: abatido; inimigo: `State.Dead` + `Event.Death` + `OnEnemyDied`). Roubo de vida cura o atacante.
- **Dano** (`UDSDamageExecution`, fórmulas em `DSDamageMath.h`): base (`Data.Damage` fixo, ou `Dano × Data.Magnitude`) × crítico × (1 − Enfraquecido) × (1 + Marca) × (1 + Exposto) × armadura (`100/(100+A)` herói, `200/(200+A)` monstro) × (1 − redução). Críticos disparam `Event.Crit` no atacante (gancho para cartas "ao critar").
- **Habilidades**: base `UDSGameplayAbility` (LocalPredicted, InstancedPerActor; bloqueada por `State.Dead/Downed`, `Status.Stun`). Recarga = `UDSGE_Cooldown` com `Data.Duration` e tags dinâmicas; custo = `UDSGE_Cost` sobre `Resource`. Ataque básico: recarga `1/AttackSpeed`, política `WhileInputActive` (segurar). Esquiva: root motion (425 cm/0,2 s), i-frames via `UDSGE_Invulnerable`. Sprint: `State.Sprinting` (×1,6), gasta 20/s, regenera 28/s fora do sprint, mínimo 10 para começar.
- **Apoio**: `UDSGA_SupportAura` + um `UDSSupportAbilityData` por habilidade; `DSSupportResolver` porta `resolveAbility` (poder, raio, duração, CDR de apoio com teto 50% + Grande Tático). Zonas (`ADSSupportZone`) pulsam `UDSGE_ZoneBuff` + cura a cada 0,5 s; o Totem provoca a IA.
- **Cartas**: regras puras em `UDSCardLibrary`; stats de atributo aplicados por um único GE infinito (`UDSGE_CardStats`, SetByCaller `Data.Card.<Attr>.Add/Mult` → `(base + Σadd) × Πmult`); stats de gatilho lidos com `UDSStanceComponent::GetCardStat`.
- **Status**: `UDSStatusLibrary::Apply*` (marca, DoTs, lentidão com metade em chefe e teto 70%, atordoamento máx. 1,5 s/chefe vira lentidão 30%, enfraquecer, exposto, provocação). Consultas "mais forte vence" (`GetStrongestStatusMagnitude`). A velocidade do personagem = `MoveSpeed × (1 − lentidão) × sprint/investida` (0 atordoado).
- **Lock-on** (`UDSTargetLockComponent`): clique do scroll/R3 liga/desliga; scroll/analógico direito troca; alvo hostil vivo mais próximo do centro da mira; solta ao morrer/sair do alcance; toda mira usa o alvo travado.
- **IA** (`ADSEnemyAIController`, servidor): alvo = provocação > Totem > herói vivo mais próximo. Comportamentos Melee, Swarm (leque), Charger (investida ×2,4), Blinker (teleporte a 35% de vida), Kiter, Ranged (para a 80% do alcance), Passive (vagueia/foge), Brood (invoca a 60%/30%).

## Assets a criar no editor

1. **Input**: `IMC_Hero` + `IA_Move` (Axis2D), `IA_Look` (Axis2D), `IA_Jump`, `IA_Attack`, `IA_ClassSkill`, `IA_Dodge`, `IA_Sprint`, `IA_Support1..4`, `IA_TargetLock` (botão do meio / R3), `IA_TargetSwitch` (Axis1D: roda do mouse / analógico direito X). Ligar em `AbilityInputBindings` do BP do herói com as tags `Input.*`.
2. **Ability Sets**: `DA_AbilitySet_Common` (Dodge→`Input.Dodge`, Sprint→`Input.Sprint`), `DA_AbilitySet_<Classe>` (BasicAttack→`Input.Attack`, skill→`Input.ClassSkill`).
3. **Classes**: 9 × `DA_Class_<Classe>` (`UDSClassData`) com os números de `classes.ts` convertidos (alcance/movimento/esquiva × 2,5) e 9 × `BP_Hero_<Classe>` (filhos de `ADSHeroCharacter`) → `HeroPawnClasses` do GameMode.
4. **Skills de classe**: BPs de `UDSGA_AreaStrike` para Onda de Luz, Pulso Arcano, Investida Sagrada (cone 80°), Ceifa Sangrenta (cone 120°, `BonusLifesteal`). `UDSGA_Whirlwind` já vem configurado.
5. **Apoio**: 8 × `DA_Support_<Habilidade>` (`UDSSupportAbilityData`, valores em `Data/Import/SupportAbilities.json`; `CooldownTag` = `Cooldown.Support.<Habilidade>`; Estandarte `bZoneDamageBuff` + `ZoneArmorBonus 15`; Santuário `bZoneRegen` + `DamageReduction 0.3`; Totem `bZoneMagnitudeIsDamageReduction` + `bTaunts`) e 8 BPs de `UDSGA_SupportAura` apontando para eles.
6. **Cartas**: `DT_Cards` (DataTable, row struct `DSCardDefinition`) importada de `Data/Import/DT_Cards.json`; `DA_CardRegistry` (`UDSCardRegistry`) com `CardTable = DT_Cards` e `SupportAbilities` (AbilityId → BP de apoio).
7. **Inimigos**: `DA_Enemy_*` (`UDSEnemyData`; trocar o arquétipo preenche os multiplicadores do protótipo), `BP_Enemy` (filho de `ADSEnemyCharacter`, malha/anim), `BP_Projectile_*`. NavMesh no mapa.
8. **Primary Asset Types** no Asset Manager: `DSClass`, `DSEnemy`, `DSCard`, `DSSupportAbility` (se forem carregados por ID).

## Testes

`Gameplay/Tests/DSCardRulesTests.cpp` (portão de pais, sorteio, validação, agregação, resolução de apoio) e `DSDamageMathTests.cpp` (armadura, cadeia de dano, regras de status, bestiário). Valores copiados de `Nakama_Legacy_Reference/test`.
