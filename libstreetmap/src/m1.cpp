#include "m1.h"
#include "FastStructs.h"
#include <unordered_map>
#include <math.h>
#include <sstream>

using namespace std;

// Private function declarations
bool loadOSM(string map_name);
string convertMapToOSMName(string mapName);
void buildStreetsTable();
void buildIntersectionStreetSegments();
void buildStreetStreetSegmentsAndIntersections();
void buildIntersectionskdTree();
void buildSortedFeatures();
void buildStreetSegmentClassifications();
void buildPlacesOfInterestClassifications();
void removeDoubles(vector<unsigned>& vect);
void getAvgLatRad();
t_point convertToWorld(LatLon point);
float computeArea(unsigned featureID);
vector<string> separateString(string str);
bool firstWithinSecond(const vector<string>& first, const vector<string>& second);
double latRad;

vector<const char*> allNames;
void buildAllNamesVector();

//load the map

bool load_map(string map_name) {
    bool load_success = loadStreetsDatabaseBIN(map_name);
    
    if (load_success) {
        buildStreetsTable();
        buildIntersectionStreetSegments();
        buildStreetStreetSegmentsAndIntersections();
        buildIntersectionskdTree();
        buildSortedFeatures();
        buildPlacesOfInterestClassifications();
        getAvgLatRad();
        buildAllNamesVector();
        
        string osmName = convertMapToOSMName(map_name);
        load_success = loadOSM(osmName);
    }

    return load_success;
}

//close the map

void close_map() {
    closeStreetDatabase();
}

// Load the OSM database

bool loadOSM(string map_name) {
    bool load_OSM_success = loadOSMDatabaseBIN(map_name);
    
    if (load_OSM_success)
        buildStreetSegmentClassifications();
    
    return load_OSM_success;
}

// Converts a map street file name to an OSM file name
string convertMapToOSMName(string mapName) {
    string osmName;
    string mapNameSuffix = "streets.bin";
    
    auto positionOfFileSuffix = mapName.find(mapNameSuffix);
    if(positionOfFileSuffix != string::npos)
        mapName.erase(positionOfFileSuffix, mapNameSuffix.size());
    
    osmName = mapName + "osm.bin";
    return osmName;
}

//function to return street id(s) for a street name
//return a 0-length vector if no street with this name exists.

vector<unsigned> find_street_ids_from_name(string street_name) {
    // Initialize the returned vector
    vector<unsigned> streetIDs =
            FastStructs::getInstance().getStreetIDsFromName(street_name);

    return streetIDs;
}
//function to return street names at an intersection (include duplicate street names in returned vector)

vector<string> find_intersection_street_names(unsigned intersection_id) {
    // Initialize the returned vector of street names
    vector<string> streetNames;

    // Create a vector of segmentIDs at the given intersection id
    vector<unsigned> segmentsAtIntersection =
            FastStructs::getInstance().getSegmentsAtIntersection(intersection_id);

    // For each segmentID in the vector,
    // acquire the streetID and use getStreetName function from StreetsDataBaseAPI
    // to get the name of the street segment and push back the name into the
    // streetNames vector
    for (auto segmentIter = segmentsAtIntersection.begin();
            segmentIter < segmentsAtIntersection.end();
            segmentIter++) {
        unsigned segmentID = *segmentIter;
        StreetSegmentInfo segInfo = getStreetSegmentInfo(segmentID);
        unsigned streetID = segInfo.streetID;
        string streetName = getStreetName(streetID);
        streetNames.push_back(streetName);
    }

    return streetNames;
}

// function to return the street segments for a given intersection

vector<unsigned> find_intersection_street_segments(unsigned intersection_id) {
    // Initialize the returned vector with the information from
    // FastStructs data structure
    vector<unsigned> streetSegs = FastStructs::getInstance().getSegmentsAtIntersection(intersection_id);

    return streetSegs;
}

//find all intersections reachable by traveling down one street segment
//from given intersection (hint: you can't travel the wrong way on a 1-way street)
//the returned vector should NOT contain duplicate intersections

vector<unsigned> find_adjacent_intersections(unsigned intersection_id) {
    // Initialize the returned vector with the information from
    // FastStructs data structure
    vector<unsigned> intersectionIDs =
            FastStructs::getInstance().getAdjacentIntersections(intersection_id);

    return intersectionIDs;
}

//can you get from intersection1 to intersection2 using a single street segment (hint: check for 1-way streets too)
//corner case: an intersection is considered to be connected to itself

bool are_directly_connected(unsigned intersection_id1, unsigned intersection_id2) {
    // If the two given intersection ids are equal, return true (connected to itself)
    if (intersection_id1 == intersection_id2)
        return true;

    // Initialize a vector of all intersections reachable from intersection_id1
    vector<unsigned> intersectionsFrom1 = find_adjacent_intersections(intersection_id1);

    // Initialize iterators at begin() and  end() of the vector above
    auto first = intersectionsFrom1.begin();
    auto last = intersectionsFrom1.end();
    // Initialize an iterator using find() to see if intersection_id2 is a
    // reachable intersection from intersection_id1
    auto found = find(first, last, intersection_id2);

    // If found is not at the end(), return true because intersection_id2 is
    // reachable
    if (found != last)
        return true;
    // Otherwise return false
    return false;
}

//for a given street, return all the street segments

vector<unsigned> find_street_street_segments(unsigned street_id) {
    // Initialize the returned vector with the information from
    // FastStructs data structure
    vector<unsigned> connectedSegments =
            FastStructs::getInstance().getSegmentsOnStreet(street_id);

    return connectedSegments;
}

//for a given street, find all the intersections

std::vector<unsigned> find_all_street_intersections(unsigned street_id) {
    // Initialize the returned vector with the information from
    // FastStructs data structure
    vector<unsigned> connectedIntersections =
            FastStructs::getInstance().getIntersectionsOnStreet(street_id);

    return connectedIntersections;
}


