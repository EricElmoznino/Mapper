/* 
 * File:   FastStructs.cpp
 * Author: elmoznin
 * 
 * Created on January 23, 2016, 10:34 PM
 */

/* This class is used as a container for efficient data structures 
 * that will be used to improve the performance of the m1 API */

#include "FastStructs.h"

// Function to access the singleton instance
FastStructs& FastStructs::getInstance() {
        static FastStructs instance;    // Instantiated on first use
        
        // Return the singleton every time called
        return instance;
}

FastStructs::FastStructs() {
        intersectionskdTree = NULL;
        intersectionLocations = NULL;
        
        // Initialize the tag aliases
        
        // Aliases for tag "automotive"
        setAliases();
}

FastStructs::~FastStructs() {
    if (intersectionskdTree != NULL) delete intersectionskdTree;
    if (intersectionLocations != NULL) annDeallocPts(intersectionLocations);
}

// Searches the hash table for the name and returns the streetIDs if found.
// Otherwise, returns an empty vector.
vector<unsigned> FastStructs::getStreetIDsFromName(string name) {
    auto streetIDsIter = streets.find(name);
    if(streetIDsIter == streets.end())
        return vector<unsigned>(0);
    return streetIDsIter->second;
}

void FastStructs::setStreets(const unordered_map<string, vector<unsigned>>& streetsCopy) {
    streets = streetsCopy;  // Use the operator= of the unordered_map
                            // class to create a copy
}

// Returns street segments connected to intersectionID
vector<unsigned>& FastStructs::getSegmentsAtIntersection(unsigned intersectionID) {
    return intersectionSegments[intersectionID];
}

// Returns adjacent intersections to intersectionID
vector<unsigned>& FastStructs::getAdjacentIntersections(unsigned intersectionID) {
    return adjacentIntersections[intersectionID];
} 

void FastStructs::setIntersectionSegments(const vector< vector<unsigned> >& intersectionSegmentsCopy) {
    intersectionSegments = intersectionSegmentsCopy;
}

void FastStructs::setAdjacentIntersections(const vector< vector<unsigned> >& adjacentIntersectionsCopy) {
    adjacentIntersections = adjacentIntersectionsCopy;
}

// Returns the street segments on streetID
vector<unsigned>& FastStructs::getSegmentsOnStreet(unsigned streetID) {
    return streetStreetSegments[streetID];
}

// Returns the intersections on streetID
vector<unsigned>& FastStructs::getIntersectionsOnStreet(unsigned streetID) {
    return streetIntersections[streetID];
}
    
void FastStructs::setStreetStreetSegments(const vector< vector<unsigned> >& segmentsCopy) {
    streetStreetSegments = segmentsCopy;
}

void FastStructs::setStreetIntersections(const vector< vector<unsigned> >& intersectionsCopy) {
    streetIntersections = intersectionsCopy;
}

// Returns the numOfNearest intersections to point
void FastStructs::getClosestIntersectionIDs(ANNpoint point, 
        ANNidxArray nearIntersectionIDs, unsigned numOfNearest) {
    
    if (intersectionskdTree == NULL)    // No intersections in kd tree
        return ;
    
    // Array of distances. distance[0] will be the distance
    // from point to nearIntersectionIDs[0]]
    ANNdistArray distance = new ANNdist[numOfNearest];
    
    // ANN kd_tree library function that searches the tree for the numOfNearest
    // intersections to point with an error tolerance of zero (last parameter)
    // and stores the result in nearIntersectionIDs as well as the distances to
    // those intersections in distance.
    intersectionskdTree->annkSearch(point, numOfNearest, nearIntersectionIDs, distance, 0);
    
    delete [] distance;
}

// Cycle through all the intersections and create the kd tree for them.
void FastStructs::setIntersectionskdTree() {
    if (intersectionskdTree != NULL) delete intersectionskdTree;
    if (intersectionLocations != NULL) annDeallocPts(intersectionLocations);
    
    unsigned numberOfIntersections = getNumberOfIntersections();
    int dimension = 2;  // 2 dimensional map
    
    // Allocate the ANN library array that will hold the location of all intersections.
    intersectionLocations = annAllocPts((int)numberOfIntersections, dimension);
        for(unsigned intersectionID = 0;
            intersectionID < numberOfIntersections;
            intersectionID++) {
            LatLon intersectionLatLon = getIntersectionPosition(intersectionID);
            double lon = intersectionLatLon.lon;
            double lat = intersectionLatLon.lat;
        
            // Create the location in ANNpoint format.
            ANNpoint intersectionLocation = intersectionLocations[intersectionID];
            intersectionLocation[0] = lon;
            intersectionLocation[1] = lat;
        }
        
        intersectionskdTree = 
            new ANNkd_tree(intersectionLocations, (int) numberOfIntersections, dimension);    
}

