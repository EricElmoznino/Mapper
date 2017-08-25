/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Proximities.cpp
 * Author: elmoznin
 * 
 * Created on March 21, 2016, 10:53 PM
 */

#include "Proximities.h"

Proximities::Proximities(const vector<DeliveryInfo>& deliveries, const vector<unsigned>& depots) {
    vector<IntersectionContent> intersectionContents(getNumberOfIntersections());
    unsigned thingsToFind = makeIntersectionContents(intersectionContents, deliveries, depots);
    
    // Create the vectors of intersections for each thread
    vector<vector<unsigned>> ranges(NUM_THREADS);
    unsigned idx = 0;
    unsigned numIntersections = getNumberOfIntersections();
    for(unsigned id = 0; id < numIntersections; id++) {
        if(intersectionContents[id].isDelivery) {
            unsigned destinationRange = idx % NUM_THREADS;
            ranges[destinationRange].push_back(id);
            idx++;
        }
    }
    
    thread threads[NUM_THREADS-1];
    costMap distanceSections[NUM_THREADS];
    closestMap closestDelSections[NUM_THREADS];
    closestMap closestDepSections[NUM_THREADS];
    
    // Execute courier Dijkstra using all threads to compute different parts at a time
    for(unsigned i = 0; i < (NUM_THREADS-1); i++) {
        vector<unsigned>& range = ranges[i+1];
        threads[i] = thread(courierDijkstra, ref(range), ref(intersectionContents),
                ref(distanceSections[i+1]), ref(closestDelSections[i+1]), ref(closestDepSections[i+1]),
                i+1, thingsToFind);
    }
    
    // Run on main thread
    vector<unsigned>& range = ranges[0];
    courierDijkstra(range, intersectionContents, 
            distanceSections[0], closestDelSections[0], closestDepSections[0], 
            0, thingsToFind);
    
    // Join the threads before proceeding
    for(unsigned i = 0; i < (NUM_THREADS-1); i++) {
        threads[i].join();
    }
    
    // Join the maps
    for(unsigned i = 0; i < NUM_THREADS; i++) {
        distanceCostMap.insert(distanceSections[i].begin(), distanceSections[i].end());
        closestDeliveryMap.insert(closestDelSections[i].begin(), closestDelSections[i].end());
        closestDepotMap.insert(closestDepSections[i].begin(), closestDepSections[i].end());
    }
}

unsigned Proximities::makeIntersectionContents(vector<IntersectionContent>& interContents,
        const vector<DeliveryInfo>& deliveries, const vector<unsigned>& depots) {
    unsigned thingsToFind = 0;
    
    unsigned numDeliveries = deliveries.size();
    for(unsigned i = 0; i < numDeliveries; i++) {
        DeliveryInfo del = deliveries[i];
        unsigned id1 = del.pickUp;
        unsigned id2 = del.dropOff;
        
        IntersectionContent& interContent1 = interContents[id1];
        IntersectionContent& interContent2 = interContents[id2];
        
        if(!interContent1.isDelivery && !interContent1.isDepot)
            thingsToFind++;
        if(!interContent2.isDelivery && !interContent2.isDelivery)
            thingsToFind++;
        
        interContent1.isDelivery = true;
        interContent2.isDelivery = true;
    }
    
    unsigned numDepots = depots.size();
    for(unsigned i = 0; i < numDepots; i++) {
        unsigned id = depots[i];
        
        IntersectionContent& interContent = interContents[id];
        
        if(!interContent.isDelivery && !interContent.isDepot)
            thingsToFind++;
        
        interContent.isDepot = true;
    }
    
    thingsToFind--;
    return thingsToFind;
}

double Proximities::costBetween(unsigned inter1, unsigned inter2) {
    return distanceCostMap[inter1][inter2];
}

unsigned Proximities::closestDelivery(unsigned delivery, unsigned idx) {
    return closestDeliveryMap[delivery][idx];
}

unsigned Proximities::closestDepot(unsigned delivery, unsigned idx) {
    return closestDepotMap[delivery][idx];
}

unsigned Proximities::numOfClosestTo(unsigned delivery) {
    return closestDeliveryMap[delivery].size();
}