// function to return all intersection ids for two intersecting streets
// this function will typically return one intersection id between two street names
// but duplicate street names are allowed, so more than 1 intersection id may exist for 2 street names

std::vector<unsigned> find_intersection_ids_from_street_names
(std::string street_name1, std::string street_name2) {

    // form sorted 2D vectors of:
    //      all street_name1 intersections
    //      all street_name2 intersections
    vector<unsigned> streetIDs1 = find_street_ids_from_name(street_name1);
    vector< vector<unsigned> > intersections1(streetIDs1.size());
    for (unsigned streetNum = 0; streetNum < streetIDs1.size(); streetNum++) {
        unsigned currentStreetID = streetIDs1[streetNum];
        intersections1[streetNum] = find_all_street_intersections(currentStreetID);
    }

    vector<unsigned> streetIDs2 = find_street_ids_from_name(street_name2);
    vector< vector<unsigned> > intersections2(streetIDs2.size());
    for (unsigned streetNum = 0; streetNum < streetIDs2.size(); streetNum++) {
        unsigned currentStreetID = streetIDs2[streetNum];
        intersections2[streetNum] = find_all_street_intersections(currentStreetID);
    }

    // every column of intersection in the 2D vectors is sorted,
    // we can efficiently compare each column
    //
    // for every column of intersections in street1,
    // check every column of intersection in street2 for a match 
    vector<unsigned> intersectionsBetweenStreets;

    for (auto streetIDIter1 = intersections1.begin();
            streetIDIter1 < intersections1.end();
            streetIDIter1++) {
        for (auto streetIDIter2 = intersections2.begin();
                streetIDIter2 < intersections2.end();
                streetIDIter2++) {
            // use iters to search through the 2 current columns
            auto intersectionIter1 = (*streetIDIter1).begin();
            auto intersectionIter2 = (*streetIDIter2).begin();
            while (
                    intersectionIter1 < (*streetIDIter1).end() &&
                    intersectionIter2 < (*streetIDIter2).end()) {

                // if there is a match, add it and increment
                if (*intersectionIter1 == *intersectionIter2) {
                    intersectionsBetweenStreets.push_back(*intersectionIter2);
                    intersectionIter1++;
                } else if (*intersectionIter1 < *intersectionIter2) {
                    intersectionIter1++;
                } else {
                    intersectionIter2++;
                }
            }
        }
    }

    removeDoubles(intersectionsBetweenStreets);

    return intersectionsBetweenStreets;
}

//find the length of a given street segment

double find_street_segment_length(unsigned street_segment_id) {
    // Initialize returned variable
    // Initialize curveID to 0
    // Get the StreetSegmentInfo of the given street segment
    double length = 0.0;
    unsigned curveID = 0;
    StreetSegmentInfo segInfo = getStreetSegmentInfo(street_segment_id);

    // Acquire the starting and ending LatLon of the given street segment
    // Also initialize pointA and pointB, which will be used for computing
    // Street segment length of a curved street segment
    LatLon pointStart, pointEnd, pointA, pointB;
    pointStart = getIntersectionPosition(segInfo.from);
    pointEnd = getIntersectionPosition(segInfo.to);

    // If the street segment is not curved, then compute the length
    // using find_distance_between_two_points();)
    if (segInfo.curvePointCount == 0)
        return find_distance_between_two_points(pointStart, pointEnd);

        // If the street segment only has one curve point, then compute the distance
        // between the start of the segment to the curve point and the distance
        // between the curve point to the end of the segment and return their sum
    else if (segInfo.curvePointCount == 1) {
        curveID = 0;
        pointA = getStreetSegmentCurvePoint(street_segment_id, curveID);

        length += find_distance_between_two_points(pointStart, pointA);
        length += find_distance_between_two_points(pointA, pointEnd);
        return length;
    }
        // If the street segment has more than one curve point, then compute
        // the distance between start of the segment to the first curve point and 
        // the distance between the last curve point and the end of the segment;
        // also compute the distances between all the curve points.
        // Return the sum of all the distances
    else {
        curveID = 0;
        pointA = getStreetSegmentCurvePoint(street_segment_id, curveID);
        length += find_distance_between_two_points(pointStart, pointA);

        for (curveID = 0;
                curveID < segInfo.curvePointCount - 1;
                curveID++) {
            pointA = getStreetSegmentCurvePoint(street_segment_id, curveID);
            pointB = getStreetSegmentCurvePoint(street_segment_id, curveID + 1);
            length += find_distance_between_two_points(pointA, pointB);
        }
        length += find_distance_between_two_points(pointB, pointEnd);
    }
    return length;
}

//find distance between two coordinates

double find_distance_between_two_points(LatLon point1, LatLon point2) {
    // Compute distance using the pythagoras' theorem 
    // on an equirectangular projection
    double distance;
    double lat1 = point1.lat * DEG_TO_RAD;
    double lat2 = point2.lat * DEG_TO_RAD;
    double lon1 = point1.lon * DEG_TO_RAD;
    double lon2 = point2.lon * DEG_TO_RAD;

    double avgLat = (lat1 + lat2) / 2;
    double term1 = ((lon2 - lon1) * cos(avgLat));
    double term2 = (lat2 - lat1);
    distance = EARTH_RADIUS_IN_METERS * sqrt(term1 * term1 + term2 * term2);
    return distance;
}

double find_street_length(unsigned street_id) {
    // Initialize returned variable
    double length = 0.0;
    // Initialize a vector of all street segments in the given street
    vector<unsigned> streetSegments = find_street_street_segments(street_id);

    // Find the lengths of all street segments in the streetSegments vector
    // and return the sum
    for (auto segmentIter = streetSegments.begin();
            segmentIter < streetSegments.end();
            segmentIter++) {
        length += find_street_segment_length(*segmentIter);
    }
    return length;
}

