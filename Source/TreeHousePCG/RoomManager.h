#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RoomTypes.h"
#include "RoomManager.generated.h"

UCLASS()
class TREEHOUSEPCG_API ARoomManager : public AActor
{
    GENERATED_BODY()
    
public: 
    ARoomManager();

protected:
    virtual void BeginPlay() override;

    // Primary storage for the room graph.
    // Using TMap for O(1) lookup as specified in Design Doc.
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Room Data")
    TMap<FHexIndex, FRoomNode> RoomMap;

    // Cache for "Rooms per floor" count query optimization
    UPROPERTY(VisibleAnywhere, Category = "Room Data")
    TMap<int32, int32> FloorRoomCounts;

    UPROPERTY(EditDefaultsOnly, Category = "Room Config")
    float HexSize = 1000.f;

    UPROPERTY(EditDefaultsOnly, Category = "Room Config")
    float FloorHeight = 500.0f;

public: 
    // --- Core API ---

    /** Creates a room at the specified index if valid. Returns true if successful. */
    UFUNCTION(BlueprintCallable, Category = "Room Logic")
    bool CreateRoom(FHexIndex Index, UDataAsset* Definition);

    /** Returns the Room Node struct. Returns a default empty node if not found. */
    UFUNCTION(BlueprintCallable, Category = "Room Logic")
    bool GetRoom(FHexIndex Index, FRoomNode& OutRoom);

    /** Checks if a room exists at the index. */
    UFUNCTION(BlueprintPure, Category = "Room Logic")
    bool HasRoomAt(FHexIndex Index) const;

    // --- Spatial Queries ---

    /** * Returns the neighbor index in a specific direction. 
     * Does NOT check if the room exists, just calculates the math.
     */
    UFUNCTION(BlueprintPure, Category = "Room Math")
    FHexIndex GetNeighborIndex(FHexIndex Origin, ERoomDirection Direction) const;

    /**
     * Checks if there is a valid, active room adjacent to Origin in the given Direction.
     */
    UFUNCTION(BlueprintCallable, Category = "Room Logic")
    bool CheckAdjacency(FHexIndex Origin, ERoomDirection Direction, FRoomNode& OutNeighbor);

    /**
     * "Line cast" search. Moves in 'Direction' from 'Origin' until it hits:
     * 1. An existing room (Target)
     * 2. An empty valid spot (Target)
     * 3. The boundary (Returns false)
     */
    UFUNCTION(BlueprintCallable, Category = "Room Logic")
    bool FindNextSpotInDirection(FHexIndex Origin, ERoomDirection Direction, FHexIndex& OutResult, bool& bHitExistingRoom);

    // --- Helpers ---

    /** Converts Grid Coordinates to World Space for spawning meshes */
    UFUNCTION(BlueprintPure, Category = "Room Math")
    FVector GetWorldLocationFromHex(FHexIndex Index) const;

    /** Validates if the hex is within the "Flower" shape (Center + 1 Ring) */
    bool IsIndexValid(const FHexIndex& Index) const;
};