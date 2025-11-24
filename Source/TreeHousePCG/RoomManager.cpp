#include "RoomManager.h"

ARoomManager::ARoomManager()
{
    PrimaryActorTick.bCanEverTick = false;
}

void ARoomManager::BeginPlay()
{
    Super::BeginPlay();
}

bool ARoomManager::CreateRoom(const FHexIndex Index, UDataAsset* Definition)
{
    // Use early returns to reduce nesting
    if (!IsIndexValid(Index)) return false;
    if (RoomMap.Contains(Index)) return false; 

    FRoomNode NewRoom(Index);
    NewRoom.RoomDefinition = Definition;
    NewRoom.bIsActive = true;

    RoomMap.Add(Index, NewRoom);

    // Update the floor count cache
    FloorRoomCounts.FindOrAdd(Index.FloorIndex)++;

    return true;
}

bool ARoomManager::GetRoom(FHexIndex Index, FRoomNode& OutRoom)
{
    if (FRoomNode* FoundRoom = RoomMap.Find(Index))
    {
        OutRoom = *FoundRoom;
        return true;
    }
    return false;
}

bool ARoomManager::HasRoomAt(FHexIndex Index) const
{
    return RoomMap.Contains(Index);
}

FHexIndex ARoomManager::GetNeighborIndex(FHexIndex Origin, ERoomDirection Direction)
{
    FHexIndex Next = Origin;

    switch (Direction)
    {
        case ERoomDirection::North:      Next.R -= 1; break;             // (0, -1)
        case ERoomDirection::NorthEast:  Next.Q += 1; Next.R -= 1; break;// (+1, -1)
        case ERoomDirection::SouthEast:  Next.Q += 1; break;             // (+1, 0)
        case ERoomDirection::South:      Next.R += 1; break;             // (0, +1)
        case ERoomDirection::SouthWest:  Next.Q -= 1; Next.R += 1; break;// (-1, +1)
        case ERoomDirection::NorthWest:  Next.Q -= 1; break;             // (-1, 0)
        case ERoomDirection::Up:         Next.FloorIndex += 1; break;
        case ERoomDirection::Down:       Next.FloorIndex -= 1; break;
    }

    return Next;
}

bool ARoomManager::TryResolveNeighborIndex(FHexIndex Origin, ERoomDirection Direction, FHexIndex& OutIndex) const
{
    FHexIndex Attempt = GetNeighborIndex(Origin, Direction);
    
    if (IsIndexValid(Attempt))
    {
        OutIndex = Attempt;
        return true;
    }
    
    // If we hit a wall going NW, try SW. If NE, try SE.
    ERoomDirection FallbackDir = Direction;
    bool bHasFallback = false;

    if (Direction == ERoomDirection::NorthWest)
    {
        FallbackDir = ERoomDirection::SouthWest;
        bHasFallback = true;
    }
    else if (Direction == ERoomDirection::NorthEast)
    {
        FallbackDir = ERoomDirection::SouthEast;
        bHasFallback = true;
    }

    if (bHasFallback)
    {
        Attempt = GetNeighborIndex(Origin, FallbackDir);
        if (IsIndexValid(Attempt))
        {
            OutIndex = Attempt;
            return true;
        }
    }

    return false;
}

bool ARoomManager::CheckAdjacency(FHexIndex Origin, ERoomDirection Direction, FRoomNode& OutNeighbor)
{
    FHexIndex TargetIndex;
    if (TryResolveNeighborIndex(Origin, Direction, TargetIndex))
    {
        return GetRoom(TargetIndex, OutNeighbor);
    }
    return false;
}

bool ARoomManager::FindNextSpotInDirection(FHexIndex Origin, ERoomDirection Direction, FHexIndex& OutResult, bool& bHitExistingRoom)
{
    if (TryResolveNeighborIndex(Origin, Direction, OutResult))
    {
        bHitExistingRoom = RoomMap.Contains(OutResult);
        return true;
    }
    
    return false;
}

FVector ARoomManager::GetRelativeLocationFromHex(FHexIndex Index) const
{
    // Pointy-topped Hex conversion
    static const float Sqrt3 = FMath::Sqrt(3.0f);
    
    float x = HexSize * Sqrt3 * (Index.Q + Index.R / 2.0f);
    float y = HexSize * 1.5f * Index.R;
    float z = Index.FloorIndex * FloorHeight;

    return FVector(x, y, z);
}

bool ARoomManager::IsIndexValid(const FHexIndex& Index) const
{
    // Axial distance formula: (abs(q) + abs(q+r) + abs(r)) / 2
    int32 Distance = (FMath::Abs(Index.Q) + FMath::Abs(Index.Q + Index.R) + FMath::Abs(Index.R)) / 2;
    return Distance <= MaxHexRadius;
}

bool ARoomManager::MoveSelector(const ERoomInputDirection InputDir)
{
    FHexIndex TargetIndex;
    bool bHitExistingRoom;
    
    // Directly pass result of ToRoomDirection into the finder
    if (FindNextSpotInDirection(CurrentIndex, ToRoomDirection(InputDir), TargetIndex, bHitExistingRoom))
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, 
                FString::Printf(TEXT("New Hex: %s"), *TargetIndex.ToString()));
        }
        
        CurrentIndex = TargetIndex;
        return true;
    }
    
    return false;
}

ERoomDirection ARoomManager::ToRoomDirection(const ERoomInputDirection InputDir) const
{
    switch (InputDir)
    {
        case ERoomInputDirection::South: return ERoomDirection::South;
        case ERoomInputDirection::Up:    return ERoomDirection::Up;
        case ERoomInputDirection::Down:  return ERoomDirection::Down;
        
        // Logic for mapping 4-way input to 6-way hexes
        case ERoomInputDirection::West:
            return (CurrentIndex.Q == 0) ? ERoomDirection::NorthWest : ERoomDirection::SouthWest;
            
        case ERoomInputDirection::East:
            return (CurrentIndex.Q == 0) ? ERoomDirection::NorthEast : ERoomDirection::SouthEast;

        case ERoomInputDirection::North:
        default:
            return ERoomDirection::North;
    }
}