//find the travel time to drive a street segment (time(minutes) = distance(km)/speed_limit(km/hr) * 60

double find_street_segment_travel_time(unsigned street_segment_id) {
    // Acquire the StreetSegmentInfo of the given street segment
    StreetSegmentInfo segInfo = getStreetSegmentInfo(street_segment_id);

    // Find the distance of the given street segment
    double distance = find_street_segment_length(street_segment_id);
    // Acquire the speed limit of the given street segment
    float speedLimit = segInfo.speedLimit;

    // Compute the distance using the speed limit and the calculated distance
    return distance / 1000 / speedLimit * 60;
}

//find the nearest point of interest to a given position

unsigned find_closest_point_of_interest(LatLon my_position) {
    double shortestDistance;
    unsigned closestPOI;

    // Initialize the shortest distance with the ID of
    // the first POI
    LatLon poiPosition = getPointOfInterestPosition(0);
    double distance = find_distance_between_two_points(my_position,
            poiPosition);
    shortestDistance = distance;
    closestPOI = 0;

    // Find the shortest distance by cycling through the rest
    // of the intersections
    unsigned maxPOIs = getNumberOfPointsOfInterest();
    for (unsigned poiID = 1;
            poiID < maxPOIs;
            poiID++) {
        poiPosition = getPointOfInterestPosition(poiID);
        distance = find_distance_between_two_points(my_position,
                poiPosition);
        if (distance < shortestDistance) {
            shortestDistance = distance;
            closestPOI = poiID;
        }
    }

    return closestPOI;
}

//find the nearest intersection (by ID) to a given position
//This function uses an external library, ANN
//which is a library that uses kd tree

unsigned find_closest_intersection(LatLon my_position) {
    // Create an ANNpoint with the location so that we can use the
    // ANN kd tree library function to find the nearest neighbors
    ANNpoint location = annAllocPt(2);
    location[0] = my_position.lon;
    location[1] = my_position.lat;

    // Number of nearest intersections to my_position that will be returned
    unsigned numOfNearest = 5;

    // Array that will hold the numOfNearest intersections to my_position
    ANNidxArray nearIntersectionIDs = new ANNidx[numOfNearest];

    // The closest point is computed based on the standard pythagorean distance
    // formula as opposed to the distance between two points formula for lat lon
    // coordinates. For this reason, numOfNearest intersections are returned
    // and then the true lat lon distance formula is used to compare the 
    // closest of the returned intersectionIDs.
    FastStructs::getInstance().getClosestIntersectionIDs(location,
            nearIntersectionIDs,
            numOfNearest);

    // Initialise the closest intersection to the first one returned.
    unsigned closestID = nearIntersectionIDs[0];
    LatLon intersectionPosition = getIntersectionPosition(closestID);
    double distance = find_distance_between_two_points(my_position,
            intersectionPosition);
    double shortestDistance = distance;

    // Cycle through all the intersection ids in the array and compute the 
    // distance between the intersection and the given position.
    // If the computed distance is smaller than the distance computed previously,
    // the the new intersection id is the closest intersection
    for (unsigned i = 1; i < numOfNearest; i++) {
        unsigned intersectionID = nearIntersectionIDs[i];
        intersectionPosition = getIntersectionPosition(intersectionID);
        distance = find_distance_between_two_points(my_position,
                intersectionPosition);
        if (distance < shortestDistance) {
            shortestDistance = distance;
            closestID = intersectionID;
        }
    }

    annDeallocPt(location);
    delete [] nearIntersectionIDs;

    return closestID;
}

//get the different kinds of roads

std::vector<unsigned>& getLocalRoads() {
    return FastStructs::getInstance().getLocalRoads();
}

std::vector<unsigned>& getServiceRoads() {
    return FastStructs::getInstance().getServiceRoads();
}

std::vector<unsigned>& getCommercialRoads() {
    return FastStructs::getInstance().getCommercialRoads();
}

std::vector<unsigned>& getMotorways() {
    return FastStructs::getInstance().getMotorways();
}

std::vector<unsigned>& getHighways() {
    return FastStructs::getInstance().getHighways();
}

// This function builds the streets hash table in the FastStructs singleton.

void buildStreetsTable() {
    // Unique key hash table.
    // Street names are keys
    // streeID vectors are values
    unordered_map<string, vector<unsigned>> streetNamesToIDs;

    unsigned numberOfStreets = getNumberOfStreets();

    for (unsigned streetID = 0; streetID < numberOfStreets; streetID++) {
        string streetName = getStreetName(streetID);
        auto streetIDsIter = streetNamesToIDs.find(streetName);
        if (streetIDsIter != streetNamesToIDs.end()) { // Entry already exists
            streetIDsIter->second.push_back(streetID); // Add streetID to vector
        } else { // Create the entry for the name and add vector with streetID
            streetNamesToIDs[streetName] = vector<unsigned>(1, streetID);
        }
    }

    FastStructs::getInstance().setStreets(streetNamesToIDs);
}

// This function builds a vector of 

