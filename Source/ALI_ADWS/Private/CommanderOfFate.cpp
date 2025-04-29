#include "CommanderOfFate.h"
#include "Tile.h"
#include "PlayerUnit.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "GameFramework/DamageType.h" // ✅ All you need

ACommanderOfFate::ACommanderOfFate()
{
    PrimaryActorTick.bCanEverTick = true;
}

void ACommanderOfFate::BeginPlay()
{
    Super::BeginPlay();

    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (PC)
    {
        PC->bShowMouseCursor = true;
        PC->bEnableClickEvents = true;
        PC->bEnableMouseOverEvents = true;
        PC->DefaultMouseCursor = EMouseCursor::Hand;
    }

    bTossWinnerIsHuman = FMath::RandBool();

    if (GEngine)
    {
        FString TossResult = bTossWinnerIsHuman ? TEXT("Human starts placing first!") : TEXT("AI starts placing first!");
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TossResult);
    }

    StartPlacementPhase();
}

void ACommanderOfFate::StartPlacementPhase()
{
    bPlacementPhaseActive = true;
    PlacementStep = 0;

    if (!IsCurrentTurnHuman())
    {
        HandlePlacement(FVector2D(-1, -1));
    }
}

void ACommanderOfFate::HandlePlacement(FVector2D TargetTileCoord)
{
    if (!bPlacementPhaseActive) return;

    EUnitType CurrentUnitType = GetUnitTypeForCurrentPlacement();

    if (IsCurrentTurnHuman())
    {
        if (TryPlaceUnitAt(TargetTileCoord, true, CurrentUnitType))
        {
            PlacementStep++;
            if (IsPlacementPhaseComplete()) EndPlacementPhase();
            else if (!IsCurrentTurnHuman()) HandlePlacement(FVector2D(-1, -1));
        }
    }
    else
    {
        ATile* RandomTile = FindRandomEmptyTile();
        if (RandomTile)
        {
            FVector2D AICoord = RandomTile->GetCoordinates();
            if (TryPlaceUnitAt(AICoord, false, CurrentUnitType))
            {
                PlacementStep++;
                if (IsPlacementPhaseComplete()) EndPlacementPhase();
                else if (!IsCurrentTurnHuman()) HandlePlacement(FVector2D(-1, -1));
            }
        }
    }
}

bool ACommanderOfFate::TryPlaceUnitAt(FVector2D Coord, bool bIsHuman, EUnitType UnitType)
{
    if (!GetWorld()) return false;

    TArray<AActor*> Tiles;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATile::StaticClass(), Tiles);

    for (AActor* Actor : Tiles)
    {
        ATile* Tile = Cast<ATile>(Actor);
        if (Tile && Tile->GetCoordinates() == Coord && Tile->GetTileType() == ETileType::Empty)
        {
            FVector Location = Tile->GetActorLocation() + FVector(0, 0, 10.0f);
            TSubclassOf<APlayerUnit> UnitClass = bIsHuman
                ? (UnitType == EUnitType::Sniper ? GreenSniperClass : GreenBrawlerClass)
                : (UnitType == EUnitType::Sniper ? RedSniperClass : RedBrawlerClass);

            if (UnitClass)
            {
                APlayerUnit* NewUnit = GetWorld()->SpawnActor<APlayerUnit>(UnitClass, Location, FRotator::ZeroRotator);
                if (NewUnit)
                {
                    NewUnit->InitializeUnit(bIsHuman ? EUnitTeam::Team_Green : EUnitTeam::Team_Red, UnitType);
                    return true;
                }
            }
        }
    }
    return false;
}

ATile* ACommanderOfFate::FindRandomEmptyTile()
{
    TArray<AActor*> Tiles;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATile::StaticClass(), Tiles);

    TArray<ATile*> EmptyTiles;
    for (AActor* Actor : Tiles)
    {
        ATile* Tile = Cast<ATile>(Actor);
        if (Tile && Tile->GetTileType() == ETileType::Empty)
        {
            EmptyTiles.Add(Tile);
        }
    }

    return EmptyTiles.Num() > 0 ? EmptyTiles[FMath::RandRange(0, EmptyTiles.Num() - 1)] : nullptr;
}

bool ACommanderOfFate::IsCurrentTurnHuman() const
{
    return (PlacementStep % 2 == 0) == bTossWinnerIsHuman;
}

bool ACommanderOfFate::IsPlacementPhaseComplete() const
{
    return PlacementStep >= 4;
}

void ACommanderOfFate::EndPlacementPhase()
{
    bPlacementPhaseActive = false;
    StartGameTurns();
}

void ACommanderOfFate::StartGameTurns()
{
    UE_LOG(LogTemp, Warning, TEXT("All units placed! Starting normal gameplay turns..."));
}

EUnitType ACommanderOfFate::GetUnitTypeForCurrentPlacement() const
{
    return (PlacementStep == 0 || PlacementStep == 1) ? EUnitType::Sniper : EUnitType::Brawler;
}

void ACommanderOfFate::HandleTileClicked(FVector2D TileCoord)
{
    if (bPlacementPhaseActive)
    {
        HandlePlacement(TileCoord);
    }
    else if (SelectedUnit)
    {
        for (ATile* ReachableTile : ReachableTilesForMovement)
        {
            if (ReachableTile && ReachableTile->GetCoordinates() == TileCoord)
            {
                MoveSelectedUnitTo(TileCoord);
                return;
            }
        }
    }
}

