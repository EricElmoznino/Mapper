#include "CourierPath.h"
#include <stdlib.h>
#include <climits>

#define MAX_MINOR_TESTS 30
#define MAX_SECTION_TESTS 30

// Constructs a legal initial courier path with the set of conditions given

CourierPath::CourierPath(const DeliveryConditions *_conditions, Proximities *_proximities, unsigned start) {
    conditions = _conditions;
    proximities = _proximities;

    // Find the closest depot to start.
    // Add it to the path.
    unsigned closestDep = proximities->closestDepot(start, 0);
    path.push_back(closestDep);

    // Insert all the delivery points with priority of closest intersections
    topologicalSort(start);

    // Find the closest depot to end.
    // Add it to the path.
    unsigned pathSize = path.size();
    unsigned last = path[pathSize - 1];
    closestDep = proximities->closestDepot(last, 0);
    path.push_back(closestDep);

    // calculate distance cost
    updateDistanceCost();
}

bool CourierPath::minorChangeAdjacent() {
    unsigned timesTested = 0;
    bool swapValid = false;
    while (!swapValid && timesTested < MAX_MINOR_TESTS) {
        // make sure intersection is not a depot (first or last)
        //      or the last intersection (second last)
        unsigned numOfIntersections = path.size();
        unsigned position1 = rand() % (numOfIntersections - 3) + 1;
        unsigned position2 = position1 + 1;

        unsigned interId1 = path[position1];
        unsigned interId2 = path[position2];

        // Swap the IDs
        sectionSwapInPath(position1, position2, 1, false);

        swapValid = conditions->isValidSingleSwap(interId1, interId2, indexInPath);

        // If it wasn't a valid swap, swap them back. Else, make the actual swap
        if (!swapValid)
            sectionSwapInPath(position1, position2, 1, true);
        else {
            // Make the actual swap
            path[position1] = interId2;
            path[position2] = interId1;

            // Update the depots and distance costs associated with the change,
            // if necessary
            if (position1 == 1) {
                updateClosestStartDepot();
            }
            if (position2 == (path.size() - 2)) {
                updateClosestEndDepot();
            }

            updateDistanceCost();
        }

        timesTested++;
    }

    return swapValid;
}

bool CourierPath::sectionChange(unsigned sectionSize) {
    unsigned numOfIntersections = path.size();
    if ((numOfIntersections - 2) < (2 * sectionSize))
        return false;

    unsigned timesTested = 0;
    bool swapValid = false;
    while (!swapValid && timesTested < MAX_SECTION_TESTS) {
        int position1, position2;
        do {
            position1 = rand() % (numOfIntersections - 1 - sectionSize) + 1;
            position2 = rand() % (numOfIntersections - 1 - sectionSize) + 1;
        } while (abs(position1 - position2) < sectionSize);

        vector<unsigned> interIds1;
        vector<unsigned> interIds2;
        for (unsigned i = 0; i < sectionSize; i++) {
            interIds1.push_back(path[position1 + i]);
            interIds2.push_back(path[position2 + i]);
        }

        sectionSwapInPath(position1, position2, sectionSize, false);

        swapValid = conditions->isValidSectionSwap(interIds1, interIds2, sectionSize, indexInPath);


        if (!swapValid)
            sectionSwapInPath(position1, position2, sectionSize, true);
        else {
            for (unsigned i = 0; i < sectionSize; i++) {
                path[position1 + i] = interIds2[i];
                path[position2 + i] = interIds1[i];
            }
            // Update the depots and distance costs associated with the change,
            // if necessary
            if (position1 == 1) {
                updateClosestStartDepot();
            }
            if ((position2 + sectionSize - 1) == (path.size() - 2)) {
                updateClosestEndDepot();
            }

            updateDistanceCost();
        }

        timesTested++;
    }

    return swapValid;
}

