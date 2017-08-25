/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   FastStructs.h
 * Author: elmoznin
 *
 * Created on January 23, 2016, 10:34 PM
 */

/* This class is used as a container for efficient data structures 
 * that will be used to improve the performance of the m1 API */

#ifndef FASTSTRUCTS_H
#define FASTSTRUCTS_H

#include <unordered_map>
#include <vector>

#include "ANN.h"
#include "StreetsDatabaseAPI.h"

using namespace std;

class FastStructs {
public:
    static FastStructs& getInstance();
    
    virtual ~FastStructs();
    
    /* See the private data structures below for explanation on what the
     following getters and setters will return */
    
    vector<unsigned> getStreetIDsFromName(string name);
    void setStreets(const unordered_map<string, vector<unsigned>>& streetsCopy);
    
    vector<unsigned>& getSegmentsAtIntersection(unsigned intersectionID);
    vector<unsigned>& getAdjacentIntersections(unsigned intersectionID);
    
    void setIntersectionSegments(const vector< vector<unsigned> >& intersectionSegmentsCopy);
    void setAdjacentIntersections(const vector< vector<unsigned> >& adjacentIntersectionsCopy);
    
    vector<unsigned>& getSegmentsOnStreet(unsigned streetID);
    vector<unsigned>& getIntersectionsOnStreet(unsigned streetID);
    
    void setStreetStreetSegments(const vector< vector<unsigned> >& segmentsCopy);
    void setStreetIntersections(const vector< vector<unsigned> >& intersectionsCopy);
    
    // Takes in a point (which is equivalent to an array of doubles),
    // the numberOfNearest intersections to the point desired,
    // and returns an array of intersectionIDs by populating
    // the array nearIntersectionIDs.
    void getClosestIntersectionIDs(ANNpoint point, 
        ANNidxArray nearIntersectionIDs, unsigned numOfNearest);
    void setIntersectionskdTree();
    
    // Getters and setters for street segment classifications
    vector<unsigned>& getLocalRoads();
    vector<unsigned>& getCommercialRoads();
    vector<unsigned>& getServiceRoads();
    vector<unsigned>& getMotorways();
    vector<unsigned>& getHighways();
    void setLocalRoads(const vector<unsigned>& localRoadsCopy);
    void setCommercialRoads(const vector<unsigned>& commercialRoadsCopy);
    void setServiceRoads(const vector<unsigned>& serviceRoadsCopy);
    void setMotorways(const vector<unsigned>& motorwaysRoadsCopy);
    void setHighways(const vector<unsigned>& highwaysRoadsCopy);
    
    // Getters and setters for Feature classifications
    vector<unsigned>& getLakes();
    vector<unsigned>& getPonds();
    vector<unsigned>& getIslands();
    vector<unsigned>& getGreens();
    vector<unsigned>& getSands();
    vector<unsigned>& getRivers();
    vector<unsigned>& getBuildings();
    vector<unsigned>& getUnknowns();
    void setLakes(const vector<unsigned>& lakesCopy);
    void setPonds(const vector<unsigned>& pondsCopy);
    void setIslands(const vector<unsigned>& islandsCopy);
    void setGreens(const vector<unsigned>& greensCopy);
    void setSands(const vector<unsigned>& sandsCopy);
    void setRivers(const vector<unsigned>& riversCopy);
    void setBuildings(const vector<unsigned>& buildingsCopy);
    void setUnknowns(const vector<unsigned>& unknownsCopy);
    
    // Getters and setters for the POIs hash table
    vector<unsigned> getPOIiDsFromName(string name);
    void setPOIs(const unordered_map<string, vector<unsigned>>& poisCopy);
    
    // Functions associated to poi types
    void setpoiTags(const vector< vector<string> >& poiTagsCopy);
    bool poiContainsTag(unsigned poiID, string tag);
    string poiType(unsigned poiID);
    string tagForAlias(string alias);
    
private:
    // Can only be created by requesting the instance using
    // getInstance, so constructor is private.
    // Cannot be copy constructed. Cannot be copied.
    FastStructs();                  
    FastStructs(const FastStructs& orig) = delete;
    void operator=(FastStructs const& rhs) = delete;
    
    // Unique key hash table.
    // Street names are keys
    // streeID vectors are values
    unordered_map<string, vector<unsigned>> streets;
    
    // Stores the street segments and adjacent intersections
    // at every intersection in the map.
    // Outer vector is ordered by intersectionID
    // (e.g. intersectionSegments[78] returns a vector
    // of street segments connected to intersectionID 78).
    vector< vector<unsigned> > intersectionSegments;
    vector< vector<unsigned> > adjacentIntersections;
    
    // Stores the street segments and intersections
    // on every street in the map.
    // Outer vector is ordered by streetID
    // (e.g. streetStreetSegments[78] returns a vector
    // of street segments on streetID 78).
    vector< vector<unsigned> > streetStreetSegments;
    vector< vector<unsigned> > streetIntersections;
    
    // Data for the kd tree, which allows for O(logn) lookups 
    // for nearest intersection to a point as opposed to O(n) 
    // for a linear search. Explanations of kd tree can be found at:
    // https://en.wikipedia.org/wiki/K-d_tree
    // The external library used is documented at:
    // https://www.cs.umd.edu/~mount/ANN/
    ANNpointArray intersectionLocations;    // A list of intersection locations
    ANNkd_tree *intersectionskdTree;        // required to build and maintain
                                            // kd tree
    
    // Street segments separated by road class
    vector<unsigned> localRoads;
    vector<unsigned> commercialRoads;
    vector<unsigned> serviceRoads;
    vector<unsigned> motorways;
    vector<unsigned> highways;
    
    // Features separated by FeatureType
    vector<unsigned> lakes; // LAKE 
    vector<unsigned> ponds; // SMALL LAKES 
    vector<unsigned> islands; // ISLAND
    vector<unsigned> greens; // PARK, GREENSPACE, GOLFCOURSE
    vector<unsigned> sands; // BEACH, SHORELINE
    vector<unsigned> rivers; // RIVER
    vector<unsigned> buildings; // BUILDING
    vector<unsigned> unknowns; // UNKNOWN
    
    // Hash table matching point of interest
    // name to its id(s)
    unordered_map<string, vector<unsigned>> pois; 
    
    // Points of interest tags.
    // Indices are poiID's and inner vectors
    // are all tags associated with that poi
    vector< vector<string> > poiTags;
    
    // Aliases for point of interest tags.
    // Used in searching to map various different
    // strings to tags used in the program.
    unordered_map<string, string> tagAliases;
    void setAliases();
};

#endif /* FASTSTRUCTS_H */