void ACommanderOfFate::MoveSelectedUnitTo(FVector2D TileCoord)
{
    if (!SelectedUnit) return;

    TArray<AActor*> Tiles;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATile::StaticClass(), Tiles);

    for (AActor* Actor : Tiles)
    {
        ATile* Tile = Cast<ATile>(Actor);
        if (Tile && Tile->GetCoordinates() == TileCoord && Tile->GetTileType() == ETileType::Empty)
        {
            FVector NewLocation = Tile->GetActorLocation() + FVector(0, 0, 10.0f);
            SelectedUnit->SetActorLocation(NewLocation);

            SelectedUnit->DeselectUnit();
            SelectedUnit = nullptr;
            ClearHighlightedTiles();
            break;
        }
    }
}

void ACommanderOfFate::SetSelectedUnit(APlayerUnit* NewSelectedUnit)
{
    if (SelectedUnit)
    {
        SelectedUnit->DeselectUnit();
        ClearHighlightedTiles();
    }

    SelectedUnit = NewSelectedUnit;

    if (SelectedUnit)
    {
        SelectedUnit->SelectUnit();
        SelectedUnit->FindReachableTiles();
        HighlightMovementTiles();
    }
}

void ACommanderOfFate::HighlightMovementTiles()
{
    if (!SelectedUnit) return;

    ClearHighlightedTiles();

    TArray<AActor*> Tiles;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATile::StaticClass(), Tiles);

    const TArray<FVector2D>& MovementCoords = SelectedUnit->GetReachableTiles();
    for (AActor* Actor : Tiles)
    {
        ATile* Tile = Cast<ATile>(Actor);
        if (Tile && MovementCoords.Contains(Tile->GetCoordinates()))
        {
            Tile->HighlightTile(true, FColor::Green);
            ReachableTilesForMovement.Add(Tile);
        }
    }
}

void ACommanderOfFate::ClearHighlightedTiles()
{
    for (ATile* Tile : ReachableTilesForMovement)
    {
        if (Tile)
        {
            Tile->HighlightTile(false);
        }
    }
    ReachableTilesForMovement.Empty();
}

FVector2D ACommanderOfFate::GetUnitGridCoord(APlayerUnit* Unit) const
{
    return FVector2D(
        FMath::RoundToInt(Unit->GetActorLocation().X / 110.0f + 12.5f),
        FMath::RoundToInt(Unit->GetActorLocation().Y / 110.0f + 12.5f)
    );
}

void ACommanderOfFate::ExecuteAITurn()
{
    TArray<APlayerUnit*> AIUnits;
    TArray<APlayerUnit*> HumanUnits;

    for (TActorIterator<APlayerUnit> It(GetWorld()); It; ++It)
    {
        APlayerUnit* Unit = *It;
        (Unit->GetTeam() == EUnitTeam::Team_Red ? AIUnits : HumanUnits).Add(Unit);
    }

    for (APlayerUnit* AIUnit : AIUnits)
    {
        if (!AIUnit) continue;

        SelectedUnit = AIUnit;
        SelectedUnit->FindReachableTiles();

        APlayerUnit* ClosestTarget = nullptr;
        int32 MinDistance = INT32_MAX;
        FVector2D AIUnitCoord = GetUnitGridCoord(AIUnit);

        for (APlayerUnit* Human : HumanUnits)
        {
            FVector2D TargetCoord = GetUnitGridCoord(Human);
            int32 Dist = FMath::Abs(TargetCoord.X - AIUnitCoord.X) + FMath::Abs(TargetCoord.Y - AIUnitCoord.Y);
            if (Dist < MinDistance)
            {
                MinDistance = Dist;
                ClosestTarget = Human;
            }
        }

        FVector2D BestMove = AIUnitCoord;

        if (ClosestTarget && SelectedUnit->GetReachableTiles().Num() > 0)
        {
            FVector2D TargetCoord = GetUnitGridCoord(ClosestTarget);
            int32 BestDist = MinDistance;

            for (const FVector2D& MoveCoord : SelectedUnit->GetReachableTiles())
            {
                int32 Dist = FMath::Abs(TargetCoord.X - MoveCoord.X) + FMath::Abs(TargetCoord.Y - MoveCoord.Y);
                if (Dist < BestDist)
                {
                    BestDist = Dist;
                    BestMove = MoveCoord;
                }
            }

            if (BestMove != AIUnitCoord)
            {
                MoveSelectedUnitTo(BestMove);
                AIUnitCoord = BestMove;
            }

            int32 AttackDist = FMath::Abs(TargetCoord.X - AIUnitCoord.X) + FMath::Abs(TargetCoord.Y - AIUnitCoord.Y);
            int32 Range = AIUnit->GetUnitType() == EUnitType::Sniper ? 10 : 1;

            if (AttackDist <= Range)
            {
                int32 Damage = (AIUnit->GetUnitType() == EUnitType::Sniper)
                    ? FMath::RandRange(4, 8)
                    : FMath::RandRange(1, 6);

                // ✅ Fully correct
                ClosestTarget->TakeDamage(static_cast<float>(Damage), FDamageEvent(), nullptr, this);

                UE_LOG(LogTemp, Warning, TEXT("AI attacked: %s dealt %d damage"), *UEnum::GetValueAsString(AIUnit->GetUnitType()), Damage);
            }
        }

        SelectedUnit->DeselectUnit();
        SelectedUnit = nullptr;
        ClearHighlightedTiles();
    }
}