bool CourierPath::reverseSection(unsigned sectionSize) {
    unsigned numOfIntersections = path.size();
    if ((numOfIntersections - 2) < (sectionSize))
        return false;

    unsigned timesTested = 0;
    bool swapValid = false;
    while (!swapValid && timesTested < MAX_SECTION_TESTS) {
        unsigned position = rand() % (numOfIntersections - 1 - sectionSize) + 1;
        vector<unsigned> interIds;
        for (unsigned i = 0; i < sectionSize; i++) {
            unsigned id = path[i + position];
            interIds.push_back(id);
        }

        sectionReverseInPath(position, sectionSize, false);

        swapValid = conditions->isValidSectionReverse(interIds, sectionSize, indexInPath);

        if (!swapValid)
            sectionReverseInPath(position, sectionSize, true);
        else {
            for (unsigned i = 0; i < sectionSize; i++)
                path[i + position] = interIds[sectionSize - 1 - i];
            // Update the depots and distance costs associated with the change,
            // if necessary
            if (position == 1) {
                updateClosestStartDepot();
            }
            if ((position + sectionSize - 1) == (path.size() - 2)) {
                updateClosestEndDepot();
            }

            updateDistanceCost();

        }
        timesTested++;
    }

    return swapValid;
}

bool CourierPath::swapReverseSection(unsigned sectionSize){
    unsigned numOfIntersections = path.size();
    if ((numOfIntersections - 2) < (sectionSize * 2 + 1))
        return false;
    
    unsigned timesTested = 0;
    bool swapValid = false;
    while (!swapValid && timesTested < MAX_SECTION_TESTS) {
        unsigned pivot = rand() % (numOfIntersections - 2 * sectionSize - 2) + sectionSize + 1;
        vector<unsigned> interIds1;
        vector<unsigned> interIds2;
        for (unsigned i = 0; i < sectionSize; i++) {
            unsigned id1 = path[pivot - sectionSize + i];
            unsigned id2 = path[pivot + 1 + i];
            interIds1.push_back(id1);
            interIds2.push_back(id2);
        }
        
        unsigned flipSide = rand() % 4; // 4 different states,
                                                // 0 - reverse left side
                                                // 1 - reverse right side
                                                // 2+ - don't reverse
      
        unsigned pos1 = pivot - sectionSize;
        unsigned pos2 = pivot + 1;
        
        if (flipSide == 0 || flipSide == 1) {
            sectionSwapReverseInPath(flipSide, pivot, sectionSize, false);
        }
        else {
            sectionSwapInPath(pos1, pos2, sectionSize, false);
        }

        swapValid = conditions->isValidSectionSwap(interIds1, interIds2, sectionSize, indexInPath);

        if (!swapValid){
            if (flipSide == 0 || flipSide == 1) {
                sectionSwapReverseInPath(flipSide, pivot, sectionSize, true);
            }
            else{
                sectionSwapInPath(pos1, pos2, sectionSize, true);
            }
        }
        else {
            if (flipSide == 0) {
                for (unsigned i = 0; i < sectionSize; i++){
                    path[pivot + 1 + i] = interIds1[sectionSize - 1 - i];
                    path[pivot - sectionSize + i] = interIds2[i];
                }
            }
            else if (flipSide == 1) {
                for (unsigned i = 0; i < sectionSize; i++){
                    path[pivot + 1 + i] = interIds1[i];
                    path[pivot - 1 - i] = interIds2[i];
                }
            }
            else {
                for (unsigned i = 0; i < sectionSize; i++){
                    path[pivot + 1 + i] = interIds1[i];
                    path[pivot - sectionSize + i] = interIds2[i];
                }
            }
            
            // Update the depots and distance costs associated with the change,
            // if necessary
            if (pivot - sectionSize == 1) {
                updateClosestStartDepot();
            }
            if ((pivot + sectionSize) == (path.size() - 2)) {
                updateClosestEndDepot();
            }

            updateDistanceCost();

        }
        
        timesTested++;
    }

    return swapValid;
    
}

