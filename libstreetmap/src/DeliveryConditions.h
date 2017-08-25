#ifndef DELIVERYCONDITIONS_H
#define DELIVERYCONDITIONS_H

#include <vector>
#include <set>
#include <unordered_map>
#include <algorithm>
#include "Proximities.h"

using namespace std;

struct InterConditions {
    vector<unsigned> beforeIntersections;
    vector<unsigned> afterIntersections;
};

class DeliveryConditions {
protected:
    unordered_map<unsigned, InterConditions> conditionsTable;
public:
    unordered_map<unsigned, InterConditions> getConditionsTable() const;
    DeliveryConditions(const vector<DeliveryInfo>& info);
    DeliveryConditions();
    bool isBefore(unsigned id1, unsigned id2); // check if id2 should be before id1
    bool validityChecker(unsigned id,
        unordered_map<unsigned, set<unsigned> >& indexInPath) const;
    bool isValidSingleSwap(unsigned id1, unsigned id2, 
        unordered_map<unsigned, set<unsigned> >& indexInPath) const;
    bool isValidSectionSwap(vector <unsigned>& ids1, vector <unsigned>& ids2, 
        unsigned sectionSize, unordered_map<unsigned, set<unsigned> >& indexInPath) const;
    bool isValidSectionReverse(vector <unsigned>& ids, unsigned sectionSize,
        unordered_map<unsigned, set<unsigned> >& indexInPath) const;
    bool isAfter(unsigned id1, unsigned id2); // check if id2 should be after id1
};
#endif /* DELIVERYCONDITIONS_H */