/* Getters and setters for street segment classifications */

vector<unsigned>& FastStructs::getLocalRoads() {
    return localRoads;
}
vector<unsigned>& FastStructs::getCommercialRoads() {
    return commercialRoads;
}
vector<unsigned>& FastStructs::getServiceRoads() {
    return serviceRoads;
}
vector<unsigned>& FastStructs::getMotorways() {
    return motorways;
}
vector<unsigned>& FastStructs::getHighways() {
    return highways;
}
void FastStructs::setLocalRoads(const vector<unsigned>& localRoadsCopy) {
    localRoads = localRoadsCopy;
}
void FastStructs::setCommercialRoads(const vector<unsigned>& commercialRoadsCopy) {
    commercialRoads = commercialRoadsCopy;
}
void FastStructs::setServiceRoads(const vector<unsigned>& serviceRoadsCopy) {
    serviceRoads = serviceRoadsCopy;
}
void FastStructs::setMotorways(const vector<unsigned>& motorwaysCopy) {
    motorways = motorwaysCopy;
}
void FastStructs::setHighways(const vector<unsigned>& highwaysCopy) {
    highways = highwaysCopy;
}

/* Getters and setters for feature classifications */

vector<unsigned>& FastStructs::getLakes() {
    return lakes;
}
vector<unsigned>& FastStructs::getPonds() {
    return ponds;
}
vector<unsigned>& FastStructs::getIslands() {
    return islands;
}
vector<unsigned>& FastStructs::getGreens() {
    return greens;
}
vector<unsigned>& FastStructs::getSands() {
    return sands;
}
vector<unsigned>& FastStructs::getRivers() {
    return rivers;
}
vector<unsigned>& FastStructs::getBuildings() {
    return buildings;
}
vector<unsigned>& FastStructs::getUnknowns() {
    return unknowns;
}
void FastStructs::setLakes(const vector<unsigned>& lakesCopy) {
    lakes = lakesCopy;
}
void FastStructs::setPonds(const vector<unsigned>& pondsCopy) {
    ponds = pondsCopy;
}
void FastStructs::setIslands(const vector<unsigned>& islandsCopy) {
    islands = islandsCopy;
}
void FastStructs::setGreens(const vector<unsigned>& greensCopy) {
    greens = greensCopy;
}
void FastStructs::setSands(const vector<unsigned>& sandsCopy) {
    sands = sandsCopy;
}
void FastStructs::setRivers(const vector<unsigned>& riversCopy) {
    rivers = riversCopy;
}
void FastStructs::setBuildings(const vector<unsigned>& buildingsCopy) {
    buildings = buildingsCopy;
}
void FastStructs::setUnknowns(const vector<unsigned>& unknownsCopy) {
    unknowns = unknownsCopy;
}

/* Functions associated to pois    */

vector<unsigned> FastStructs::getPOIiDsFromName(string name) {
    auto poiIDsIter = pois.find(name);
    if(poiIDsIter == pois.end())
        return vector<unsigned>(0);
    return poiIDsIter->second;
}

void FastStructs::setPOIs(const unordered_map<string, vector<unsigned>>& poisCopy) {
    pois = poisCopy;
}

void FastStructs::setpoiTags(const vector< vector<string> >& poiTagsCopy) {
    poiTags = poiTagsCopy;
}

bool FastStructs::poiContainsTag(unsigned poiID, string tag) {
    vector<string> tags = poiTags[poiID];
    for(unsigned i = 0; i < tags.size(); i++)
        if(tag == tags[i])
            return true;
    
    return false;
}

string FastStructs::poiType(unsigned poiID) {
    if(poiTags[poiID].size() >= 1)
        return poiTags[poiID][0];
    else
        return "<unknown>";
}