void CourierPath::sectionSwapInPath(unsigned pos1, unsigned pos2, unsigned sectionSize, bool reverse) {
    for (unsigned i = 0; i < sectionSize; i++) {
        unsigned id1 = path[pos1 + i];
        unsigned id2 = path[pos2 + i];

        if (reverse) {
            indexInPath[id1].erase(pos2 + i);
            indexInPath[id2].erase(pos1 + i);
            indexInPath[id1].insert(pos1 + i);
            indexInPath[id2].insert(pos2 + i);
        } else {
            indexInPath[id1].erase(pos1 + i);
            indexInPath[id2].erase(pos2 + i);
            indexInPath[id1].insert(pos2 + i);
            indexInPath[id2].insert(pos1 + i);
        }
    }
}

void CourierPath::sectionReverseInPath(unsigned pos, unsigned sectionSize, bool reverse) {
    for (unsigned i = 0; i < (sectionSize) / 2; i++) {
        unsigned pos1 = i + pos;
        unsigned pos2 = pos + sectionSize - 1 - i;
        unsigned id1 = path[pos1];
        unsigned id2 = path[pos2];

        if (reverse) {
            indexInPath[id1].erase(pos2);
            indexInPath[id2].erase(pos1);
            indexInPath[id1].insert(pos1);
            indexInPath[id2].insert(pos2);
        } else {
            indexInPath[id1].erase(pos1);
            indexInPath[id2].erase(pos2);
            indexInPath[id1].insert(pos2);
            indexInPath[id2].insert(pos1);
        }
    }
}

void CourierPath::sectionSwapReverseInPath(unsigned flipSide, unsigned pivot, unsigned sectionSize, bool reverse){
    for(unsigned i=0; i<sectionSize; i++){
        unsigned pos1 = pivot - sectionSize + i;
        unsigned pos2 = pivot + 1 + i;
        unsigned id1 = path[pos1];
        unsigned id2 = path[pos2];
        
        if(reverse){
            if(flipSide == 0){
                indexInPath[id1].erase(pivot + sectionSize - i);
                indexInPath[id2].erase(pivot - sectionSize + i);
                indexInPath[id1].insert(pos1);
                indexInPath[id2].insert(pos2);
            }
            else {
                indexInPath[id1].erase(pivot + 1 + i);
                indexInPath[id2].erase(pivot - 1 - i);
                indexInPath[id1].insert(pos1);
                indexInPath[id2].insert(pos2);
            }
        }
        else{
            if(flipSide == 0){
                indexInPath[id1].erase(pos1);
                indexInPath[id2].erase(pos2);
                indexInPath[id1].insert(pivot + sectionSize - i);
                indexInPath[id2].insert(pivot - sectionSize + i);
            }
            else {
                indexInPath[id1].erase(pos1);
                indexInPath[id2].erase(pos2);
                indexInPath[id1].insert(pivot + 1 + i);
                indexInPath[id2].insert(pivot - 1 - i);
            }
        }
    }
}

void CourierPath::updateDistanceCost() {
    double distance = 0.0;

    unsigned pathSize = path.size();

    unsigned startDepot = path[0];
    unsigned firstPickUp = path[1];
    unsigned endDepot = path[pathSize - 1];
    unsigned lastDropOff = path[pathSize - 2];

    distance += proximities->costBetween(firstPickUp, startDepot);
    distance += proximities->costBetween(lastDropOff, endDepot);

    for (unsigned i = 1; i < (pathSize - 2); i++) {
        unsigned inter1 = path[i];
        unsigned inter2 = path[i + 1];

        distance += proximities->costBetween(inter1, inter2);
    }

    distanceCost = distance;
}

void CourierPath::updateClosestStartDepot() {
    unsigned firstPickUp = path[1]; // Closest to first delivery intersection
    unsigned newDepot = proximities->closestDepot(firstPickUp, 0);
    path[0] = newDepot;
}

void CourierPath::updateClosestEndDepot() {
    unsigned lastDropOff = path[path.size() - 2]; // Closest to last delivery intersection
    unsigned newDepot = proximities->closestDepot(lastDropOff, 0);
    path[path.size() - 1] = newDepot;
}

// DEPRECATED -- DO NOT USE!!!!!