void buildIntersectionStreetSegments() {
    unsigned numberOfIntersections = getNumberOfIntersections();
    unsigned numberOfStreetSegments = getNumberOfStreetSegments();

    // Stores the street segments and adjacent intersections
    // at every intersection in the map.
    // Outer vector is ordered by intersectionID
    // (e.g. intersectionSegments[78] returns a vector
    // of street segments connected to intersectionID 78).
    vector< vector<unsigned> > intersectionStreetSegments(numberOfIntersections);
    vector< vector<unsigned> > adjacentIntersections(numberOfIntersections);

    // Populate the vectors above by cycling through every street
    // segment then adding it and its to/from intersections to
    // the street it is on, on the condition that it is accessible.
    for (unsigned streetSegmentID = 0;
            streetSegmentID < numberOfStreetSegments;
            streetSegmentID++) {
        StreetSegmentInfo segmentInfo = getStreetSegmentInfo(streetSegmentID);
        unsigned firstIntersection = segmentInfo.from;
        unsigned secondIntersection = segmentInfo.to;

        intersectionStreetSegments[firstIntersection].push_back(streetSegmentID);
        intersectionStreetSegments[secondIntersection].push_back(streetSegmentID);

        if (!segmentInfo.oneWay) { // to/from is irrelevant
            adjacentIntersections[firstIntersection].push_back(secondIntersection);
            adjacentIntersections[secondIntersection].push_back(firstIntersection);
        } else { // Only add the "to" to the "from" because of oneWay
            adjacentIntersections[firstIntersection].push_back(secondIntersection);
        }
    }

    for (unsigned intersectionID = 0;
            intersectionID < numberOfIntersections;
            intersectionID++) {
        removeDoubles(adjacentIntersections[intersectionID]);
    }

    FastStructs::getInstance().setIntersectionSegments(intersectionStreetSegments);
    FastStructs::getInstance().setAdjacentIntersections(adjacentIntersections);
}

// Builds a vector of all street segments and intersections on all streets

void buildStreetStreetSegmentsAndIntersections() {
    unsigned numberOfStreets = getNumberOfStreets();
    unsigned numberOfIntersections = getNumberOfIntersections();

    // Stores the street segments and intersections
    // on every street in the map.
    // Outer vector is ordered by streetID
    // (e.g. streetStreetSegments[78] returns a vector
    // of street segments on streetID 78).
    vector< vector<unsigned> > streetStreetSegments(numberOfStreets);
    vector< vector<unsigned> > streetIntersections(numberOfStreets);

    // Similar to above, but inner vector is not a vector
    // of all the intersections on the street. Instead,
    // it is a vector of size numberOfIntersections where
    // streetIntersectionsBool[78][154] would be true if 
    // intersectionID 154 is on streetID 78, and false
    // otherwise.
    // This permits for quick look ups to see if a street already
    // contains particular intersection by checking through
    // random access instead of searching through the whole vector,
    // and will thus help in not added the same intersection twice
    // to the same street.
    vector < vector<bool> > addedIntersections(numberOfStreets,
            vector<bool>(numberOfIntersections, false));

    // Populate the vectors above by cycling through every street
    // segment then adding it and its to/from intersections to
    // the street it is on.
    unsigned maxSegments = getNumberOfStreetSegments();
    for (unsigned segmentID = 0; segmentID < maxSegments; segmentID++) {
        StreetSegmentInfo segmentInfo = getStreetSegmentInfo(segmentID);
        unsigned streetID = segmentInfo.streetID;
        streetStreetSegments[streetID].push_back(segmentID);

        unsigned firstIntersection = segmentInfo.from;
        unsigned secondIntersection = segmentInfo.to;

        // Only add the intersections if they haven't already been added
        if (!addedIntersections[streetID][firstIntersection]) {
            streetIntersections[streetID].push_back(firstIntersection);
            addedIntersections[streetID][firstIntersection] = true;
        }
        if (!addedIntersections[streetID][secondIntersection]) {
            streetIntersections[streetID].push_back(secondIntersection);
            addedIntersections[streetID][secondIntersection] = true;
        }
    }

    for (unsigned streetID = 0; streetID < numberOfStreets; streetID++) {
        sort(streetIntersections[streetID].begin(), streetIntersections[streetID].end());
    }

    FastStructs::getInstance().setStreetStreetSegments(streetStreetSegments);
    FastStructs::getInstance().setStreetIntersections(streetIntersections);
}

// Sorts features into different categories based on their type.
void buildSortedFeatures() {
    unsigned numOfFeatures = getNumberOfFeatures();
    vector<unsigned> lakes;
    vector<unsigned> ponds;
    vector<unsigned> islands;
    vector<unsigned> greens;
    vector<unsigned> sands;
    vector<unsigned> rivers;
    vector<unsigned> buildings;
    vector<unsigned> unknowns;
    
    for (unsigned featureID = 0; featureID < numOfFeatures; featureID++) {
        // For the lake type, sort them into lake and pond based on their area
        if (getFeatureType(featureID) == Lake) {
            float area = computeArea(featureID);
            if (area < 4000000)
                ponds.push_back(featureID);
            else
                lakes.push_back(featureID);
        }
        else if (getFeatureType(featureID) == Island)
            islands.push_back(featureID);
        else if (getFeatureType(featureID) == Park ||
                getFeatureType(featureID) == Greenspace ||
                getFeatureType(featureID) == Golfcourse)
            greens.push_back(featureID);
        else if (getFeatureType(featureID) == Beach ||
                getFeatureType(featureID) == Shoreline)
            sands.push_back(featureID);
        else if (getFeatureType(featureID) == River)
            rivers.push_back(featureID);
        else if (getFeatureType(featureID) == Building)
            buildings.push_back(featureID);
        else
            unknowns.push_back(featureID);
    }
        
    FastStructs::getInstance().setLakes(lakes);
    FastStructs::getInstance().setPonds(ponds);
    FastStructs::getInstance().setIslands(islands);
    FastStructs::getInstance().setGreens(greens);
    FastStructs::getInstance().setSands(sands);
    FastStructs::getInstance().setRivers(rivers);
    FastStructs::getInstance().setBuildings(buildings);
    FastStructs::getInstance().setUnknowns(unknowns);
}

std::vector<unsigned>& getLakes() {
    return FastStructs::getInstance().getLakes();
}

std::vector<unsigned>& getPonds() {
    return FastStructs::getInstance().getPonds();
}

std::vector<unsigned>& getIslands() {
    return FastStructs::getInstance().getIslands();
}

std::vector<unsigned>& getGreens() {
    return FastStructs::getInstance().getGreens();
}

std::vector<unsigned>& getSands() {
    return FastStructs::getInstance().getSands();
}

std::vector<unsigned>& getRivers() {
    return FastStructs::getInstance().getRivers();
}

