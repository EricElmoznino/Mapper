#include "m3.h"
#include <queue>

// Constant upper speed limit for heuristic function
const float upperSpeedLimit = 120.0;    // km/h

// Constant turn time penalty for path finding
const double turnTime = 0.25;           // minutes

// Structure for path finding facilitation
struct pathNode {
    double distance;
    bool visited;
    unsigned previous;
    
    pathNode(){
        distance = FLT_MAX; // Initialize with distance infinity from source
        visited = false;    // Has not yet been visited by algorithm
        previous = UINT_MAX;    // Initialize previous street segment id with undefined value
    }
};

struct QueueNode {
    unsigned id;
    double distance;
    QueueNode(unsigned _id, double _distance) {
        id = _id;
        distance = _distance;
    }
};

// Vector of path nodes with size of total number of intersections
vector<pathNode> pathNodes(getNumberOfIntersections());

// Comparator function for the priority_queue
struct compareIntersectionDistances {
    bool operator()(const QueueNode& first, const QueueNode& second) {
        return first.distance > second.distance;
    }
};

// Helper function declarations
void resetPathNodes();
vector<unsigned> constructPath(unsigned end);
double getDistanceCost(unsigned currentNode, unsigned nextNode, 
        unsigned segID, unsigned endNode, bool aStar);
double heuristicDistanceCost(unsigned currentNode, unsigned nextNode, unsigned endNode);
double turnPenalty(unsigned currentNode, unsigned segID);
void printTravelTime(unsigned destination);
void directions(const vector<unsigned>& path, unsigned startIntersection);
void printStartDirection(unsigned startIntersection, unsigned streetSeg);
void printUnambiguousAction(string streetName, double angle);
void printAmbiguousAction(string streetName, int turnNumber);
bool noChangeInDirection(unsigned seg1, unsigned seg2);
double angleOfStreetSeg(unsigned startIntersection, unsigned streetSeg);
double angleBetweenStreetSegs(unsigned seg1, unsigned seg2);
int ambiguityForAction(unsigned seg1, unsigned seg2, double angle);
unsigned sharedIntersectionBetweenSegments(unsigned seg1, unsigned seg2);

// Returns a path (route) between the start intersection and the end 
// intersection, if one exists. If no path exists, this routine returns 
// an empty (size == 0) vector. If more than one path exists, the path 
// with the shortest travel time is returned. The path is returned as a vector 
// of street segment ids; traversing these street segments, in the given order,
// would take one from the start to the end intersection.
std::vector<unsigned> find_path_between_intersections(unsigned 
                   intersect_id_start, unsigned intersect_id_end) {
    vector<unsigned> pathBetweenIntersections;
    
    // Reset the path
    resetPathNodes();
    
    // Set the distance associated to the start to the ideal point-to-point
    // travel time from the start intersection to the end intersection
    LatLon startLatLon = getIntersectionPosition(intersect_id_start);
    LatLon endLatLon = getIntersectionPosition(intersect_id_end);
    double startToEndDistance = 
        find_distance_between_two_points(startLatLon, endLatLon);
    double startToEndTravelTime = 
        startToEndDistance / 1000.0 / upperSpeedLimit * 60.0;
    pathNodes[intersect_id_start].distance = startToEndTravelTime;
    
    // Initialize the priority queue for frontier unvisited intersections
    priority_queue<QueueNode, vector<QueueNode>, compareIntersectionDistances> frontier;
    double queueNodeDistance = pathNodes[intersect_id_start].distance;
    QueueNode queueNode(intersect_id_start, queueNodeDistance);
    frontier.push(queueNode);
    
    // While there is still a possible path to the destination
    while(!frontier.empty()) {
        
        // Evaluate the unvisited node with the lowest distance cost
        unsigned currentNode = frontier.top().id;
        frontier.pop();
        if(pathNodes[currentNode].visited)
            continue;
        
        pathNodes[currentNode].visited = true;
        
        // Check if we have reached the destination
        if(currentNode == intersect_id_end)
            break;
        
        // All the connected street segments to the current intersection
        vector<unsigned> connected = find_intersection_street_segments(currentNode);
        unsigned numOfConnected = connected.size();
        
        // For every outgoing intersection
        for(unsigned i = 0; i < numOfConnected; i++) {
            unsigned segID = connected[i];      // Segment id
            
            // Check if one way going towards the current intersection.
            // If it is, skip it
            StreetSegmentInfo segInfo = getStreetSegmentInfo(segID);
            if(segInfo.oneWay && segInfo.to == currentNode)
                continue;
            
            // Find the connected intersection id
            unsigned nextNode;
            if(segInfo.to == currentNode)
                nextNode = segInfo.from;
            else
                nextNode = segInfo.to;
            
            // Avoid going in a back and forth loop between two intersections
            if(pathNodes[nextNode].visited && pathNodes[currentNode].previous == segID)
                continue;
            
            // Distance cost associated with nextNode along this path
            double distance = 
                getDistanceCost(currentNode, nextNode, segID, intersect_id_end, true);
            
            // If the distance cost is more than the nextNode's current
            // distance cost, skip it
            if(distance > pathNodes[nextNode].distance)
                continue;
            
            // Update the distance cost of the connected node
            pathNodes[nextNode].distance = distance;
            
            // Update the previous street segment
            pathNodes[nextNode].previous = segID;
            
            // Add the nextNode to the frontier.
            // Note: this algorithm uses a lazy delete. If the nextNode is
            // already in the frontier, it will simply be re-added. This is to
            // avoid having to: 
            // 1) Update the frontier and resort it with the new value
            // 2) Search through the frontier to check if nextNode is inside
            // Instead, because the distance cost associated with nextNode is 
            // lower than what it use to be at this point in the algorithm,
            // the new addition in frontier will precede the old one. Therefore,
            // the new updated one will be visited first and have its visited flag
            // set to true. When popping from the top of the frontier, we continue
            // until the popped intersection has not been visited (was not a double)
            QueueNode nextQueueNode(nextNode, distance);
            frontier.push(nextQueueNode);
        }
    }
    
    // Construct the path from the start to end intersection (if one was found),
    // and print the travel directions
    
    pathBetweenIntersections = constructPath(intersect_id_end);
    /*
    if(pathBetweenIntersections.size() > 0) {
        printTravelTime(intersect_id_end);
        directions(pathBetweenIntersections, intersect_id_start);
    }
    else if(intersect_id_start == intersect_id_end)
        cout << "You have arrived at your destination." << endl << endl;
    else
        cout << "There is no path between these intersections." << endl << endl;
    */
    return pathBetweenIntersections;
}