string FastStructs::tagForAlias(string alias) {
    auto matchingTag = tagAliases.find(alias);
    
    // No matching tag for the alias
    if(matchingTag == tagAliases.end())
        return "<unknown>";
    
    // Return the matching tag
    else
        return matchingTag->second;
}

void FastStructs::setAliases() {
    tagAliases["automotive"] = "automotive";
    tagAliases["Automotive"] = "automotive";
    tagAliases["auto"] = "automotive";
    tagAliases["Auto"] = "automotive";
    tagAliases["car"] = "automotive";
    tagAliases["Car"] = "automotive";
    tagAliases["gas station"] = "gas station";
    tagAliases["Gas Station"] = "gas station";
    tagAliases["gas stations"] = "gas station";
    tagAliases["Gas Stations"] = "gas station";
    tagAliases["fuel"] = "gas station";
    tagAliases["Fuel"] = "gas station";
    tagAliases["gas"] = "gas station";
    tagAliases["Gas"] = "gas station";
    tagAliases["oil"] = "gas station";
    tagAliases["Oil"] = "gas station";
    tagAliases["charging station"] = "charging station";
    tagAliases["charging stations"] = "charging station";
    tagAliases["Charging Station"] = "charging station";
    tagAliases["Charging Stations"] = "charging station";
    tagAliases["charging"] = "charging station";
    tagAliases["Charging"] = "charging station";
    tagAliases["charge"] = "charging station";
    tagAliases["Charge"] = "charging station";
    tagAliases["religious"] = "religious";
    tagAliases["Religious"] = "religious";
    tagAliases["religion"] = "religious";
    tagAliases["Religion"] = "religious";
    tagAliases["mosque"] = "mosque";
    tagAliases["mosques"] = "mosque";
    tagAliases["Mosque"] = "mosque";
    tagAliases["Mosques"] = "mosque";
    tagAliases["temple"] = "temple";
    tagAliases["temples"] = "temple";
    tagAliases["Temple"] = "temple";
    tagAliases["Temples"] = "temple";
    tagAliases["worship"] = "temple";
    tagAliases["Worship"] = "temple";
    tagAliases["place of worship"] = "temple";
    tagAliases["places of worship"] = "temple";
    tagAliases["Place of Worship"] = "temple";
    tagAliases["Places of Worship"] = "temple";
    tagAliases["Place Of Worship"] = "temple";
    tagAliases["Places Of Worship"] = "temple";
    tagAliases["grave yard"] = "grave yard";
    tagAliases["grave yards"] = "grave yard";
    tagAliases["Grave Yard"] = "grave yard";
    tagAliases["Grave Yards"] = "grave yard";
    tagAliases["cemetery"] = "grave yard";
    tagAliases["cemeteries"] = "grave yard";
    tagAliases["Cemetery"] = "grave yard";
    tagAliases["Cemeteries"] = "grave yard";
    tagAliases["church"] = "church";
    tagAliases["churches"] = "church";
    tagAliases["Church"] = "church";
    tagAliases["Churches"] = "church";
    tagAliases["parish hall"] = "parish hall";
    tagAliases["parish halls"] = "parish hall";
    tagAliases["Parish Hall"] = "parish hall";
    tagAliases["Parish Halls"] = "parish hall";
    tagAliases["wedding chapel"] = "wedding chapel";
    tagAliases["wedding chapels"] = "wedding chapel";
    tagAliases["Wedding Chapel"] = "wedding chapel";
    tagAliases["Wedding Chapels"] = "wedding chapel";
    tagAliases["wedding hall"] = "wedding chapel";
    tagAliases["wedding halls"] = "wedding chapel";
    tagAliases["Wedding Hall"] = "wedding chapel";
    tagAliases["Wedding Halls"] = "wedding chapel";
    tagAliases["education"] = "education";
    tagAliases["Education"] = "education";
    tagAliases["school"] = "school";
    tagAliases["schools"] = "school";
    tagAliases["School"] = "school";
    tagAliases["Schools"] = "school";
    tagAliases["university"] = "university";
    tagAliases["universities"] = "university";
    tagAliases["University"] = "university";
    tagAliases["Universities"] = "university";
    tagAliases["higher education"] = "university";
    tagAliases["Higher Education"] = "university";
    tagAliases["college"] = "college";
    tagAliases["colleges"] = "college";
    tagAliases["College"] = "college";
    tagAliases["Colleges"] = "college";
    tagAliases["kindergarten"] = "kindergarten";
    tagAliases["kindergartens"] = "kindergarten";
    tagAliases["Kindergarten"] = "kindergarten";
    tagAliases["Kindergartens"] = "kindergarten";
    tagAliases["preschool"] = "preschool";
    tagAliases["preschools"] = "preschool";
    tagAliases["Preschool"] = "preschool";
    tagAliases["Preschools"] = "preschool";
    tagAliases["pre-school"] = "preschool";
    tagAliases["pre-schools"] = "preschool";
    tagAliases["Pre-School"] = "preschool";
    tagAliases["Pre-Schools"] = "preschool";
    tagAliases["public place"] = "public place";
    tagAliases["public places"] = "public place";
    tagAliases["Public Place"] = "public place";
    tagAliases["Public Places"] = "public place";
    tagAliases["public"] = "public place";
    tagAliases["Public"] = "public place";
    tagAliases["government"] = "public place";
    tagAliases["Government"] = "public place";
    tagAliases["public building"] = "public place";
    tagAliases["public buildings"] = "public place";
    tagAliases["Public Building"] = "public place";
    tagAliases["Public Buildings"] = "public place";
    tagAliases["embassy"] = "embassy";
    tagAliases["embassies"] = "embassy";
    tagAliases["Embassy"] = "embassy";
    tagAliases["Embassies"] = "embassy";
    tagAliases["courthouse"] = "courthouse";
    tagAliases["courthouses"] = "courthouse";
    tagAliases["Courthouse"] = "courthouse";
    tagAliases["Courthouses"] = "courthouse";
    tagAliases["court house"] = "courthouse";
    tagAliases["court houses"] = "courthouse";
    tagAliases["Court House"] = "courthouse";
    tagAliases["Court Houses"] = "courthouse";
    tagAliases["townhall"] = "townhall";
    tagAliases["townhalls"] = "townhall";
    tagAliases["Townhall"] = "townhall";
    tagAliases["Townhalls"] = "townhall";
    tagAliases["town hall"] = "townhall";
    tagAliases["town halls"] = "townhall";
    tagAliases["Town Hall"] = "townhall";
    tagAliases["Town Halls"] = "townhall";
    tagAliases["cityhall"] = "townhall";
    tagAliases["cityhalls"] = "townhall";
    tagAliases["Cityhall"] = "townhall";
    tagAliases["Cityhalls"] = "townhall";
    tagAliases["city hall"] = "townhall";
    tagAliases["city halls"] = "townhall";
    tagAliases["City Hall"] = "townhall";
    tagAliases["City Halls"] = "townhall";
    tagAliases["post office"] = "post office";
    tagAliases["post offices"] = "post office";
    tagAliases["Post Office"] = "post office";
    tagAliases["Post Offices"] = "post office";
    tagAliases["post"] = "post office";
    tagAliases["Post"] = "post office";
    tagAliases["library"] = "library";
    tagAliases["libraries"] = "library";
    tagAliases["Library"] = "library";
    tagAliases["Libraries"] = "library";
    tagAliases["government office"] = "government office";
    tagAliases["government offices"] = "government office";
    tagAliases["Government Office"] = "government office";
    tagAliases["Government Offices"] = "government office";
    tagAliases["art"] = "arts";
    tagAliases["arts"] = "arts";
    tagAliases["Art"] = "arts";
    tagAliases["Arts"] = "arts";
    tagAliases["arts centre"] = "arts centre";
    tagAliases["arts centres"] = "arts centre";
    tagAliases["Arts Centre"] = "arts centre";
    tagAliases["Arts Centres"] = "arts centre";
    tagAliases["arts center"] = "arts centre";
    tagAliases["arts centers"] = "arts centre";
    tagAliases["Arts Center"] = "arts centre";
    tagAliases["Arts Centers"] = "arts centre";
    tagAliases["cinema"] = "movie theatre";
    tagAliases["cinemas"] = "movie theatre";
    tagAliases["Cinema"] = "movie theatre";
    tagAliases["Cinemas"] = "movie theatre";
    tagAliases["theatre"] = "movie theatre";
    tagAliases["theatres"] = "movie theatre";
    tagAliases["Theatre"] = "movie theatre";
    tagAliases["Theatres"] = "movie theatre";
    tagAliases["theater"] = "movie theatre";
    tagAliases["theaters"] = "movie theatre";
    tagAliases["theaters"] = "movie theatre";
    tagAliases["movie theatre"] = "movie theatre";
    tagAliases["movie theatres"] = "movie theatre";
    tagAliases["Movie Theatre"] = "movie theatre";
    tagAliases["Movie Theatres"] = "movie theatre";
    tagAliases["movie theater"] = "movie theatre";
    tagAliases["movie theaters"] = "movie theatre";
    tagAliases["Movie Theater"] = "movie theatre";
    tagAliases["Movie Theaters"] = "movie theatre";
    tagAliases["music venue"] = "music venue";
    tagAliases["music venues"] = "music venue";
    tagAliases["Music Venue"] = "music venue";
    tagAliases["Music Venues"] = "music venue";
    tagAliases["music hall"] = "music venue";
    tagAliases["music halls"] = "music venue";
    tagAliases["Music Hall"] = "music venue";
    tagAliases["Music Halls"] = "music venue";
    tagAliases["music"] = "music venue";
    tagAliases["Music"] = "music venue";
    tagAliases["concert hall"] = "music venue";
    tagAliases["concert halls"] = "music venue";
    tagAliases["Concert Hall"] = "music venue";
    tagAliases["Concert Halls"] = "music venue";
    tagAliases["food"] = "food";
    tagAliases["Food"] = "food";
    tagAliases["cafe"] = "cafe";
    tagAliases["cafes"] = "cafe";
    tagAliases["Cafe"] = "cafe";
    tagAliases["Cafes"] = "cafe";
    tagAliases["coffee"] = "cafe";
    tagAliases["Coffee"] = "cafe";
    tagAliases["coffee shop"] = "cafe";
    tagAliases["coffee shops"] = "cafe";
    tagAliases["Coffee Shop"] = "cafe";
    tagAliases["Coffee Shops"] = "cafe";
    tagAliases["restaurant"] = "restaurant";
    tagAliases["Restaurant"] = "restaurant";
    tagAliases["restaurants"] = "restaurant";
    tagAliases["Restaurants"] = "restaurant";
    tagAliases["fast food"] = "fast food";
    tagAliases["Fast Food"] = "fast food";
    tagAliases["food court"] = "food court";
    tagAliases["Food Court"] = "food court";
    tagAliases["internet cafe"] = "internet cafe";
    tagAliases["internet cafes"] = "internet cafe";
    tagAliases["inter-net cafe"] = "internet cafe";
    tagAliases["inter-net cafes"] = "internet cafe";
    tagAliases["Internet Cafe"] = "internet cafe";
    tagAliases["Internet Cafes"] = "internet cafe";
    tagAliases["Inter-net Cafe"] = "internet cafe";
    tagAliases["Inter-net Cafes"] = "internet cafe";
    tagAliases["internet"] = "internet cafe";
    tagAliases["Internet"] = "internet cafe";
    tagAliases["inter-net"] = "internet cafe";
    tagAliases["Inter-net"] = "internet cafe";
    tagAliases["ferry"] = "ferry";
    tagAliases["ferries"] = "ferry";
    tagAliases["Ferry"] = "ferry";
    tagAliases["Ferries"] = "ferry";
    tagAliases["ferry terminal"] = "ferry";
    tagAliases["ferry terminals"] = "ferry";
    tagAliases["Ferry Terminal"] = "ferry";
    tagAliases["Ferry Terminals"] = "ferry";
    tagAliases["ferry station"] = "ferry";
    tagAliases["ferry stations"] = "ferry";
    tagAliases["Ferry Station"] = "ferry";
    tagAliases["Ferry Stations"] = "ferry";
    tagAliases["hospital"] = "hospital";
    tagAliases["hospitals"] = "hospital";
    tagAliases["Hospital"] = "hospital";
    tagAliases["Hospitals"] = "hospital";
    tagAliases["medical"] = "hospital";
    tagAliases["Medical"] = "hospital";
    tagAliases["medical centre"] = "hospital";
    tagAliases["medical centres"] = "hospital";
    tagAliases["medical center"] = "hospital";
    tagAliases["medical centers"] = "hospital";
    tagAliases["Medical Centre"] = "hospital";
    tagAliases["Medical Centres"] = "hospital";
    tagAliases["Medical Center"] = "hospital";
    tagAliases["Medical Centers"] = "hospital";
    tagAliases["pharmacy"] = "pharmacy";
    tagAliases["pharmacies"] = "pharmacy";
    tagAliases["Pharmacy"] = "pharmacy";
    tagAliases["Pharmacies"] = "pharmacy";
    tagAliases["drug store"] = "pharmacy";
    tagAliases["drug stores"] = "pharmacy";
    tagAliases["Drug Store"] = "pharmacy";
    tagAliases["Drug Stores"] = "pharmacy";
    tagAliases["recreation"] = "recreation";
    tagAliases["Recreation"] = "recreation";
    tagAliases["recreation centre"] = "recreation";
    tagAliases["recreation centres"] = "recreation";
    tagAliases["Recreation Centre"] = "recreation";
    tagAliases["Recreation Centres"] = "recreation";
    tagAliases["recreation center"] = "recreation";
    tagAliases["recreation centers"] = "recreation";
    tagAliases["Recreation Center"] = "recreation";
    tagAliases["Recreation Centers"] = "recreation";
    tagAliases["pool"] = "pool";
    tagAliases["pools"] = "pool";
    tagAliases["Pool"] = "pool";
    tagAliases["Pools"] = "pool";
    tagAliases["swimming pool"] = "pool";
    tagAliases["swimming pools"] = "pool";
    tagAliases["Swimming Pool"] = "pool";
    tagAliases["Swimming Pools"] = "pool";
    tagAliases["swimming"] = "pool";
    tagAliases["Swimming"] = "pool";
    tagAliases["fitness center"] = "fitness centre";
    tagAliases["fitness centers"] = "fitness centre";
    tagAliases["Fitness Center"] = "fitness centre";
    tagAliases["Fitness Centers"] = "fitness centre";
    tagAliases["fitness centre"] = "fitness centre";
    tagAliases["fitness centres"] = "fitness centre";
    tagAliases["Fitness centre"] = "fitness centre";
    tagAliases["Fitness Centres"] = "fitness centre";
    tagAliases["fitness"] = "fitness centre";
    tagAliases["Fitness"] = "fitness centre";
    tagAliases["ice rink"] = "ice rink";
    tagAliases["ice rinks"] = "ice rink";
    tagAliases["Ice Rink"] = "ice rink";
    tagAliases["Ice Rinks"] = "ice rink";
    tagAliases["ice"] = "ice rink";
    tagAliases["Ice"] = "ice rink";
    tagAliases["rink"] = "ice rink";
    tagAliases["Rink"] = "ice rink";
    tagAliases["skating"] = "ice rink";
    tagAliases["Skating"] = "ice rink";
    tagAliases["skating rink"] = "ice rink";
    tagAliases["skating rinks"] = "ice rink";
    tagAliases["Skating Rink"] = "ice rink";
    tagAliases["Skating Rinks"] = "ice rink";
    tagAliases["gym"] = "gym";
    tagAliases["gyms"] = "gym";
    tagAliases["Gym"] = "gym";
    tagAliases["Gyms"] = "gym";
    tagAliases["gymnasium"] = "gymnasium";
    tagAliases["gymnasiums"] = "gymnasium";
    tagAliases["Gymnasium"] = "gymnasium";
    tagAliases["Gymnasiums"] = "gymnasium";
    tagAliases["entertainment"] = "entertainment";
    tagAliases["Entertainment"] = "entertainment";
    tagAliases["fun"] = "entertainment";
    tagAliases["Fun"] = "entertainment";
    tagAliases["bar"] = "bar";
    tagAliases["bars"] = "bar";
    tagAliases["Bar"] = "bar";
    tagAliases["Bars"] = "bar";
    tagAliases["pub"] = "pub";
    tagAliases["pubs"] = "pub";
    tagAliases["Pub"] = "pub";
    tagAliases["Pubs"] = "pub";
    tagAliases["bowling"] = "bowling";
    tagAliases["Bowling"] = "bowling";
    tagAliases["bowling alley"] = "bowling";
    tagAliases["bowling alleys"] = "bowling";
    tagAliases["Bowling Alley"] = "bowling";
    tagAliases["Bowling Alleys"] = "bowling";
    tagAliases["billiards"] = "billiards";
    tagAliases["Billiards"] = "billiards";
    tagAliases["pool bar"] = "billiards";
    tagAliases["Pool Bar"] = "billiards";
    tagAliases["pool bars"] = "billiards";
    tagAliases["Pool Bars"] = "billiards";
    tagAliases["club"] = "club";
    tagAliases["clubs"] = "club";
    tagAliases["Club"] = "club";
    tagAliases["Clubs"] = "club";
    tagAliases["nightclub"] = "nightclub";
    tagAliases["nightclubs"] = "nightclub";
    tagAliases["Nightclub"] = "nightclub";
    tagAliases["Nightclubs"] = "nightclub";
    tagAliases["night club"] = "nightclub";
    tagAliases["night clubs"] = "nightclub";
    tagAliases["Night Club"] = "nightclub";
    tagAliases["Night Clubs"] = "nightclub";
    tagAliases["dance club"] = "nightclub";
    tagAliases["dance clubs"] = "nightclub";
    tagAliases["Dance Club"] = "nightclub";
    tagAliases["Dance Clubs"] = "nightclub";
    tagAliases["stripclub"] = "stripclub";
    tagAliases["stripclubs"] = "stripclub";
    tagAliases["Stripclub"] = "stripclub";
    tagAliases["Stripclubs"] = "stripclub";
    tagAliases["strip club"] = "stripclub";
    tagAliases["strip clubs"] = "stripclub";
    tagAliases["Strip Club"] = "stripclub";
    tagAliases["Strip Clubs"] = "stripclub";
    tagAliases["gentlemen club"] = "stripclub";
    tagAliases["gentlement clubs"] = "stripclub";
    tagAliases["Gentlemen Club"] = "stripclub";
    tagAliases["Gentlemen Clubs"] = "stripclub";
    tagAliases["gentlemen's club"] = "stripclub";
    tagAliases["gentlemen's clubs"] = "stripclub";
    tagAliases["Gentlemen's Club"] = "stripclub";
    tagAliases["Gentlemen's Clubs"] = "stripclub";
    tagAliases["stripper"] = "stripclub";
    tagAliases["strippers"] = "stripclub";
    tagAliases["Stripper"] = "stripclub";
    tagAliases["Strippers"] = "stripclub";
    tagAliases["comedy club"] = "comedy club";
    tagAliases["comedy clubs"] = "comedy club";
    tagAliases["Comedy club"] = "comedy club";
    tagAliases["Comedy Clubs"] = "comedy club";
    tagAliases["comedy"] = "comedy club";
    tagAliases["Comedy"] = "comedy club";
    tagAliases["karaoke"] = "karaoke";
    tagAliases["Karaoke"] = "karaoke";
    tagAliases["karaoke bar"] = "karaoke";
    tagAliases["karaoke bars"] = "karaoke";
    tagAliases["Karaoke Bar"] = "karaoke";
    tagAliases["Karaoke Bars"] = "karaoke";
    tagAliases["japanese people"] = "karaoke";
    tagAliases["bank"] = "bank";
    tagAliases["banks"] = "bank";
    tagAliases["Bank"] = "bank";
    tagAliases["Banks"] = "bank";
    tagAliases["atm"] = "atm";
    tagAliases["atms"] = "atm";
    tagAliases["atm's"] = "atm";
    tagAliases["ATM"] = "atm";
    tagAliases["ATMS"] = "atm";
    tagAliases["ATM'S"] = "atm";
    tagAliases["ATM's"] = "atm";
    tagAliases["tourism"] = "tourism";
    tagAliases["Tourism"] = "tourism";
    tagAliases["tourist"] = "tourism";
    tagAliases["tourists"] = "tourism";
    tagAliases["Tourist"] = "tourism";
    tagAliases["Tourists"] = "tourism";
    tagAliases["tourist attraction"] = "tourism";
    tagAliases["tourist attractions"] = "tourism";
    tagAliases["Tourist Attraction"] = "tourism";
    tagAliases["Tourist Attractions"] = "tourism";
    tagAliases["observatory"] = "observatory";
    tagAliases["observatories"] = "observatory";
    tagAliases["Observatory"] = "observatory";
    tagAliases["Observatories"] = "observatory";
    tagAliases["fountain"] = "fountain";
    tagAliases["fountains"] = "fountain";
    tagAliases["Fountain"] = "fountain";
    tagAliases["Fountains"] = "fountain";
    tagAliases["emergency"] = "emergency";
    tagAliases["emergencies"] = "emergency";
    tagAliases["Emergency"] = "emergency";
    tagAliases["Emergencies"] = "emergency";
    tagAliases["police"] = "police station";
    tagAliases["Police"] = "police station";
    tagAliases["police station"] = "police station";
    tagAliases["police stations"] = "police station";
    tagAliases["Police Station"] = "police station";
    tagAliases["Police Stations"] = "police station";
    tagAliases["cop"] = "police station";
    tagAliases["cops"] = "police station";
    tagAliases["Cop"] = "police station";
    tagAliases["Cops"] = "police station";
    tagAliases["fire station"] = "fire station";
    tagAliases["fire stations"] = "fire station";
    tagAliases["Fire Station"] = "fire station";
    tagAliases["Fire Stations"] = "fire station";
    tagAliases["fire"] = "fire station";
    tagAliases["fires"] = "fire station";
    tagAliases["Fire"] = "fire station";
    tagAliases["Fires"] = "fire station";
    tagAliases["ambulance"] = "ambulance station";
    tagAliases["ambulances"] = "ambulance station";
    tagAliases["ambulance station"] = "ambulance station";
    tagAliases["ambulance stations"] = "ambulance station";
    tagAliases["Ambulance"] = "ambulance station";
    tagAliases["Ambulances"] = "ambulance station";
    tagAliases["Ambulance Station"] = "ambulance station";
    tagAliases["Ambulance Stations"] = "ambulance station";
    tagAliases["casino"] = "casino";
    tagAliases["casinos"] = "casino";
    tagAliases["Casino"] = "casino";
    tagAliases["Casinos"] = "casino";
    tagAliases["gambling"] = "casino";
    tagAliases["Gambling"] = "casino";
    tagAliases["gamble"] = "casino";
    tagAliases["Gamble"] = "casino";
    tagAliases["parking"] = "parking";
    tagAliases["parkings"] = "parking";
    tagAliases["Parking"] = "parking";
    tagAliases["Parkings"] = "parking";
    tagAliases["parking spot"] = "parking";
    tagAliases["parking spots"] = "parking";
    tagAliases["Parking Spot"] = "parking";
    tagAliases["Parking Spots"] = "parking";
    tagAliases["lodging"] = "lodging";
    tagAliases["lodgings"] = "lodging";
    tagAliases["Lodging"] = "lodging";
    tagAliases["Lodgings"] = "lodging";
    tagAliases["hotel"] = "hotel";
    tagAliases["hotels"] = "hotel";
    tagAliases["Hotel"] = "hotel";
    tagAliases["Hotels"] = "hotel";
    tagAliases["motel"] = "motel";
    tagAliases["motels"] = "motel";
    tagAliases["motel"] = "motel";
    tagAliases["Motels"] = "motel";
    tagAliases["shop"] = "shop";
    tagAliases["shops"] = "shop";
    tagAliases["Shop"] = "shop";
    tagAliases["Shops"] = "shop";
    tagAliases["shopping"] = "shop";
    tagAliases["Shopping"] = "shop";
    tagAliases["marketplace"] = "marketplace";
    tagAliases["marketplaces"] = "marketplace";
    tagAliases["Marketplace"] = "marketplace";
    tagAliases["Marketplaces"] = "marketplace";
    tagAliases["market"] = "marketplace";
    tagAliases["markets"] = "marketplace";
    tagAliases["Market"] = "marketplace";
    tagAliases["Markets"] = "marketplace";
    tagAliases["bus"] = "bus station";
    tagAliases["buses"] = "bus station";
    tagAliases["Bus"] = "bus station";
    tagAliases["Buses"] = "bus station";
    tagAliases["bus station"] = "bus station";
    tagAliases["bus stations"] = "bus station";
    tagAliases["Bus Station"] = "bus station";
    tagAliases["Bus Stations"] = "bus station";
    tagAliases["bus stop"] = "bus station";
    tagAliases["bus stops"] = "bus station";
    tagAliases["Bus Stop"] = "bus station";
    tagAliases["Bus Stops"] = "bus station";
    tagAliases["public transit"] = "bus station";
    tagAliases["Public Transit"] = "bus station";
    tagAliases["commute"] = "bus station";
    tagAliases["Commute"] = "bus station";
}