std::vector<unsigned>& getBuildings() {
    return FastStructs::getInstance().getBuildings();
}

std::vector<unsigned>& getUnknowns() {
    return FastStructs::getInstance().getUnknowns();
}

// Builds an ANNkd tree

void buildIntersectionskdTree() {
    FastStructs::getInstance().setIntersectionskdTree();
}

// Build structure that classifies street segments based on road type

void buildStreetSegmentClassifications() {
    unsigned numOfStreets = getNumberOfStreets();

    // Different classifications
    vector<unsigned> highways(numOfStreets);
    vector<unsigned> motorways(numOfStreets);
    vector<unsigned> serviceRoads(numOfStreets);
    vector<unsigned> commercialRoads(numOfStreets);
    vector<unsigned> localRoads(numOfStreets);
    
    // For every street segment, search through every OSM way for the
    // one that matches the segment wayOSMID
    unsigned long long numberOfWays = getNumberOfWays();
    unsigned maxStreetSegs = getNumberOfStreetSegments();
    // Maps OSM ids to the street segments that are a part of it
    unordered_map<OSMID, vector<unsigned>> osmToSeg;    
    for (unsigned seg = 0; seg < maxStreetSegs; seg++) {
        StreetSegmentInfo segInfo = getStreetSegmentInfo(seg);
        OSMID segOSM = segInfo.wayOSMID;
        auto osmToSegIter = osmToSeg.find(segOSM);
        if (osmToSegIter == osmToSeg.end()) {
            osmToSeg[segOSM] = vector<unsigned>(1, seg);
        } else {
            osmToSegIter->second.push_back(seg);
        }
    }
    
    // Go through every OSM way and add the street segment ids that are a part
    // of it to the proper street classification category, based on the highway tag
    for (unsigned wayIndex = 0; wayIndex < numberOfWays; wayIndex++) {
        const OSMWay *way = getWayByIndex(wayIndex);
        OSMID wayID = way->id();
        auto osmToSegIter = osmToSeg.find(wayID);
        
        if (osmToSegIter != osmToSeg.end()) {
            unsigned tagCount = getTagCount(way);
            unsigned tagIdx = 0;
            pair<string, string> tag;

            do {
                tag = getTagPair(way, tagIdx);
                tagIdx++;
            } while (tag.first != "highway" && tagIdx < tagCount);

            if (tag.second == "motorway")
                highways.insert(highways.end(),
                    osmToSegIter->second.begin(),
                    osmToSegIter->second.end());
            else if (tag.second == "trunk" ||
                    tag.second == "primary" ||
                    tag.second == "motorway_link")
                motorways.insert(motorways.end(),
                    osmToSegIter->second.begin(),
                    osmToSegIter->second.end());
            else if (tag.second == "secondary" ||
                    tag.second == "secondary_link")
                serviceRoads.insert(serviceRoads.end(),
                    osmToSegIter->second.begin(),
                    osmToSegIter->second.end());
            else if (tag.second == "tertiary" ||
                    tag.second == "tertiary_link" ||
                    tag.second == "unclassified")
                commercialRoads.insert(commercialRoads.end(),
                    osmToSegIter->second.begin(),
                    osmToSegIter->second.end());
            else
                localRoads.insert(localRoads.end(),
                    osmToSegIter->second.begin(),
                    osmToSegIter->second.end());
        }
    }

    FastStructs::getInstance().setLocalRoads(localRoads);
    FastStructs::getInstance().setCommercialRoads(commercialRoads);
    FastStructs::getInstance().setServiceRoads(serviceRoads);
    FastStructs::getInstance().setMotorways(motorways);
    FastStructs::getInstance().setHighways(highways);
}

// Finds point of interest IDs with a given name
vector<unsigned> poiIDsFromName(string name) {
    return FastStructs::getInstance().getPOIiDsFromName(name);
}

// Checks if a given poi contains a tag
bool doesContainTag(unsigned poiID, string tag) {
    return FastStructs::getInstance().poiContainsTag(poiID, tag);
}

// Gets the type of a given poi
string typeForPOI(unsigned poiID) {
    return FastStructs::getInstance().poiType(poiID);
}

// Gets the tag for a given alias
string tagForAlias(string alias) {
    return FastStructs::getInstance().tagForAlias(alias);
}

// Gets the average latitude of a city in radians
void getAvgLatRad() {
    unsigned numOfIntersections = getNumberOfIntersections();
    LatLon pos = getIntersectionPosition(0);
    float minLat = pos.lat;
    float maxLat = pos.lat;
    
    // Find the extremes of the lattitude on the map
    for(unsigned i = 1; i < numOfIntersections; i++) {
        pos = getIntersectionPosition(i);
        if(pos.lat < minLat)
            minLat = pos.lat;
        if(pos.lat > maxLat)
            maxLat = pos.lat;
    }
    
    // Get the mean of the extremes
    latRad = (minLat + maxLat) / 2.0 * DEG_TO_RAD;
}

// Converts latitude longitude to world coordinates in meters for the map.
// Uses the equal-lateral projection.
t_point convertToWorld(LatLon point) {
    double lat = point.lat * DEG_TO_RAD;
    double lon = point.lon * DEG_TO_RAD;
    
    double x = lon * cos(latRad) * EARTH_RADIUS_IN_METERS;
    double y = lat * EARTH_RADIUS_IN_METERS;
    
    t_point convertedPoint(x, y);
    return convertedPoint;
}