// Returns the time required to travel along the path specified. The path
// is passed in as a vector of street segment ids, and this function can 
// assume the vector either forms a legal path or has size == 0.
// The travel time is the sum of the length/speed-limit of each street 
// segment, plus 15 seconds per turn implied by the path. A turn occurs
// when two consecutive street segments have different street names.
double compute_path_travel_time(const std::vector<unsigned>& path) {
    double pathTravelTime = 0.0;
    
    unsigned pathSize = path.size();
    unsigned previousStreetID;
    
    // Compute the travel time from the first intersection to the next one,
    // if the size of the path is not zero.
    // This computation is outside of the for-loop because the first
    // street segment does not have a turn penalty associated to it.
    if(pathSize) {
        pathTravelTime += find_street_segment_travel_time(path[0]);
        StreetSegmentInfo segInfo = getStreetSegmentInfo(path[0]);
        previousStreetID = segInfo.streetID;
    }
    
    // Compute the travel time associated to each street segment and
    // the turn penalties for switching streetIDs.
    for(unsigned segIdx = 1; segIdx < pathSize; segIdx++) {
        unsigned segID = path[segIdx];
        
        pathTravelTime += find_street_segment_travel_time(segID);
        
        StreetSegmentInfo segInfo = getStreetSegmentInfo(segID);
        unsigned streetID = segInfo.streetID;
        
        // If we changed streetIDs relative to the previous segment in the path,
        // add the turn penalty
        if(streetID != previousStreetID)
            pathTravelTime += turnTime;
        
        // The current streetID will become the previous one in the next
        // iteration of the loop
        previousStreetID = streetID;
    }
    
    return pathTravelTime;
}


