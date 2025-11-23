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

FHexIndex ARoomManager::GetNeighborIndex(FHexIndex Origin, ERoomDirection Direction) const
{
    FHexIndex Next = Origin;

    switch (Direction)
    {
    case ERoomDirection::North:      // (0, -1)
        Next.R -= 1;
        break;
    case ERoomDirection::NorthEast:  // (+1, -1)
        Next.Q += 1;
        Next.R -= 1;
        break;
    case ERoomDirection::SouthEast:  // (+1, 0)
        Next.Q += 1;
        break;
    case ERoomDirection::South:      // (0, +1)
        Next.R += 1;
        break;
    case ERoomDirection::SouthWest:  // (-1, +1)
        Next.Q -= 1;
        Next.R += 1;
        break;
    case ERoomDirection::NorthWest:  // (-1, 0)
        Next.Q -= 1;
        break;
    case ERoomDirection::Up:
        Next.FloorIndex += 1;
        break;
    case ERoomDirection::Down:
        Next.FloorIndex -= 1;
        break;
    }

    return Next;
}

bool ARoomManager::CheckAdjacency(FHexIndex Origin, ERoomDirection Direction, FRoomNode& OutNeighbor)
{
    FHexIndex TargetIndex = GetNeighborIndex(Origin, Direction);

    if (!IsIndexValid(TargetIndex))
    {
        return false;
    }

    return GetRoom(TargetIndex, OutNeighbor);
}

bool ARoomManager::FindNextSpotInDirection(FHexIndex Origin, ERoomDirection Direction, FHexIndex& OutResult, bool& bHitExistingRoom)
{
    // Simple implementation: Look at immediate neighbor. 
    // Since the map is small (radius 1), "Search" is just checking the next tile.
    // If radius expands, convert this to a while loop.

    FHexIndex TargetIndex = GetNeighborIndex(Origin, Direction);

    if (!IsIndexValid(TargetIndex))
    {
        return false; // Hit the edge of the valid map area
    }

    OutResult = TargetIndex;
    bHitExistingRoom = RoomMap.Contains(TargetIndex);
    
    return true;
}

FVector ARoomManager::GetWorldLocationFromHex(FHexIndex Index) const
{
    // Pointy-topped Hex conversion
    // x = size * sqrt(3) * (q + r/2)
    // y = size * 3/2 * r
    // Note: TDD formula was slightly different, using standard Pointy Top here:
    
    float x = HexSize * FMath::Sqrt(3.0f) * (Index.Q + Index.R / 2.0f);
    float y = HexSize * (3.0f / 2.0f) * Index.R;
    float z = Index.FloorIndex * FloorHeight;

    return FVector(x, y, z);
}

bool ARoomManager::IsIndexValid(const FHexIndex& Index) const
{
    // Axial distance formula: (abs(q) + abs(q+r) + abs(r)) / 2
    int32 Distance = (FMath::Abs(Index.Q) + FMath::Abs(Index.Q + Index.R) + FMath::Abs(Index.R)) / 2;
    return Distance <= MaxHexRadius;
}