// Compute the area of a polygon by roughly approximating it as a rectangle.
// The purpose of this function is to sort lakes into large and small sub-categories
// based on their area
float computeArea(unsigned featureID) {
    float area = 0.0;
    float maxX, minX, maxY, minY;
    unsigned numOfPoints = getFeaturePointCount(featureID);
    t_point point = convertToWorld(getFeaturePoint(featureID, 0));
    maxX = point.x;
    minX = point.x;
    maxY = point.y;
    minY = point.y;
    
    // Find the max and min vertices of the feature
    for (unsigned pointCount = 1; pointCount < numOfPoints; pointCount++) {
        t_point point = convertToWorld(getFeaturePoint(featureID, pointCount));
        
        if(point.x < minX)
            minX = point.x;
        if(point.x > maxX)
            maxX = point.x;
        if(point.y < minY)
            minY = point.y;
        if(point.y > maxY)
            maxY = point.y;
    }
    
    // Compute area by approximating polygon as a rectangle for performance
    // purposes
    area = (maxX - minX) * (maxY - minY);
   
    area = abs(area);
    
    
    return area;
}

// A helper function for removing double entries in a vector
void removeDoubles(vector<unsigned>& vect) {
    for (auto i = vect.begin();
            i < vect.end();
            i++) {
        for (auto j = i;
                j < vect.end();
                j++) {
            if (*i == *j && i != j)
                    j = vect.erase(j) - 1;
            }
    }
}

// Find intersection ids by parts of street names
vector<unsigned> searchIntersectionByPartsOfName(string streetName1, 
        string streetName2) {
    // form sorted 2D vectors of:
    //      all street_name1 intersections
    //      all street_name2 intersections
    vector<unsigned> streetIDs1 = searchStreetByPartOfName(streetName1);
    vector< vector<unsigned> > intersections1(streetIDs1.size());
    for (unsigned streetNum = 0; streetNum < streetIDs1.size(); streetNum++) {
        unsigned currentStreetID = streetIDs1[streetNum];
        intersections1[streetNum] = find_all_street_intersections(currentStreetID);
    }

    vector<unsigned> streetIDs2 = searchStreetByPartOfName(streetName2);
    vector< vector<unsigned> > intersections2(streetIDs2.size());
    for (unsigned streetNum = 0; streetNum < streetIDs2.size(); streetNum++) {
        unsigned currentStreetID = streetIDs2[streetNum];
        intersections2[streetNum] = find_all_street_intersections(currentStreetID);
    }

    // every column of intersection in the 2D vectors is sorted,
    // we can efficiently compare each column
    //
    // for every column of intersections in street1,
    // check every column of intersection in street2 for a match 
    vector<unsigned> intersectionsBetweenStreets;

    for (auto streetIDIter1 = intersections1.begin();
            streetIDIter1 < intersections1.end();
            streetIDIter1++) {
        for (auto streetIDIter2 = intersections2.begin();
                streetIDIter2 < intersections2.end();
                streetIDIter2++) {
            // use iters to search through the 2 current columns
            auto intersectionIter1 = (*streetIDIter1).begin();
            auto intersectionIter2 = (*streetIDIter2).begin();
            while (
                    intersectionIter1 < (*streetIDIter1).end() &&
                    intersectionIter2 < (*streetIDIter2).end()) {

                // if there is a match, add it and increment
                if (*intersectionIter1 == *intersectionIter2) {
                    intersectionsBetweenStreets.push_back(*intersectionIter2);
                    intersectionIter1++;
                } else if (*intersectionIter1 < *intersectionIter2) {
                    intersectionIter1++;
                } else {
                    intersectionIter2++;
                }
            }
        }
    }

    removeDoubles(intersectionsBetweenStreets);

    return intersectionsBetweenStreets;
}

// Find street ids by parts of a name
vector<unsigned> searchStreetByPartOfName(string searchField) {
    vector<unsigned> streetIDs; // ids of streets's that contain searchField
    
    // Separate the searchField into its constituent words
    vector<string> searchWords = separateString(searchField);
    string upperSearchField = capitalizeWords(searchField); // Capitalized version
    vector<string> upperSearchWords = separateString(upperSearchField);
    
    // For every street
    unsigned numOfStreets = getNumberOfStreets();
    for(unsigned streetID = 0; streetID < numOfStreets; streetID++) {
        string streetName = getStreetName(streetID);
        vector<string> streetWords = separateString(streetName);    // Separate into
                                                                    // constituent words
        // If either the search field or its capitalized version is within
        // the street name, that street id is a match
        if(firstWithinSecond(searchWords, streetWords) ||
                firstWithinSecond(upperSearchWords, streetWords))
            streetIDs.push_back(streetID);
    }
    
    return streetIDs;
}

// Find point of interest ids by part of a name
vector<unsigned> searchPOIByPartOfName(string searchField) {
    vector<unsigned> poiIDs;    // ids of poi's that contain searchField
    
    // Separate the searchField into its constituent words
    vector<string> searchWords = separateString(searchField);
    string upperSearchField = capitalizeWords(searchField); // Capitalized version
    vector<string> upperSearchWords = separateString(upperSearchField);
    
    // For every poi
    unsigned numOfPOIs = getNumberOfPointsOfInterest();
    for(unsigned poiID = 0; poiID < numOfPOIs; poiID++) {
        string poiName = getPointOfInterestName(poiID);
        vector<string> poiWords = separateString(poiName);  // Separate into
                                                            // constituent words
        // If either the search field or its capitalized version is within
        // the point of interest name, that poi id is a match
        if(firstWithinSecond(searchWords, poiWords) ||
                firstWithinSecond(upperSearchWords, poiWords))
            poiIDs.push_back(poiID);
    }
    
    return poiIDs;
}

// Checks if a vector of words is part of another vector of words
bool firstWithinSecond(const vector<string>& first, const vector<string>& second) {
    
    unsigned sizeOfFirst = first.size();
    unsigned sizeOfSecond = second.size();
    
    // First is larger than second. Cannot be within it.
    if(sizeOfFirst > sizeOfSecond)
        return false;
    
    // The max amount of iterations we have to go through to compare both vectors
    unsigned idxOfComparison = sizeOfSecond - sizeOfFirst;
    
    for(unsigned i = 0; i <= idxOfComparison; i++) {    // For a given segment within
        bool isWithin = true;                           // second,
        for(unsigned j = 0; j < sizeOfFirst; j++) {     // Check if first is within
            if(first[j] != second[i+j]) {               // that segment
                isWithin = false;
                break;
            }
        }
        
        if(isWithin)
            return true;
    }
    
    return false;
}