// Returns the shortest travel time path (vector of street segments) from 
// the start intersection to a point of interest with the specified name.
// If no such path exists, returns an empty (size == 0) vector.
std::vector<unsigned> find_path_to_point_of_interest (
        unsigned intersect_id_start,
        std::string point_of_interest_name
        ) {
    vector<unsigned> path(0);
    
    // The intersection id near the closest poi to the start.
    // Initialize it to an invalid number since no path has been found yet.
    unsigned closestIntersection = UINT_MAX;
    
    // Reset the path
    resetPathNodes();
    
    // All points of interest with the given name
    vector<unsigned> poiIDs = poiIDsFromName(point_of_interest_name);
    
    // Destination intersections closest to each point of interest
    vector<unsigned> endIntersections(0);
    
    // Populate the destination intersections
    unsigned numOfDestinations = poiIDs.size();
    for(unsigned poiIdx = 0; poiIdx < numOfDestinations; poiIdx++) {
        unsigned poiID = poiIDs[poiIdx];
        LatLon poiLatLon = getPointOfInterestPosition(poiID);
        unsigned destination = find_closest_intersection(poiLatLon);
        endIntersections.push_back(destination);
    }
    
    // If there is only one point of interest with the given name,
    // We find the path with A* the exact same way as the path between
    // two intersections.
    if(numOfDestinations == 1) {
        path = find_path_between_intersections(intersect_id_start, endIntersections[0]);
        return path;
    }
    
    // Set the distance associated to the start to 0
    pathNodes[intersect_id_start].distance = 0.0;
    
    // Initialize the priority queue for frontier unvisited intersections
    priority_queue<QueueNode, vector<QueueNode>, compareIntersectionDistances> frontier;
    double queueNodeDistance = pathNodes[intersect_id_start].distance;
    QueueNode queueNode(intersect_id_start, queueNodeDistance);
    frontier.push(queueNode);
    
    // While there is still a possible path to the destination
    while(!frontier.empty()) {
        
        // Evaluate the unvisited node with the lowest distance cost
        unsigned currentNode = frontier.top().id;
        frontier.pop();
        if(pathNodes[currentNode].visited)
            continue;
        
        pathNodes[currentNode].visited = true;
        
        // Check if we have reached one of the possible destinations
        auto closest = find(endIntersections.begin(), 
                endIntersections.end(), currentNode);
        if(closest != endIntersections.end()) {
            closestIntersection = *closest;
            break;
        }
        
        // All the connected street segments to the current intersection
        vector<unsigned> connected = find_intersection_street_segments(currentNode);
        unsigned numOfConnected = connected.size();
        
        // For every outgoing intersection
        for(unsigned i = 0; i < numOfConnected; i++) {
            unsigned segID = connected[i];      // Segment id
            
            // Check if one way going towards the current intersection.
            // If it is, skip it
            StreetSegmentInfo segInfo = getStreetSegmentInfo(segID);
            if(segInfo.oneWay && segInfo.to == currentNode)
                continue;
            
            // Find the connected intersection id
            unsigned nextNode;
            if(segInfo.to == currentNode)
                nextNode = segInfo.from;
            else
                nextNode = segInfo.to;
            
            // Avoid going in a back and forth loop between two intersections
            if(pathNodes[nextNode].visited && pathNodes[currentNode].previous == segID)
                continue;
            
            // Distance cost associated with nextNode along this path.
            // No need to pass in a destination intersection because
            // we are not using A*.
            double distance = 
                getDistanceCost(currentNode, nextNode, segID, 0, false);
            
            // If the distance cost is more than the nextNode's current
            // distance cost, skip it
            if(distance > pathNodes[nextNode].distance)
                continue;
            
            // Update the distance cost of the connected node
            pathNodes[nextNode].distance = distance;
            
            // Update the previous street segment
            pathNodes[nextNode].previous = segID;
            
            // Add the nextNode to the frontier.
            // Note: this algorithm uses a lazy delete. If the nextNode is
            // already in the frontier, it will simply be re-added. This is to
            // avoid having to: 
            // 1) Update the frontier and resort it with the new value
            // 2) Search through the frontier to check if nextNode is inside
            // Instead, because the distance cost associated with nextNode is 
            // lower than what it use to be at this point in the algorithm,
            // the new addition in frontier will precede the old one. Therefore,
            // the new updated one will be visited first and have its visited flag
            // set to true. When popping from the top of the frontier, we continue
            // until the popped intersection has not been visited (was not a double)
            QueueNode nextQueueNode(nextNode, distance);
            frontier.push(nextQueueNode);
        }
    }
    
    // Construct the path from the start to end intersection (if one was found)
    if(closestIntersection != UINT_MAX){
        path = constructPath(closestIntersection);
    }
    
    // Print the travel directions
    if(path.size() > 0) {
        printTravelTime(closestIntersection);
        directions(path, intersect_id_start);
    }
    else if(intersect_id_start == closestIntersection)
        cout << "You have arrived at your destination." << endl << endl;
    else
        cout << "There is no path between these intersections." << endl << endl;
    
    return path;
    
    
}

// Constructs a path from the end intersection by tracing back the
// previous street segments recursively
vector<unsigned> constructPath(unsigned end) {
    vector<unsigned> path;
    
    vector<unsigned> reversePath;
    
    // Construct the reverse path from end to start
    unsigned currentNode = end;
    while(pathNodes[currentNode].previous != UINT_MAX) {    // While there is a previous segment
        unsigned segID = pathNodes[currentNode].previous;
        reversePath.push_back(segID);
        
        // Get the next intersection in the list, which is the intersection at
        // the other end of the street segment
        StreetSegmentInfo segInfo = getStreetSegmentInfo(segID);
        if(segInfo.to == currentNode)
            currentNode = segInfo.from;
        else
            currentNode = segInfo.to;
    }
    
    // Reverse the path
    int pathSize = reversePath.size();
    for(int i = (pathSize-1); i >= 0; i--) {
        path.push_back(reversePath[i]);
    }
    
    return path;
}

double getDistanceCost(unsigned currentNode, unsigned nextNode, 
    unsigned segID, unsigned endNode, bool aStar) {
    // Travel time along the street segment from current to next intersection
    double segDistance = find_street_segment_travel_time(segID);
    
    // Ideal travel time from nextNode to the destination minus
    // ideal travel time from currentNode to the destination
    double heuristicDistance = 0.0;
    if(aStar)
        heuristicDistance = heuristicDistanceCost(currentNode, nextNode, endNode);
    
    // Turn penalty associated with going from currentNode to nextNode
    double penalty = turnPenalty(currentNode, segID);
    
    // Travel time associated with going from the current node to the next node,
    // then ideally from the next node to the destination node, minus turn penalties
    double distance = pathNodes[currentNode].distance + 
        segDistance + heuristicDistance + penalty;
    
    return distance;
}

