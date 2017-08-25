/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   TopologicalSorting.h
 * Author: elmoznin
 *
 * Created on March 22, 2016, 4:43 PM
 */

#ifndef TOPOLOGICALSORTING_H
#define TOPOLOGICALSORTING_H

#include "DeliveryConditions.h"

class TopologicalSorting: public DeliveryConditions {
public:
    TopologicalSorting(const DeliveryConditions& conditions);
    bool done();
    void added(unsigned inter);
    bool needsToBeAdded(unsigned inter);
    bool canBeAddedOnlyOnce(unsigned inter);
private:
    unsigned numToInsert;
    unordered_map<unsigned, bool> needToInsert;
    unordered_map<unsigned, bool> oneInsertLeft;
};

#endif /* TOPOLOGICALSORTING_H */