// Takes in a string and converts the first letter
// in each word to upper case
string capitalizeWords(string inStr) {
    string outStr = ""; // Output string
    bool shouldCapitalize = true;
    
    // Iterate through the input string
    for(unsigned i = 0; i < inStr.size(); i++) {
        char c = inStr[i];
        
        // New word
        if(shouldCapitalize) {
            if(c == ' ')    // Ignore extra whitespace
                continue;
            else if(c >= 'a' && c <= 'z')   // Capitalize the letter
                outStr += toupper(c);
            else            // Letter is already a capital or not alphabetic
                outStr += c;
            shouldCapitalize = false;
        }
        
        // Middle of a word
        else {
            if(c == ' ')
                shouldCapitalize = true;
            outStr += c;
        }
    }
    
    return outStr;
}

// Separates a string into its constituent words
vector<string> separateString(string str) {
    vector<string> strVect; // Output vector of words
    string word = "";
    
    stringstream sstream(str);
    while(!sstream.eof()) {     // Fill in the vector of words one at a time
        sstream >> word;
        strVect.push_back(word);
        word.clear();
    }
    
    return strVect;
}

// Builds the structure that classifies poi's based on type,
// as well as the hash table that maps poi names to ids.
void buildPlacesOfInterestClassifications() {
    unsigned numOfPOIs = getNumberOfPointsOfInterest();
    unordered_map<string, vector<unsigned>> poiNamesToIDs;
    vector< vector<string> > poiTags(numOfPOIs);
    
    for (unsigned poiID = 0; poiID < numOfPOIs; poiID++) {
        string name = getPointOfInterestName(poiID);
        string type = getPointOfInterestType(poiID);
        
        // Build hash table
        auto poiIDsIter = poiNamesToIDs.find(name);
        if (poiIDsIter != poiNamesToIDs.end()) { // Entry already exists
            poiIDsIter->second.push_back(poiID); // Add poiID to vector
        } else { // Create the entry for the name and add vector with poiID
            poiNamesToIDs[name] = vector<unsigned>(1, poiID);
        }
        
        // Build classifications
        if(type == "fuel") {
            poiTags[poiID].push_back("automotive");
            poiTags[poiID].push_back("gas station");
        }
        else if(type == "charging_station") {
            poiTags[poiID].push_back("automotive");
            poiTags[poiID].push_back("charging station");
        }
        else if(type == "Mosque") {
            poiTags[poiID].push_back("religious");
            poiTags[poiID].push_back("mosque");
        }
        else if(type == "place_of_worship") {
            poiTags[poiID].push_back("religious");
            poiTags[poiID].push_back("temple");
        }
        else if(type == "grave_yard") {
            poiTags[poiID].push_back("religious");
            poiTags[poiID].push_back("grave yard");
        }
        else if(type == "parish_hall") {
            poiTags[poiID].push_back("religious");
            poiTags[poiID].push_back("church");
            poiTags[poiID].push_back("parish hall");
        }
        else if(type == "wedding_chapel") {
            poiTags[poiID].push_back("religious");
            poiTags[poiID].push_back("church");
            poiTags[poiID].push_back("wedding chapel");
        }
        else if(type == "school") {
            poiTags[poiID].push_back("education");
            poiTags[poiID].push_back("school");
        }
        else if(type == "university") {
            poiTags[poiID].push_back("education");
            poiTags[poiID].push_back("university");
        }
        else if(type == "college") {
            poiTags[poiID].push_back("education");
            poiTags[poiID].push_back("college");
        }
        else if(type == "kindergarten") {
            poiTags[poiID].push_back("education");
            poiTags[poiID].push_back("kindergarten");
        }
        else if(type == "education") {
            poiTags[poiID].push_back("education");
        }
        else if(type == "preschool") {
            poiTags[poiID].push_back("education");
            poiTags[poiID].push_back("preschool");
        }
        else if(type == "embassy") {
            poiTags[poiID].push_back("public place");
            poiTags[poiID].push_back("embassy");
        }
        else if(type == "courthouse") {
            poiTags[poiID].push_back("public place");
            poiTags[poiID].push_back("courthouse");
        }
        else if(type == "public_building") {
            poiTags[poiID].push_back("public place");
        }
        else if(type == "townhall") {
            poiTags[poiID].push_back("public place");
            poiTags[poiID].push_back("townhall");
        }
        else if(type == "post_office") {
            poiTags[poiID].push_back("public place");
            poiTags[poiID].push_back("post office");
        }
        else if(type == "library") {
            poiTags[poiID].push_back("public place");
            poiTags[poiID].push_back("library");
        }
        else if(type == "government_office") {
            poiTags[poiID].push_back("public place");
            poiTags[poiID].push_back("government office");
        }
        else if(type == "arts_centre") {
            poiTags[poiID].push_back("arts");
            poiTags[poiID].push_back("arts centre");
        }
        else if(type == "cinema") {
            poiTags[poiID].push_back("arts");
            poiTags[poiID].push_back("movie theatre");
        }
        else if(type == "theatre") {
            poiTags[poiID].push_back("arts");
            poiTags[poiID].push_back("movie theatre");
        }
        else if(type == "music_venue") {
            poiTags[poiID].push_back("arts");
            poiTags[poiID].push_back("music venue");
        }
        else if(type == "cafe") {
            poiTags[poiID].push_back("food");
            poiTags[poiID].push_back("cafe");
        }
        else if(type == "restaurant") {
            poiTags[poiID].push_back("food");
            poiTags[poiID].push_back("restaurant");
        }
        else if(type == "fast_food") {
            poiTags[poiID].push_back("food");
            poiTags[poiID].push_back("fast food");
        }
        else if(type == "food") {
            poiTags[poiID].push_back("food");
        }
        else if(type == "food_court") {
            poiTags[poiID].push_back("food");
            poiTags[poiID].push_back("food_court");
        }
        else if(type == "internet_cafe") {
            poiTags[poiID].push_back("food");
            poiTags[poiID].push_back("cafe");
            poiTags[poiID].push_back("internet cafe");
        }
        else if(type == "Coffee Shop") {
            poiTags[poiID].push_back("food");
            poiTags[poiID].push_back("cafe");
        }
        else if(type == "ferry_terminal") {
            poiTags[poiID].push_back("ferry");
        }
        else if(type == "hospital") {
            poiTags[poiID].push_back("hospital");
        }
        else if(type == "pharmacy") {
            poiTags[poiID].push_back("pharmacy");
        }
        else if(type == "pool; fitness centre; ice rinks") {
            poiTags[poiID].push_back("recreation");
            poiTags[poiID].push_back("pool");
            poiTags[poiID].push_back("fitness centre");
            poiTags[poiID].push_back("ice rink");
        }
        else if(type == "gym") {
            poiTags[poiID].push_back("recreation");
            poiTags[poiID].push_back("fitness centre");
            poiTags[poiID].push_back("gym");
        }
        else if(type == "swimming_pool") {
            poiTags[poiID].push_back("recreation");
            poiTags[poiID].push_back("pool");
        }
        else if(type == "fitness_center" || type == "fitness centre") {
            poiTags[poiID].push_back("recreation");
            poiTags[poiID].push_back("fitness centre");
        }
        else if(type == "ice_rink") {
            poiTags[poiID].push_back("recreation");
            poiTags[poiID].push_back("ice rink");
        }
        else if(type == "gymnasium") {
            poiTags[poiID].push_back("recreation");
            poiTags[poiID].push_back("fitness centre");
            poiTags[poiID].push_back("gym");
            poiTags[poiID].push_back("gymnasium");
        }
        else if(type == "bar") {
            poiTags[poiID].push_back("entertainment");
            poiTags[poiID].push_back("bar");
        }
        else if(type == "pub") {
            poiTags[poiID].push_back("entertainment");
            poiTags[poiID].push_back("bar");
            poiTags[poiID].push_back("pub");
        }
        else if(type == "bowling") {
            poiTags[poiID].push_back("entertainment");
            poiTags[poiID].push_back("bowling");
        }
        else if(type == "nightclub") {
            poiTags[poiID].push_back("entertainment");
            poiTags[poiID].push_back("club");
            poiTags[poiID].push_back("nightclub");
        }
        else if(type == "stripclub") {
            poiTags[poiID].push_back("entertainment");
            poiTags[poiID].push_back("club");
            poiTags[poiID].push_back("stripclub");
        }
        else if(type == "billiards") {
            poiTags[poiID].push_back("entertainment");
            poiTags[poiID].push_back("billiards");
        }
        else if(type == "karaoke") {
            poiTags[poiID].push_back("entertainment");
            poiTags[poiID].push_back("karaoke");
        }
        else if(type == "comedy_club") {
            poiTags[poiID].push_back("entertainment");
            poiTags[poiID].push_back("club");
            poiTags[poiID].push_back("comedy club");
        }
        else if(type == "bank") {
            poiTags[poiID].push_back("bank");
        }
        else if(type == "atm") {
            poiTags[poiID].push_back("bank");
            poiTags[poiID].push_back("atm");
        }
        else if(type == "observatory") {
            poiTags[poiID].push_back("tourism");
            poiTags[poiID].push_back("observatory");
        }
        else if(type == "fountain") {
            poiTags[poiID].push_back("tourism");
            poiTags[poiID].push_back("fountain");
        }
        else if(type == "police") {
            poiTags[poiID].push_back("emergency");
            poiTags[poiID].push_back("police station");
        }
        else if(type== "fire_station") {
            poiTags[poiID].push_back("emergency");
            poiTags[poiID].push_back("fire station");
        }
        else if(type == "ambulance") {
            poiTags[poiID].push_back("emergency");
            poiTags[poiID].push_back("ambulance station");
        }
        else if(type == "casino") {
            poiTags[poiID].push_back("casino");
        }
        else if(type == "parking") {
            poiTags[poiID].push_back("parking");
        }
        else if(type == "hotel") {
            poiTags[poiID].push_back("lodging");
            poiTags[poiID].push_back("hotel");
        }
        else if(type == "motel") {
            poiTags[poiID].push_back("lodging");
            poiTags[poiID].push_back("motel");
        }
        else if(type == "shop") {
            poiTags[poiID].push_back("shop");
        }
        else if(type == "marketplace") {
            poiTags[poiID].push_back("shop");
            poiTags[poiID].push_back("marketplace");
        }
        else if(type == "bus_station") {
            poiTags[poiID].push_back("bus station");
        }
    }
    
    FastStructs::getInstance().setPOIs(poiNamesToIDs);
    FastStructs::getInstance().setpoiTags(poiTags);
}

void buildAllNamesVector() {
    unsigned numOfStreets = getNumberOfStreets();
    unsigned numOfPOIs = getNumberOfPointsOfInterest();
    const char* streetName;
    const char* poiName;
    string tempName;
    
    for (unsigned streetID = 0; 
            streetID < numOfStreets; 
            streetID++) {
        tempName = getStreetName(streetID);
        streetName = tempName.c_str();
        allNames.push_back(streetName);
    }
    for (unsigned poiID = 0;
            poiID < numOfPOIs;
            poiID++) {
        tempName = getPointOfInterestName(poiID);
        poiName = tempName.c_str();
        allNames.push_back(poiName);
    }
}

vector <const char*> getAllNames() {
    return allNames;
}