// The heuristic function for the A* path finding algorithm.
// The heuristic cost is essentially the travel time corresponding to going from
// the next node all the way to the destination node if you were traveling on
// an ideal straight highway. We also subtract the ideal travel time from the
// current node to the destination node, so that when we add the heuristic cost
// to the total distance cost, the resulting travel time corresponds to going
// from the start node to the next node and then straight to the destination node.
double heuristicDistanceCost(unsigned currentNode, unsigned nextNode, unsigned endNode) {
    LatLon currentNodeLatLon = getIntersectionPosition(currentNode);
    LatLon nextNodeLatLon = getIntersectionPosition(nextNode);
    LatLon endNodeLatLon = getIntersectionPosition(endNode);
    
    double distanceCurrentToEnd = 
        find_distance_between_two_points(currentNodeLatLon, endNodeLatLon);
    double distanceNextToEnd = 
        find_distance_between_two_points(nextNodeLatLon, endNodeLatLon);
    
    double currentToEndTravelTime = 
        distanceCurrentToEnd / 1000.0 / upperSpeedLimit * 60.0;
    double nextToEndTravelTime = 
        distanceNextToEnd / 1000.0 / upperSpeedLimit * 60.0;
    
    return nextToEndTravelTime - currentToEndTravelTime;
}

// A time penalty associated to taking a turn (changing streetIDs)
double turnPenalty(unsigned currentNode, unsigned segID) {
    unsigned previousSegID = pathNodes[currentNode].previous;
    
    // There was no previous street segment. We were at the start
    if(previousSegID == UINT_MAX)
        return 0.0;
    
    StreetSegmentInfo previousSegInfo = getStreetSegmentInfo(previousSegID);
    StreetSegmentInfo currentSegInfo = getStreetSegmentInfo(segID);
    
    unsigned previousStreetID = previousSegInfo.streetID;
    unsigned currentStreetID = currentSegInfo.streetID;
    
    // If the street is different, assume a turn was made and add
    // the turn penalty. Otherwise, no turn was made and add no penalty.
    if(previousStreetID != currentStreetID)
        return turnTime;
    else
        return 0.0;
}

// Reset the pathNodes to their initialization state, so that we can
// compute a new path
void resetPathNodes() {
    pathNodes = vector<pathNode>(getNumberOfIntersections());
}

// Prints the travel time
void printTravelTime(unsigned destination) {
    double travel = pathNodes[destination].distance;
    int hours = (int)(travel / 60);
    int minutes = (int)travel % 60;
    int seconds = (int)((travel - 60*hours - minutes)*60);
    
    cout << endl << "Travel Time: ";
    if(hours)
        cout << hours << " hours, ";
    if(minutes)
        cout << minutes << " minutes, ";
    cout << seconds << " seconds" << endl << endl;
}

// Prints out the directions for a path of street segments
void directions(const vector<unsigned>& path, unsigned startIntersection) {
    // Print out the direction for the first street segment
    unsigned currentSeg = path[0];
    printStartDirection(startIntersection, path[0]);
    double length = find_street_segment_length(currentSeg);
    cout << "Continue for ";
    
    // Print out directions for all subsequent street segments
    unsigned previousSeg = currentSeg;
    StreetSegmentInfo curSegInfo = getStreetSegmentInfo(previousSeg);
    string prevStreetName = getStreetName(curSegInfo.streetID);
    string curStreetName;
    unsigned sizeOfPath = path.size();
    for(unsigned i = 1; i < sizeOfPath; i++) {
        currentSeg = path[i];
        
        curSegInfo = getStreetSegmentInfo(currentSeg);
        curStreetName = getStreetName(curSegInfo.streetID);
        
        // If we haven't changed streets, or only have one option, 
        //simply increment the distance of the current sub-path we are traveling on
        if(noChangeInDirection(previousSeg, currentSeg))
            length += find_street_segment_length(currentSeg);
        
        // Report the distance that the user needed to travel along the previous
        // sub-path.
        // Report a turn or action that needs to be made.
        // Reset the length for the new sub-path.
        else {
            cout << (int)length << " meters" << endl;   // Continue for x meters
            
            // Make a u-turn if the street names are the same by this point in
            // the code and their names are known
            if(prevStreetName != "<unknown>" &&
                    prevStreetName == curStreetName) {
                cout << "Make a u-turn" << endl;
            }
            
            // There is a turn to report
            else {
                double angleBetweenSegments = angleBetweenStreetSegs(previousSeg, currentSeg);
                int ambiguity = ambiguityForAction(previousSeg, currentSeg, angleBetweenSegments);

                // No other possible turns in the direction from previousSeg to currentSeg.
                // Simply report the appropriate turn to the user.
                if(ambiguity == 0 || fabs(angleBetweenSegments) < 10)
                    printUnambiguousAction(curStreetName, angleBetweenSegments);
                // Other possible turns in the direction from previousSeg to currentSeg.
                // Report as the n'th turn from the right/left.
                else 
                    printAmbiguousAction(curStreetName, ambiguity);
            }
            
            // Reset the length for the new sub-path
            length = find_street_segment_length(currentSeg);
            cout << "Continue for ";
        }
        
        previousSeg = currentSeg;
        prevStreetName = curStreetName;
    }
    
    cout << (int)length << " meters" << endl;   // Continue for x meters
    
    cout << "You have arrived at your destination." << endl << endl;
}

