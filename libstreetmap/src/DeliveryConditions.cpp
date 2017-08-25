#include <set>

#include "DeliveryConditions.h"
#include <set>

DeliveryConditions::DeliveryConditions(const vector<DeliveryInfo>& info) {
    unsigned numOfDeliveries = info.size();
    for(unsigned i=0; i < numOfDeliveries; i++){
        unsigned pickUp = info[i].pickUp;
        unsigned dropOff = info[i].dropOff;
        
        // For pickup id, afterIntersection gets dropOff.
        // For dropoff id, beforeintersections gets pickUp
        conditionsTable[pickUp].afterIntersections.push_back(dropOff);
        conditionsTable[dropOff].beforeIntersections.push_back(pickUp);
    }
}

DeliveryConditions::DeliveryConditions() {
    ;
}

bool DeliveryConditions::isBefore(unsigned id1, unsigned id2){
    auto finder = conditionsTable.find(id1);
    
    if (finder == conditionsTable.end())
        return false;
    
    else {
        auto vecBegin = finder->second.beforeIntersections.begin();
        auto vecEnd = finder->second.beforeIntersections.end();
        auto foundIter = find(vecBegin, vecEnd, id2);
        
        if (foundIter == vecEnd)
            return false;
        
        else 
            return true;
    }
}

bool DeliveryConditions::isAfter(unsigned id1, unsigned id2) {
    auto finder = conditionsTable.find(id1);
    
    if (finder == conditionsTable.end())
        return false;
    
    else {
        auto vecBegin = finder->second.afterIntersections.begin();
        auto vecEnd = finder->second.afterIntersections.end();
        auto foundIter = find(vecBegin, vecEnd, id2);
        
        if (foundIter == vecEnd)
            return false;
        
        else 
            return true;
    }
}

bool DeliveryConditions::validityChecker(unsigned id, 
        unordered_map<unsigned, set<unsigned> >& indexInPath) const {
    
    vector<unsigned> before, after;
    InterConditions idConditions;
    
    // check validity of second
    // get set for the intersection we are checking
    //      we know the intersections min position and max position in the path
    idConditions = conditionsTable.find(id)->second;
    before = idConditions.beforeIntersections;
    after = idConditions.afterIntersections;
    
    
    set<unsigned> setId = indexInPath[id];
    unsigned setIdMin = *(setId.begin());
    unsigned setIdMax = *(setId.rbegin());
    // if the min position of an intersection in the "before intersections"
    //      is greater than the max position of the intersection we are checking
    //      then the swap is invalid
    for(auto i = before.begin(); i < before.end(); i++){
        set<unsigned> &currentSet = indexInPath[*i];
        if( *(currentSet.begin()) > setIdMax )
            return false;
    }
    // if the max position of an intersection in the "after intersections"
    //      is less than the min position of the intersection we are checking
    //      then the swap is invalid
    for(auto i = after.begin(); i < after.end(); i++){
        set<unsigned> &currentSet = indexInPath[*i];
        if( *(currentSet.rbegin()) < setIdMin )
            return false;
    }
    
    return true;
}

bool DeliveryConditions::isValidSingleSwap(unsigned id1, unsigned id2, 
        unordered_map<unsigned, set<unsigned> >& indexInPath) const {
    if (!validityChecker(id1, indexInPath) || !validityChecker(id2, indexInPath))
        return false;
    
    return true;
}

bool DeliveryConditions::isValidSectionSwap(vector <unsigned>& ids1, vector <unsigned>& ids2, 
        unsigned sectionSize, unordered_map<unsigned, set<unsigned> >& indexInPath) const {
    
    for (unsigned i = 0; i < sectionSize; i++) {
        if (!validityChecker(ids1[i], indexInPath) || !validityChecker(ids2[i], indexInPath))
            return false;
    }
    
    return true;
}

bool DeliveryConditions::isValidSectionReverse(vector <unsigned>& ids, unsigned sectionSize,
        unordered_map<unsigned, set<unsigned> >& indexInPath) const {
    
    for (unsigned i = 0; i < sectionSize; i++)
        if (!validityChecker(ids[i], indexInPath))
            return false;
    
    return true;
}

unordered_map<unsigned, InterConditions> DeliveryConditions::getConditionsTable() const {
    return conditionsTable;
}