/*
// Updates the distance cost associated with swapping 2 ranges intersections in the path.
// NOTE: This assumes that the first range comes before the second range in the path.
void CourierPath::updateDistanceFromSwap(unsigned firstStart, unsigned firstEnd,
        unsigned secondStart, unsigned secondEnd) {
    // The node intersections of the swap
    unsigned leftInter = path[firstStart-1];
    unsigned firstStartInter = path[firstStart];
    unsigned firstEndInter = path[firstEnd];
    unsigned middleLeftInter = path[firstEnd+1];
    unsigned middleRightInter = path[secondStart-1];
    unsigned secondStartInter = path[secondStart];
    unsigned secondEndInter = path[secondEnd];
    unsigned rightInter = path[secondEnd+1];
    
    // The distances between the nodes before the swap
    double firstDistanceStart = proximities->costBetween(leftInter, firstStartInter);
    double secondDistanceEnd = proximities->costBetween(secondEndInter, rightInter);
    double firstDistanceEnd = 0.0;
    double secondDistanceStart = 0.0;
    if(middleLeftInter <= middleRightInter) {       // If the swapped ranges are not connected
        firstDistanceEnd = proximities->costBetween(firstEndInter, middleLeftInter);
        secondDistanceStart = proximities->costBetween(middleRightInter, secondStartInter);
    }
    double deltaDistance = firstDistanceStart + firstDistanceEnd + 
        secondDistanceStart + secondDistanceEnd;
    distanceCost -= deltaDistance;
    
    // The distances between the nodes after the swap
    firstDistanceStart = proximities->costBetween(leftInter, secondStartInter);
    secondDistanceEnd = proximities->costBetween(firstEndInter, rightInter);
    firstDistanceEnd = 0.0;
    secondDistanceStart = 0.0;
    if(middleLeftInter <= middleRightInter) {       // If the swapped ranges are not connected
        firstDistanceEnd = proximities->costBetween(secondEndInter, middleLeftInter);
        secondDistanceStart = proximities->costBetween(middleRightInter, firstStartInter);
    }
    deltaDistance = firstDistanceStart + firstDistanceEnd + 
        secondDistanceStart + secondDistanceEnd;
    distanceCost += deltaDistance;
}
 */

double CourierPath::getDistanceCost() {
    return distanceCost;
}

vector<unsigned> CourierPath::getPath() {
    return path;
}

// Walk through the path and reset all the indices associated to the
// pickup and dropoff locations

void CourierPath::updateIndexInPath() {
    indexInPath = unordered_map<unsigned, set<unsigned>>();
    unsigned pathSize = path.size();
    for (unsigned i = 1; i < (pathSize - 1); i++) {
        unsigned interID = path[i];
        indexInPath[interID].insert(i);
    }
}

void CourierPath::topologicalSort(unsigned start) {
    // Data structures for topological search
    TopologicalSorting topSort(*conditions);

    // The index at which an element is being inserted into the path
    unsigned indexOfInsert = 1;

    // Add start to the path
    path.push_back(start);
    indexInPath[start].insert(indexOfInsert);
    indexOfInsert++;
    topSort.added(start);

    // While there are still delivery intersection conditions that haven't been satisfied
    unsigned current = start;
    unsigned next;
    while (!topSort.done()) {
        bool inserted = false;
        unsigned numOfClosest = proximities->numOfClosestTo(current);

        // Try added the closest delivery that only needs to be added once
        for (unsigned i = 0; i < numOfClosest && !inserted; i++) {
            next = proximities->closestDelivery(current, i);
            if (topSort.needsToBeAdded(next)) {
                if (topSort.canBeAddedOnlyOnce(next)) {
                    path.push_back(next);
                    indexInPath[next].insert(indexOfInsert);
                    indexOfInsert++;
                    topSort.added(next);
                    inserted = true;
                    current = next;
                }
            }
        }

        // If we haven't inserted anything yet (nothing only needed to be inserted once)
        if (!inserted) {
            for (unsigned i = 0; i < numOfClosest && !inserted; i++) {
                next = proximities->closestDelivery(current, i);
                if (topSort.needsToBeAdded(next)) {
                    path.push_back(next);
                    indexInPath[next].insert(indexOfInsert);
                    indexOfInsert++;
                    topSort.added(next);
                    inserted = true;
                    current = next;
                }
            }
        }
    }
}