// Print out the initial direction on the street that the user has to go,
// based on the angle they need to head in
void printStartDirection(unsigned startIntersection, unsigned streetSeg) {
    double startAngle = angleOfStreetSeg(startIntersection, streetSeg);
    
    StreetSegmentInfo segInfo = getStreetSegmentInfo(streetSeg);
    unsigned streetID = segInfo.streetID;
    string streetName = getStreetName(streetID);
    
    cout << "Head ";
    
    if(startAngle <= 22.5 || startAngle > 337.5)
        cout << "East ";
    else if(startAngle <= 67.5)
        cout << "North East ";
    else if(startAngle <= 112.5)
        cout << "North ";
    else if(startAngle <= 157.5)
        cout << "North West ";
    else if(startAngle <= 202.5)
        cout << "West ";
    else if(startAngle <= 247.5)
        cout << "South West ";
    else if(startAngle <= 292.5)
        cout << "South ";
    else
        cout << "South East ";
    
    cout << "down " << streetName << endl; 
}

// Print an action that needs to be taken where there is no ambiguity
// i.e. there is only one possible left turn or right turn
void printUnambiguousAction(string streetName, double angle) {
    if(angle < -120)
        cout << "Take a sharp left";
    else if(angle < -50)
        cout << "Take a left";
    else if(angle < -10)
        cout << "Keep left";
    else if(angle <= 10)
        cout << "Continue straight";
    else if(angle <= 50)
        cout << "Keep right";
    else if(angle <= 120)
        cout << "Take a right";
    else
        cout << "Take a sharp right";
    
    // If the street name is unknown, don't print it
    if(streetName != "<unknown>")
        cout << " onto " << streetName;
    cout << endl;
        
}

// Print an action that needs to be taken where there is ambiguity
// i.e. the user needs to turn left but there are multiple left turns possible.
// Prints in the format Keep left onto x, or Take the n'th left onto x
void printAmbiguousAction(string streetName, int turnNumber) {
    unsigned turnOrder = abs(turnNumber);
    
    if(turnOrder == 1)
        cout << "Keep ";
    else {
        cout << "Take the " << turnOrder;
        if(turnOrder == 2)
            cout << "nd street from the ";
        else if(turnOrder == 3)
            cout << "rd street from the ";
        else
            cout << "th street from the ";
    }
    
    if(turnNumber < 0)
        cout << "left";
    else
        cout << "right";
    
    // If the street name is unknown, don't print it
    if(streetName != "<unknown>")
        cout << " onto " << streetName;
    cout << endl;
}

// Returns true if the user is staying on the same street (without u-turns),
// or if there is only one option available
bool noChangeInDirection(unsigned seg1, unsigned seg2) {
    StreetSegmentInfo segInfo1 = getStreetSegmentInfo(seg1);
    StreetSegmentInfo segInfo2 = getStreetSegmentInfo(seg2);
    string streetName1 = getStreetName(segInfo1.streetID);
    string streetName2 = getStreetName(segInfo2.streetID);
    
    // If there is no other option but to go from seg1 to seg2,
    // no change in directions
    unsigned sharedIntersection = sharedIntersectionBetweenSegments(seg1, seg2);
    vector<unsigned> connected = find_intersection_street_segments(sharedIntersection);
    unsigned numOfConnected = connected.size();
    if(numOfConnected == 2)
        return true;
    
    // If the streets are unknown, we cannot determine if there is a change in directions.
    // Assume that there is
    if(streetName1 == "<unknown>" && streetName2 == "<unknown>")
        return false;
    
    // If the street name is the same, and there was no u-turn, change in directions
    unsigned numOfSameStreets = 0;
    for(unsigned i = 0; i < numOfConnected; i++) {
        StreetSegmentInfo segInfo = getStreetSegmentInfo(connected[i]);
        string streetName = getStreetName(segInfo.streetID);
        if(streetName == streetName1)
            numOfSameStreets++;
    }
    double angle = angleBetweenStreetSegs(seg1, seg2);
    if(streetName1 == streetName2 &&  (numOfSameStreets <= 2 || fabs(angle) < 90))
        return true;
    
    // There is a change in directions
    return false;
}

// Returns a rough approximation of the street segment's angle between 0 and 360 degrees,
// relative to the horizontal right
double angleOfStreetSeg(unsigned startIntersection, unsigned streetSeg) {
    LatLon startLatLon = getIntersectionPosition(startIntersection);
    
    StreetSegmentInfo segInfo = getStreetSegmentInfo(streetSeg);
    LatLon endLatLon;
    if(segInfo.from == startIntersection) {
        if(segInfo.curvePointCount)
            endLatLon = getStreetSegmentCurvePoint(streetSeg, 0);
        else
            endLatLon = getIntersectionPosition(segInfo.to);
    }
    else {
        if(segInfo.curvePointCount)
            endLatLon = getStreetSegmentCurvePoint(streetSeg, segInfo.curvePointCount-1);
        else
            endLatLon = getIntersectionPosition(segInfo.from);
    }
    
    double horizontal = endLatLon.lon - startLatLon.lon;
    double vertical = endLatLon.lat - startLatLon.lat;
    
    double angle = atan2(vertical, horizontal)/DEG_TO_RAD;
    if(angle < 0.0)
        angle += 360.0;
    
    return angle;
}

