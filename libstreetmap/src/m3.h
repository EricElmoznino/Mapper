#pragma once
#include <vector>
#include <string>
#include <set>
#include "m1.h"

#define NUM_THREADS 8

typedef unordered_map<unsigned, unordered_map<unsigned, double>> costMap;
typedef unordered_map<unsigned, vector<unsigned>> closestMap;

struct IntersectionContent {
    bool isDelivery;
    bool isDepot;
    IntersectionContent() {
        isDelivery = false;
        isDepot = false;
    }
};

// Returns a path (route) between the start intersection and the end 
// intersection, if one exists. If no path exists, this routine returns 
// an empty (size == 0) vector. If more than one path exists, the path 
// with the shortest travel time is returned. The path is returned as a vector 
// of street segment ids; traversing these street segments, in the given order,
// would take one from the start to the end intersection.
std::vector<unsigned> find_path_between_intersections(unsigned 
                   intersect_id_start, unsigned intersect_id_end);


// Returns the time required to travel along the path specified. The path
// is passed in as a vector of street segment ids, and this function can 
// assume the vector either forms a legal path or has size == 0.
// The travel time is the sum of the length/speed-limit of each street 
// segment, plus 15 seconds per turn implied by the path. A turn occurs
// when two consecutive street segments have different street names.
double compute_path_travel_time(const std::vector<unsigned>& path);


// Returns the shortest travel time path (vector of street segments) from 
// the start intersection to a point of interest with the specified name.
// If no such path exists, returns an empty (size == 0) vector.
std::vector<unsigned> find_path_to_point_of_interest (
        unsigned intersect_id_start,
        std::string point_of_interest_name
        );

// Maps every combination of intersections to an associated distance
// between them and stores it in the distanceCostMap.
// Maps every delivery intersection to its closest delivery intersections
// and stores it in closestDeliveryMap.
// Maps every delivery intersection to its closest depot intersections
// and stores it in closestDepotMap.
void courierDijkstra(const vector<unsigned>& range, const vector<IntersectionContent>& intersectionContents,
        costMap& distanceCostMap, closestMap& closestDeliveryMap, closestMap& closestDepotMap,
        unsigned thread, unsigned thingsToFind);

// Builds the cost maps with simple distance point to point
void costsSimple(const vector<unsigned>& range, const set<unsigned>& deliveries, const set<unsigned>& depots,
        costMap& distanceCostMap, closestMap& closestDeliveryMap, closestMap& closestDepotMap);