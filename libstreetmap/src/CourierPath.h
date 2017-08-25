#ifndef COURIERPATH_H
#define COURIERPATH_H
#include <set>
#include "TopologicalSorting.h"

class CourierPath {
private:
    Proximities *proximities;           // The distance costs and closest elements
    const DeliveryConditions *conditions; // The before/after conditions for all deliveries
    vector<unsigned> path;  // The list of intersection id's for 
                            // pickup/dropoff and depot locations
    unordered_map<unsigned, set<unsigned> > indexInPath;  // The index in the path vector
                                                          // for a given intersection id
    double distanceCost;
    bool isBeforeInPath(unsigned id1, unsigned id2); // check if id2 is before id1 in the path
    bool isAfterInPath(unsigned id1, unsigned id2); // check if id2 is after id1 in the path
    unsigned smallestPositionInPath(unsigned id);
    unsigned greatestPositionInPath(unsigned id);
    void sectionSwapInPath(unsigned pos1, unsigned pos2, unsigned sectionSize, bool reverse);
    void sectionReverseInPath(unsigned pos, unsigned sectionSize, bool reverse);
    void sectionSwapReverseInPath(unsigned flipSide, unsigned pivot, unsigned sectionSize, bool reverse);
    /*void updateDistanceFromSwap(unsigned firstStart, unsigned firstEnd,
        unsigned secondStart, unsigned secondEnd);*/
    void updateClosestStartDepot();
    void updateClosestEndDepot();
    void updateDistanceCost();
    void updateIndexInPath();
    void topologicalSort(unsigned start);
public:
    CourierPath(const DeliveryConditions *_conditions, Proximities *_proximities, unsigned start);
    bool minorChangeAdjacent(); // swap two adjacent intersections
    bool sectionChange(unsigned sectionSize);
    bool reverseSection(unsigned sectionSize);
    bool swapReverseSection(unsigned sectionSize);
    double getDistanceCost();
    vector<unsigned> getPath();
    
};

#endif /* COURIERPATH_H */