// Returns a rough approximation of the angle going from seg1 to seg2 between
// -180 and 180. Negative signifies a left, positive signifies a right. 0 signifies straight.
double angleBetweenStreetSegs(unsigned seg1, unsigned seg2) {
    StreetSegmentInfo segInfo1 = getStreetSegmentInfo(seg1);
    StreetSegmentInfo segInfo2 = getStreetSegmentInfo(seg2);
    
    // Find the start point, middle (shared intersection), and end point.
    // If the segment has curved points, the start/end will be the curve point
    // closest to the middle, in order to increase accuracy of the angle.
    LatLon start, middle, end;
    if(segInfo1.from == segInfo2.from) {
        if(segInfo1.curvePointCount)
            start = getStreetSegmentCurvePoint(seg1, 0);
        else
            start = getIntersectionPosition(segInfo1.to);
        middle = getIntersectionPosition(segInfo1.from);
        if(segInfo2.curvePointCount)
            end = getStreetSegmentCurvePoint(seg2, 0);
        else
            end = getIntersectionPosition(segInfo2.to);
    } else if(segInfo1.from == segInfo2.to) {
        if(segInfo1.curvePointCount)
            start = getStreetSegmentCurvePoint(seg1, 0);
        else
            start = getIntersectionPosition(segInfo1.to);
        middle = getIntersectionPosition(segInfo1.from);
        if(segInfo2.curvePointCount)
            end = getStreetSegmentCurvePoint(seg2, segInfo2.curvePointCount-1);
        else
            end = getIntersectionPosition(segInfo2.from);
    } else if(segInfo1.to == segInfo2.from) {
        if(segInfo1.curvePointCount)
            start = getStreetSegmentCurvePoint(seg1, segInfo1.curvePointCount-1);
        else
            start = getIntersectionPosition(segInfo1.from);
        middle = getIntersectionPosition(segInfo1.to);
        if(segInfo2.curvePointCount)
            end = getStreetSegmentCurvePoint(seg2, 0);
        else
            end = getIntersectionPosition(segInfo2.to);
    } else {
        if(segInfo1.curvePointCount)
            start = getStreetSegmentCurvePoint(seg1, segInfo1.curvePointCount-1);
        else
            start = getIntersectionPosition(segInfo1.from);
        middle = getIntersectionPosition(segInfo1.to);
        if(segInfo2.curvePointCount)
            end = getStreetSegmentCurvePoint(seg2, segInfo2.curvePointCount-1);
        else
            end = getIntersectionPosition(segInfo2.from);
    }
    
    double horizontal1 = middle.lon - start.lon;
    double vertical1 = middle.lat - start.lat;
    double angle1 = atan2(vertical1, horizontal1)/DEG_TO_RAD;
    if(angle1 < 0.0)    // Normalize the angle
        angle1 += 360.0;
    
    double horizontal2 = end.lon - middle.lon;
    double vertical2 = end.lat - middle.lat;
    double angle2 = atan2(vertical2, horizontal2)/DEG_TO_RAD;
    if(angle2 < 0.0)    // Normalize the angle
        angle2 += 360.0;
    
    // Angle between must be in rangle -180 to +180 degrees
    double angleBetween = angle1 - angle2;
    if(angleBetween < -180.0)
        angleBetween += 360.0;
    else if(angleBetween > 180.0)
        angleBetween -= 360;
    
    // If the points are so close that the distance between them was zero,
    // then there is no angle. Simply return 0 degrees (no angle between segs)
    if((horizontal1==0&&vertical1==0)||(horizontal2==0&&vertical2==0))
        return 0.0;
    
    return angleBetween;
}

// Checks to see if there are multiple possible turns in the direction going
// from seg1 to seg2.
// If there are, it returns a positive integer n which signifies that the turn
// from seg1 to seg2 corresponds to the n'th turn from the right or left.
// -ve signifies a left, +ve signifies a right.
int ambiguityForAction(unsigned seg1, unsigned seg2, double angle) {
    // Find the shared intersection between the two segments
    unsigned sharedIntersection = sharedIntersectionBetweenSegments(seg1, seg2);
    
    // All the connected street segments to the current intersection
    vector<unsigned> connected = find_intersection_street_segments(sharedIntersection);
    unsigned numOfConnected = connected.size();
    
    // Count of the number of street segments on the same turn side as
    // seg2
    int turnCount = 0;
    // For every outgoing segment
    for(unsigned i = 0; i < numOfConnected; i++) {
        unsigned segID = connected[i];      // Segment id
        
        // If the segment is one of the segments involved in the action,
        // skip it
        if(segID == seg1 || segID == seg2)
            continue;
        
        // If the segment is a continuation of the previous one,
        // then skip it
        StreetSegmentInfo segInfo1 = getStreetSegmentInfo(seg1);
        StreetSegmentInfo newSegInfo = getStreetSegmentInfo(segID);
        if(getStreetName(segInfo1.streetID) == getStreetName(newSegInfo.streetID))
            continue;

        // Check if one way going towards the current intersection.
        // If it is, skip it
        StreetSegmentInfo segInfo = getStreetSegmentInfo(segID);
        if(segInfo.oneWay && segInfo.to == sharedIntersection)
            continue;
        
        // Find the angle between seg1 and the current segment being checked
        double newAngle = angleBetweenStreetSegs(seg1, segID);
        
        // If the current segment is on the same turn side as seg2
        if(angle > 0.0) {   // Turn is on the right
            if(newAngle < 0.0)  // The other segment is on the left
                continue;
            if(turnCount == 0)  // The turn is ambiguous.
                turnCount = 1;
            if(newAngle > angle)    // At least the n'th turn from the right
                turnCount++;
        }
        else {              // Turn is on the left
            if(newAngle > 0.0)  // The other segment is on the right
                continue;
            if(turnCount == 0)  // The turn is ambiguous.
                turnCount = -1;
            if(newAngle < angle)    // At least the n'th turn from the left
                turnCount--;
        }
    }
    
    return turnCount;
}

