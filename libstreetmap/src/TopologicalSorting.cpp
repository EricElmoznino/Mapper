/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   TopologicalSorting.cpp
 * Author: elmoznin
 * 
 * Created on March 22, 2016, 4:43 PM
 */

#include "TopologicalSorting.h"

TopologicalSorting::TopologicalSorting(const DeliveryConditions& conditions) {
    conditionsTable = conditions.getConditionsTable();
    numToInsert = conditionsTable.size();
    
    for(auto i = conditionsTable.begin();
            i != conditionsTable.end();
            i++) {
        unsigned inter = i->first;
        needToInsert[inter] = true;
        InterConditions& conditionsForInter = i->second;
        if(conditionsForInter.beforeIntersections.size() == 0)
            oneInsertLeft[inter] = true;
        else
            oneInsertLeft[inter] = false;
    }
}

bool TopologicalSorting::done() {
    return (numToInsert == 0);
}

void TopologicalSorting::added(unsigned inter) {
    // Update the needToInsert flag
    if(oneInsertLeft[inter]) {
        needToInsert[inter] = false;
        numToInsert--;
    }
    
    // Remove the inter from all its "after"'s "before"'s and update their
    // oneInsertLeft flag if needed
    InterConditions& conditionsForInter = conditionsTable[inter];
    
    for(unsigned i = 0; i < conditionsForInter.afterIntersections.size(); i++) {
        unsigned interToModify = conditionsForInter.afterIntersections[i];
        InterConditions& conditionsForModified = conditionsTable[interToModify];
        vector<unsigned>& befores = conditionsForModified.beforeIntersections;
        befores.erase(remove(befores.begin(), befores.end(), inter), befores.end());
        
        if(befores.size() == 0)
            oneInsertLeft[interToModify] = true;
    }
}

bool TopologicalSorting::needsToBeAdded(unsigned inter) {
    return needToInsert[inter];
}

bool TopologicalSorting::canBeAddedOnlyOnce(unsigned inter) {
    return oneInsertLeft[inter];
}