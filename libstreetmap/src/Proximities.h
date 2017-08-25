/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Proximities.h
 * Author: elmoznin
 *
 * Created on March 21, 2016, 10:53 PM
 */

#ifndef PROXIMITIES_H
#define PROXIMITIES_H

#include <vector>
#include <set>
#include <unordered_map>
#include <thread>
#include "m3.h"

using namespace std;

struct DeliveryInfo {
    //Specifies a delivery order.
    //
    //To satisfy the order the item-to-be-delivered must have been picked-up 
    //from the pickUp intersection before visiting the dropOff intersection.

    DeliveryInfo(unsigned pick_up, unsigned drop_off)
        : pickUp(pick_up), dropOff(drop_off) {}

    //The intersection id where the item-to-be-delivered is picked-up.
    unsigned pickUp;

    //The intersection id where the item-to-be-delivered is dropped-off.
    unsigned dropOff;
};

class Proximities {
public:
    Proximities(const vector<DeliveryInfo>& deliveries, const vector<unsigned>& depots);
    double costBetween(unsigned inter1, unsigned inter2);
    unsigned closestDelivery(unsigned delivery, unsigned idx);
    unsigned closestDepot(unsigned delivery, unsigned idx);
    unsigned numOfClosestTo(unsigned delivery);
    costMap getCostMap() {return distanceCostMap;}
private:
    costMap distanceCostMap;
    closestMap closestDeliveryMap;
    closestMap closestDepotMap;
    unsigned makeIntersectionContents(vector<IntersectionContent>& interContents,
        const vector<DeliveryInfo>& deliveries, const vector<unsigned>& depots);
};

#endif /* PROXIMITIES_H */