// Returns the shared intersection id between two street segments
unsigned sharedIntersectionBetweenSegments(unsigned seg1, unsigned seg2) {
    // Find the shared intersection between the two segments
    unsigned sharedIntersection;
    StreetSegmentInfo segInfo1 = getStreetSegmentInfo(seg1);
    StreetSegmentInfo segInfo2 = getStreetSegmentInfo(seg2);
    if(segInfo1.from == segInfo2.from || segInfo1.from == segInfo2.to)
        sharedIntersection = segInfo1.from;
    else
        sharedIntersection = segInfo1.to;
    
    return sharedIntersection;
}

/* ---------------------------------------------------------------------------*/
/* MULTI THREADING CODE -- VERY MESSY */
vector<vector<pathNode>> threadedPathNodes(NUM_THREADS);

void resetPathNodesThread(unsigned i) {
    threadedPathNodes[i] = vector<pathNode>(getNumberOfIntersections());
}

// Comparator function for the priority_queue
struct threadedCompareIntersectionDistances {
    bool operator()(const QueueNode& first, const QueueNode& second) {
        return first.distance > second.distance;
    }
};

double threadTurnPenalty(unsigned currentNode, unsigned segID, unsigned thread) {
    unsigned previousSegID = threadedPathNodes[thread][currentNode].previous;
    
    // There was no previous street segment. We were at the start
    if(previousSegID == UINT_MAX)
        return 0.0;
    
    StreetSegmentInfo previousSegInfo = getStreetSegmentInfo(previousSegID);
    StreetSegmentInfo currentSegInfo = getStreetSegmentInfo(segID);
    
    unsigned previousStreetID = previousSegInfo.streetID;
    unsigned currentStreetID = currentSegInfo.streetID;
    
    // If the street is different, assume a turn was made and add
    // the turn penalty. Otherwise, no turn was made and add no penalty.
    if(previousStreetID != currentStreetID)
        return turnTime;
    else
        return 0.0;
}

double threadGetDistanceCost(unsigned currentNode, unsigned nextNode, 
    unsigned segID, unsigned thread) {
    // Travel time along the street segment from current to next intersection
    double segDistance = find_street_segment_travel_time(segID);
    
    // Turn penalty associated with going from currentNode to nextNode
    double penalty = threadTurnPenalty(currentNode, segID, thread);
    
    // Travel time associated with going from the current node to the next node,
    // then ideally from the next node to the destination node, minus turn penalties
    double distance = threadedPathNodes[thread][currentNode].distance + 
        segDistance + penalty;
    
    return distance;
}

// Maps every combination of intersections to an associated distance
// between them and stores it in the distanceCostMap.
// Maps every delivery intersection to its closest delivery intersections
// and stores it in closestDeliveryMap.
// Maps every delivery intersection to its closest depot intersections
// and stores it in closestDepotMap.
void courierDijkstra(const vector<unsigned>& range, const vector<IntersectionContent>& intersectionContents,
        costMap& distanceCostMap, closestMap& closestDeliveryMap, closestMap& closestDepotMap,
        unsigned thread, unsigned thingsToFind) { 
    // Apply dijkstra to every delivery in the set
    unsigned rangeSize = range.size();
    for(unsigned i = 0; i < rangeSize; i++) {
        unsigned foundCount = 0;
        unsigned startIntersection = range[i];
        
        // Reset the path
        resetPathNodesThread(thread);

        // Set the distance associated to the start to 0
        threadedPathNodes[thread][startIntersection].distance = 0.0;

        // Initialize the priority queue for frontier unvisited intersections
        typedef priority_queue<QueueNode, vector<QueueNode>, threadedCompareIntersectionDistances> threadQueue;
        threadQueue frontier;
        double queueNodeDistance = threadedPathNodes[thread][startIntersection].distance;
        QueueNode queueNode(startIntersection, queueNodeDistance);
        frontier.push(queueNode);
        
        // Dijkstra
        // While there is still a possible path to the destination
        while(!frontier.empty() && foundCount < thingsToFind) {

            // Evaluate the unvisited node with the lowest distance cost
            unsigned currentNode = frontier.top().id;
            frontier.pop();
            if(threadedPathNodes[thread][currentNode].visited)
                continue;

            threadedPathNodes[thread][currentNode].visited = true;
            
            if(currentNode != startIntersection) {
                // Check if we have reached a delivery
                if(intersectionContents[currentNode].isDelivery) {
                    foundCount++;
                    // Update the cost map
                    distanceCostMap[startIntersection][currentNode] = threadedPathNodes[thread][currentNode].distance;
                    // Add the closest intersection to the closest map
                    closestDeliveryMap[startIntersection].push_back(currentNode);
                }
                
                // Check if we have reached a depot
                if(intersectionContents[currentNode].isDepot) {
                    foundCount++;
                    // Update the cost map
                    distanceCostMap[startIntersection][currentNode] = threadedPathNodes[thread][currentNode].distance;
                    
                    // Add the closest depot to the closest map
                    closestDepotMap[startIntersection].push_back(currentNode);
                }
            }

            // All the connected street segments to the current intersection
            vector<unsigned> connected = find_intersection_street_segments(currentNode);
            unsigned numOfConnected = connected.size();

            // For every outgoing intersection
            for(unsigned i = 0; i < numOfConnected; i++) {
                unsigned segID = connected[i];      // Segment id

                // Check if one way going towards the current intersection.
                // If it is, skip it
                StreetSegmentInfo segInfo = getStreetSegmentInfo(segID);
                if(segInfo.oneWay && segInfo.to == currentNode)
                    continue;

                // Find the connected intersection id
                unsigned nextNode;
                if(segInfo.to == currentNode)
                    nextNode = segInfo.from;
                else
                    nextNode = segInfo.to;

                // Do not revisit nodes
                if(threadedPathNodes[thread][nextNode].visited)
                    continue;

                // Distance cost associated with nextNode along this path
                double distance = 
                    threadGetDistanceCost(currentNode, nextNode, segID, thread);

                // If the distance cost is more than the nextNode's current
                // distance cost, skip it
                if(distance > threadedPathNodes[thread][nextNode].distance)
                    continue;

                // Update the distance cost of the connected node
                threadedPathNodes[thread][nextNode].distance = distance;

                // Update the previous street segment
                threadedPathNodes[thread][nextNode].previous = segID;

                // Add the nextNode to the frontier.
                // Note: this algorithm uses a lazy delete. If the nextNode is
                // already in the frontier, it will simply be re-added. This is to
                // avoid having to: 
                // 1) Update the frontier and resort it with the new value
                // 2) Search through the frontier to check if nextNode is inside
                // Instead, because the distance cost associated with nextNode is 
                // lower than what it use to be at this point in the algorithm,
                // the new addition in frontier will precede the old one. Therefore,
                // the new updated one will be visited first and have its visited flag
                // set to true. When popping from the top of the frontier, we continue
                // until the popped intersection has not been visited (was not a double)
                QueueNode nextQueueNode(nextNode, distance);
                frontier.push(nextQueueNode);
            }
        }
    }
}

// Builds the cost maps with simple distance point to point
void costsSimple(const vector<unsigned>& range, const set<unsigned>& deliveries, const set<unsigned>& depots,
        costMap& distanceCostMap, closestMap& closestDeliveryMap, closestMap& closestDepotMap) {
    unsigned rangeSize = range.size();
    for(unsigned i = 0; i < rangeSize; i++) {
        unsigned start = range[i];
        LatLon startLatLon = getIntersectionPosition(start);
        
        // Build the deliveries costMap
        for(auto delIter = deliveries.begin();
                delIter != deliveries.end();
                delIter++) {
            unsigned end = *delIter;
            if(end == start)
                continue;
            LatLon endLatLon = getIntersectionPosition(end);
            
            double distance = find_distance_between_two_points(startLatLon, endLatLon);
            distanceCostMap[start][end] = distance;
            
            vector<unsigned>& closestDeliveries = closestDeliveryMap[start];
            bool found = false;
            auto insertPos = closestDeliveries.begin();
            while(insertPos < closestDeliveries.end() && !found) {
                unsigned currentInter = *insertPos;
                double distanceTo = distanceCostMap[start][currentInter];
                if(distance <= distanceTo)
                    found = true;
                else
                    insertPos++;
            }
            closestDeliveries.insert(insertPos, end);
        }
        
        // Build the depots costMap
        for(auto depIter = depots.begin();
                depIter != depots.end();
                depIter++) {
            unsigned end = *depIter;
            if(end == start)
                continue;
            LatLon endLatLon = getIntersectionPosition(end);
            
            double distance = find_distance_between_two_points(startLatLon, endLatLon);
            distanceCostMap[start][end] = distance;
            
            vector<unsigned>& closestDepots = closestDepotMap[start];
            bool found = false;
            auto insertPos = closestDepots.begin();
            while(insertPos < closestDepots.end() && !found) {
                unsigned currentInter = *insertPos;
                double distanceTo = distanceCostMap[start][currentInter];
                if(distance <= distanceTo)
                    found = true;
                else
                    insertPos++;
            }
            closestDepots.insert(insertPos, end);
        }
    }
}