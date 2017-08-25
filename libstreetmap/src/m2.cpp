#include "m2.h"
#include "m4.h"
#include "mapping_constants.h"
#include "StreetsDatabaseAPI.h"
#include <sstream>

using namespace std;

////////////////////////////////////////////////////////////////////////////////
// GLOBAL VARIABLES
////////////////////////////////////////////////////////////////////////////////

double avgLatRad;

// feature highlighting for different modes
bool pathFindingMode = false;
bool destIsPOI = false;

    // mode: normal
    vector<unsigned>    highlightedIntersections;
    vector<unsigned>    highlightedSegments;
    vector<unsigned>    highlightedPOIs;

    // mode: path finding
    vector<unsigned>    fromIntersection;
    vector<unsigned>    toIntersection;
    vector<string>      POInames;
    vector<unsigned>    destPOI;
    vector<unsigned>    pathSegments;

////////////////////////////////////////////////////////////////////////////////
// HELPER FUNCTION DECLARATIONS
////////////////////////////////////////////////////////////////////////////////

// real to world conversion
t_point convertLatLonToWorld(LatLon point);
LatLon convertWorldToLatLon(t_point point);
float getWidthOfCurrentWindow(); // (in km)


// point manipulation
float angleBetweenPoints(t_point p1, t_point p2);
float angleBetweenPoints360(t_point p1, t_point p2);
    // gives angle in range [0,360] instead of [-90,90]
float distanceBetweenPoints(t_point p1, t_point p2);
t_point centerOfPoints(t_point p1, t_point p2);
void rotatePointAboutCen(t_point cen, float rotAngle, t_point &point);


// draw streets
void drawHighways();
void drawMotorways();
void drawServiceRoads();
void drawCommercialRoads();
void drawLocalRoads();
void drawSegments(const vector<unsigned>& segments, t_color color, int width);
void drawArrow(t_point cen, float angle);


// draw street text
void drawHighwaysText();
void drawMotorwaysText();
void drawServiceRoadsText();
void drawCommercialRoadsText();
void drawLocalRoadsText();
void drawSegmentsText(const vector<unsigned>& segments, int fontSize);
void drawTextBetweenPoints(t_point p1, t_point p2, string name);
    // assumes font size and color are already set


// draw features
void drawLakes();
void drawPonds();
void drawIslands();
void drawGreens();
void drawSands();
void drawRivers();
void drawBuildings();
void drawUnknowns();
void drawPolygonFeatures(unsigned featureID, t_color color);
void drawLineFeatures(unsigned featureID, t_color color);


// draw POIs
void drawPOIs();
void drawPOI(float windowWidth, float symbolCutoff, float textCutoff,
        t_color symbolColor, float symbolRadius, string poiType, string name, 
        string symbol, int symbolSize, t_point pos);

void drawReligious(t_point pos, float symbolRadius);
void drawEducation(t_point pos, float symbolRadius);
void drawShop(t_point pos, float symbolRadius);
void drawFood(t_point pos, float symbolRadius);
void drawBusStation(t_point pos, float symbolRadius);
void drawAutomotive(t_point pos, float symbolRadius);
void drawFerry(t_point pos, float symbolRadius);
void drawTourism(t_point pos, float symbolRadius);
void drawArts(t_point pos, float symbolRadius);
void drawLodging(t_point pos, float symbolRadius);
void drawMisc(t_point pos, float symbolRadius);


// draw highlighted features
void drawHighlightedIntersections();
void drawHighlightedStreetSegments();
void drawHighlightedPOIs();
void drawPathFinding();


// readline
char** command_completion(const char* stem_text, int start, int end);
char* name_generator(const char* stem_text, int state);


// user-interface buttons
void button_search(void (*drawscreen_ptr) (void));
void button_pathMode(void (*drawscreen_ptr) (void));
void button_help(void (*drawscreen_ptr) (void));
void promptTerminal(); // on screen prompt to use terminal


// searching
void act_on_button(void (*drawscreen_ptr) (void));
bool findIntersections(string searchField, vector<unsigned> &intersections);
        // fills passed in intersections vector
bool findStreets(string searchField);
        // automatically fills highlightedStreets vector
bool findPOIs(string searchField, vector<unsigned> &POIs);
        // fills passed in POIs vector
bool findDirections(string searchField);
        // fills pathfinding vector from user input


// miscellaneous
void removeDoubles(vector<string>& vect);
t_point getPointFromIntersectionID(unsigned intersectionID);
void outputIntersection(unsigned intersectionID);
void getClickInfo(
        const t_point &clickPoint,
        bool &clickedIntersection,
        t_point &intersectionPoint,
        unsigned &intersectionID
        );


// function that actually draws the map itself
// draw_map() opens the window and initializes
// draw_map_a() draws the new screen
void draw_map_a();


////////////////////////////////////////////////////////////////////////////////
// DRAW_MAP
////////////////////////////////////////////////////////////////////////////////
// main drawing function called by event_loop
// draws all elements of the map in the given order

void debug(void (*drawscreen_ptr) (void)) {
    highlightedSegments = find_path_between_intersections(10192, 59249);
    //fromIntersection.clear();
    //fromIntersection.push_back(12788);
    //pathSegments = find_path_to_point_of_interest(12788, "Wendy's / Tim Horton's", toIntersection);
    /*
    t_bound_box bb = get_visible_world();
    float xdiff = bb.right() - bb.left();
    float ydiff = bb.top() - bb.bottom();
    float xcen = bb.left() + xdiff/2;
    float ycen = bb.bottom() + ydiff/2;
    setcolor(PATH);
    float offset = 200;
    fillrect(xcen-offset, ycen-offset, xcen+offset, ycen+offset);
    */
    //promptTerminal();
    draw_map_a();
}

// event loop, map opening
void draw_map(){
            
    // Create a window with name and background color as specified
    t_color tan = t_color(BISQUE);
    init_graphics(" TopKEK ", tan);

    // Set the world (drawing) coordinates for the loaded map
    t_bound_box worldCoord = getWorldCoordinates();
    set_visible_world(worldCoord);

    // Set the buttons
    destroy_button("PostScript");
    destroy_button("Window");
    change_button_text("Proceed", "Change Map");
    create_button("Zoom Fit", "Help", button_help);
    create_button("Zoom Fit", "Path Mode", button_pathMode);
    create_button("Zoom Fit", "Search", button_search);
    create_button("Zoom Fit", "DEBUG", debug);

    // Start the event loop. Will continue until user clicks
    // proceed or exit. If user clicks exit, program ends.
    // If user clicks proceed, prompts for new city name.
    event_loop(act_on_mouse_click, NULL, NULL, draw_map_a);

    // Event loop is done, close the graphics window
    close_graphics();
    highlightedIntersections.clear();
    highlightedSegments.clear();
    highlightedPOIs.clear();
    fromIntersection.clear();
    toIntersection.clear();
    POInames.clear();
    pathSegments.clear();
    pathFindingMode = false;
}

// actual draw map
void draw_map_a() {
    clearscreen();
    
    // Features
    drawLakes();
    drawIslands();
    drawGreens();
    drawPonds();
    drawSands();
    drawRivers();
    
    // Buildings
    drawBuildings();
    
    // Streets
    drawLocalRoads();
    drawCommercialRoads();
    drawServiceRoads();
    drawMotorways();
    drawHighways();
    
    // Highlights
    drawHighlightedIntersections();
    drawHighlightedStreetSegments();
    drawHighlightedPOIs();
    drawPathFinding();
    
    // Street Names
    drawLocalRoadsText();
    drawCommercialRoadsText();
    drawServiceRoadsText();
    drawMotorwaysText();
    drawHighwaysText();
    
    // POIs
    drawPOIs();
}

////////////////////////////////////////////////////////////////////////////////
// FEATURE DRAWING
////////////////////////////////////////////////////////////////////////////////
// 1) get the vector of features of the feature type
// 2) set the color for the feature type
// 3) draw every feature, either as a polygon or as a line

void drawLakes() {
    // get points and color
    vector<unsigned> lakes = getLakes();
    t_color color = LIGHTSKYBLUE;
    
    // for every "lake" draw either polygon or line
    // large rivers may be classified as lake
    for (auto iter = lakes.begin();
            iter < lakes.end();
            iter++) {
        unsigned featureID = *iter;
        unsigned numOfPoints = getFeaturePointCount(featureID);
        LatLon begin = getFeaturePoint(featureID, 0);
        LatLon end = getFeaturePoint(featureID, numOfPoints - 1);
        
        // connected first and last => polygon
        if (begin.lat == end.lat && begin.lon == end.lon)
            drawPolygonFeatures(featureID, color);
        else
            drawLineFeatures(featureID, color);
    }
}

void drawPonds() {
    // get points and color
    vector<unsigned> ponds = getPonds();
    t_color color = LIGHTSKYBLUE;
    
    for (auto iter = ponds.begin();
            iter < ponds.end();
            iter++) {
        unsigned featureID = *iter;
        unsigned numOfPoints = getFeaturePointCount(featureID);
        LatLon begin = getFeaturePoint(featureID, 0);
        LatLon end = getFeaturePoint(featureID, numOfPoints - 1);
        
        // connected first and last => polygon
        if (begin.lat == end.lat && begin.lon == end.lon)
            drawPolygonFeatures(featureID, color);
        else
            drawLineFeatures(featureID, color);
    }
}
void drawIslands() {
    // get points and color
    vector<unsigned> islands = getIslands();
    t_color color = BISQUE;
    
    for (auto iter = islands.begin();
            iter < islands.end();
            iter++) {
        unsigned featureID = *iter;
        unsigned numOfPoints = getFeaturePointCount(featureID);
        LatLon begin = getFeaturePoint(featureID, 0);
        
        // connected first and last => polygon
        LatLon end = getFeaturePoint(featureID, numOfPoints - 1);
        if (begin.lat == end.lat && begin.lon == end.lon)
            drawPolygonFeatures(featureID, color);
        else
            drawLineFeatures(featureID, color);
    }
}

void drawGreens() {
    // get points and color
    vector<unsigned> greens = getGreens();
    t_color color = GRASS;
    
    for (auto iter = greens.begin();
            iter < greens.end();
            iter++) {
        unsigned featureID = *iter;
        unsigned numOfPoints = getFeaturePointCount(featureID);
        LatLon begin = getFeaturePoint(featureID, 0);
        LatLon end = getFeaturePoint(featureID, numOfPoints - 1);
        
        // connected first and last => polygon
        if (begin.lat == end.lat && begin.lon == end.lon)
            drawPolygonFeatures(featureID, color);
        else
            drawLineFeatures(featureID, color);
    }
}

void drawSands() {
    // get points and color
    vector<unsigned> sands = getSands();
    t_color color = SANDS;
    
    for (auto iter = sands.begin();
            iter < sands.end();
            iter++) {
        unsigned featureID = *iter;
        unsigned numOfPoints = getFeaturePointCount(featureID);
        LatLon begin = getFeaturePoint(featureID, 0);
        LatLon end = getFeaturePoint(featureID, numOfPoints - 1);
        
        // connected first and last => polygon
        if (begin.lat == end.lat && begin.lon == end.lon)
            drawPolygonFeatures(featureID, color);
        else
            drawLineFeatures(featureID, color);
    }
}

void drawRivers() {
    // get points and color
    vector<unsigned> rivers = getRivers();
    t_color color = LIGHTSKYBLUE;
    
    for (auto iter = rivers.begin();
            iter < rivers.end();
            iter++) {
        unsigned featureID = *iter;
        unsigned numOfPoints = getFeaturePointCount(featureID);
        LatLon begin = getFeaturePoint(featureID, 0);
        LatLon end = getFeaturePoint(featureID, numOfPoints - 1);
        
        // connected first and last => polygon
        if (begin.lat == end.lat && begin.lon == end.lon)
            drawPolygonFeatures(featureID, color);
        else
            drawLineFeatures(featureID, color);
    }
}

void drawBuildings() {
    // only draw houses at HOUSE zoom level
    float currentWidth = getWidthOfCurrentWindow();
    if (currentWidth > HOUSE) return; 
    
    // get points and color
    vector<unsigned> buildings = getBuildings();
    t_color color = LIGHTGREY;
    
    for (auto iter = buildings.begin();
            iter < buildings.end();
            iter++) {
        unsigned featureID = *iter;
        unsigned numOfPoints = getFeaturePointCount(featureID);
        
        // connected first and last => polygon
        LatLon begin = getFeaturePoint(featureID, 0);
        LatLon end = getFeaturePoint(featureID, numOfPoints - 1);
        if (begin.lat == end.lat && begin.lon == end.lon)
            drawPolygonFeatures(featureID, color);
        else
            drawLineFeatures(featureID, color);
    }
}

void drawPolygonFeatures(unsigned featureID, t_color color) {
    // create array of t_points to be filled with each point in polygon
    unsigned numOfPoints = getFeaturePointCount(featureID);
    t_point *points = new t_point[numOfPoints];
    
    // set the drawing color
    setcolor(color);
    
    // fill the t_point array
    for (unsigned pointCount = 0; pointCount < numOfPoints; pointCount++) {
        t_point point = convertLatLonToWorld(getFeaturePoint(featureID, pointCount));
        points[pointCount] = point;
    }
    
    // draw the polygon
    fillpoly(points, numOfPoints);
    
    delete[] points;
}

void drawLineFeatures(unsigned featureID, t_color color) {
    // create array of t_points to be filled with each point of line
    unsigned numOfPoints = getFeaturePointCount(featureID);
    t_point *points = new t_point[numOfPoints];
    
    // set the drawing color
    setcolor(color);
    
    // fill the t_point array
    for (unsigned pointCount = 0; pointCount < numOfPoints; pointCount++) {
        t_point point = convertLatLonToWorld(getFeaturePoint(featureID, pointCount));
        points[pointCount] = point;
    }
    
    // draw a line between consecutive points
    for (unsigned pointCount = 0; pointCount < numOfPoints - 1; pointCount++)
        drawline(points[pointCount], points[pointCount + 1]);
    
    delete[] points;
}

////////////////////////////////////////////////////////////////////////////////
// STREET DRAWING
////////////////////////////////////////////////////////////////////////////////
// 1) get the vector of streets corresponding to that street class
// 2) set the color and linewidth depending on the current zoom level
//      - ex) draw commercial roads blended at low zoom but
//              normally at high zoom
// 3) only draw the streets if the zoom level is appropriate
//      - ex) don't draw local streets and low zoom
// 4) if zoom is high enough, increase street width based on zoom level
//      - (at very high zoom, streets should be thicker,
//          proportionate to surrounding buildings)

void drawHighways() {
    vector<unsigned> highways = getHighways();

    t_color coreColor = HIGHWAY;
    int coreWidth = STREETWIDTH_CORE;

    float currentWidth = getWidthOfCurrentWindow();
    if(currentWidth < HOUSE) {
        coreWidth = 1/currentWidth * 8;
    }

    drawSegments(highways, coreColor, coreWidth);
}

void drawMotorways() {
    vector<unsigned> motorways = getMotorways();

    t_color coreColor = MOTORWAY;
    int coreWidth = STREETWIDTH_CORE;

    float currentWidth = getWidthOfCurrentWindow();
    if(currentWidth < HOUSE) {
        coreWidth = 1/currentWidth * 8;
    }

    drawSegments(motorways, coreColor, coreWidth);
}

void drawServiceRoads() {
    vector<unsigned> serviceRoads = getServiceRoads();
    float currentWidth = getWidthOfCurrentWindow();
    t_color coreColor;
    int coreWidth;

    if (currentWidth < AREA) {
        coreColor = t_color(WHITE);
        coreWidth = STREETWIDTH_CORE;
    }
    else {
        coreColor = BLENDEDROAD;
        coreWidth = STREETWIDTH_BLENDED;
    }

    if(currentWidth < HOUSE) {
        coreWidth = 1/currentWidth * 8;
    }

    if (currentWidth < CITY) {
        drawSegments(serviceRoads, coreColor, coreWidth);
    }
}

void drawCommercialRoads() {
    vector<unsigned> commercialRoads = getCommercialRoads();
    float currentWidth = getWidthOfCurrentWindow();
    t_color coreColor;
    int coreWidth;
    
    if (currentWidth < STREET) {
        coreColor = t_color(WHITE);
        coreWidth = STREETWIDTH_CORE;
    }
    else {
        coreColor = BLENDEDROAD;
        coreWidth = STREETWIDTH_BLENDED;
    }


    if(currentWidth < HOUSE) {
        coreWidth = 1/currentWidth * 8;
    }

    if (currentWidth < AREA) {
        drawSegments(commercialRoads, coreColor, coreWidth);
    }
}

void drawLocalRoads() { // CHANGE THIS
    vector<unsigned> localRoads = getLocalRoads();
    float currentWidth = getWidthOfCurrentWindow();
    t_color coreColor;
    int coreWidth;
    
    if (currentWidth < LOCAL) {
        coreColor = t_color(WHITE);
        coreWidth = STREETWIDTH_CORE;
    }
    else {
        coreColor = BLENDEDROAD;
        coreWidth = STREETWIDTH_BLENDED;
    }

    if(currentWidth < HOUSE) {
        coreWidth = 1/currentWidth * 8;
    }

    if (currentWidth < STREET) {
        drawSegments(localRoads, coreColor, coreWidth);
    }
}

////////////////////////////////////////////////////////////////////////////////
// STREET TEXT DRAWING
////////////////////////////////////////////////////////////////////////////////
// same idea as street drawing
// 1) increase font size at higher zooms
// 2) only draw names of streets if at appropriate zoom

void drawHighwaysText() {
    vector<unsigned> highways = getHighways();
    int fontSize;
    
    float currentWidth = getWidthOfCurrentWindow();
    if(currentWidth < HOUSE) {
        fontSize = 10;
    }
    else if(currentWidth < AREA) {
        fontSize = 9;
    }
    else {
        fontSize = 8;
    }
    
    drawSegmentsText(highways, fontSize);
}

void drawMotorwaysText() {
    vector<unsigned> motorways = getMotorways();
    int fontSize;
    
    float currentWidth = getWidthOfCurrentWindow();
    if(currentWidth < HOUSE) {
        fontSize = 10;
    }
    else if(currentWidth < AREA) {
        fontSize = 9;
    }
    else {
        fontSize = 8;
    }
    
    drawSegmentsText(motorways, fontSize);
}

void drawServiceRoadsText() {
    vector<unsigned> serviceRoads = getServiceRoads();
    int fontSize;
    
    float currentWidth = getWidthOfCurrentWindow();
    if(currentWidth < HOUSE) {
        fontSize = 10;
    }
    else if(currentWidth < AREA) {
        fontSize = 10;
    }
    else {
        fontSize = 9;
    }

    if (currentWidth < AREA) {
        drawSegmentsText(serviceRoads, fontSize);
    }
}

void drawCommercialRoadsText() {
    vector<unsigned> commercialRoads = getCommercialRoads();
    int fontSize;
    
    float currentWidth = getWidthOfCurrentWindow();
    if(currentWidth < HOUSE) {
        fontSize = 10;
    }
    else if(currentWidth < STREET) {
        fontSize = 9;
    }
    
    if(currentWidth < STREET) {
        drawSegmentsText(commercialRoads, fontSize);
    }
}

void drawLocalRoadsText() {
    vector<unsigned> localRoads = getLocalRoads();
    int fontSize;
    
    float currentWidth = getWidthOfCurrentWindow();
    if(currentWidth < HOUSE) {
        fontSize = 10;
    }
    else {
        fontSize = 9;
    }
    
    if(currentWidth < LOCAL) {
        drawSegmentsText(localRoads, fontSize);
    }
}
////////////////////////////////////////////////////////////////////////////////
// HELPER FUNCTIONS
////////////////////////////////////////////////////////////////////////////////
// Draw the given vector of street segments

void drawSegments(const vector<unsigned>& segments,
        t_color color, int width) {
    
    // initialize
    unsigned numOfSegments = segments.size();
    setcolor(color);
    setlinewidth(width);
    float windowWidth = getWidthOfCurrentWindow();
    t_point cen;
    float angle;
    
    
    // for every segment
    for(unsigned i = 0; i < numOfSegments; i++) {
        unsigned segmentID = segments[i];
        
        // get segment info
        StreetSegmentInfo segInfo = getStreetSegmentInfo(segmentID);
        unsigned curvePoints = segInfo.curvePointCount;
        
        // No curve points
        if(curvePoints == 0) {
            unsigned start = segInfo.from;
            unsigned end = segInfo.to;
            LatLon startLatLon = getIntersectionPosition(start);
            LatLon endLatLon = getIntersectionPosition(end);
            t_point p1 = convertLatLonToWorld(startLatLon);
            t_point p2 = convertLatLonToWorld(endLatLon);
            drawline(p1, p2);
            
            // draw one way arrow in middle of segment
            if(windowWidth < HOUSE && segInfo.oneWay == true){
                cen = centerOfPoints(p1, p2);
                angle = angleBetweenPoints360(p2, p1) - 90; // weird bug
                t_color temp = getcolor();
                setcolor(LIGHTGREY);
                drawArrow(cen, angle);
                setcolor(temp);
                setlinewidth(width);
            }
        }
        
        
        // Curve points
        else {
            // Draw the start to the first curve point
            unsigned start = segInfo.from;
            LatLon startLatLon = getIntersectionPosition(start);
            LatLon endLatLon = getStreetSegmentCurvePoint(segmentID, 0);
            t_point p1 = convertLatLonToWorld(startLatLon);
            t_point p2 = convertLatLonToWorld(endLatLon);
            drawline(p1, p2);
            
            // Draw the last curve point to the end
            startLatLon = getStreetSegmentCurvePoint(segmentID, curvePoints-1);
            unsigned end = segInfo.to;
            endLatLon = getIntersectionPosition(end);
            p1 = convertLatLonToWorld(startLatLon);
            p2 = convertLatLonToWorld(endLatLon);
            drawline(p1, p2);
            
            t_point arrow1;
            t_point arrow2;
            bool foundOneWay = false;
                // if this curved segment is a oneway, remember to draw
                // an arrow in the middle of the segment afterwards
            
            // Draw everything in between
            for(unsigned curvePointIdx = 0;
                    curvePointIdx < (curvePoints-1);
                    curvePointIdx++) {
                startLatLon = getStreetSegmentCurvePoint(segmentID, curvePointIdx);
                endLatLon = getStreetSegmentCurvePoint(segmentID, curvePointIdx+1);
                p1 = convertLatLonToWorld(startLatLon);
                p2 = convertLatLonToWorld(endLatLon);
                drawline(p1, p2);
                if(windowWidth < HOUSE &&
                        segInfo.oneWay == true &&
                        curvePointIdx == curvePoints/2){
                    arrow1 = p1;
                    arrow2 = p2;
                    foundOneWay = true;
                }
            }
            
            // draw the oneway arrow in the middle of the segment
            // arrow is drawn after all curved pieces are drawn so that
            // part of the curve is not drawn on top of the arrow
            if(foundOneWay == true){
                cen = centerOfPoints(arrow1, arrow2);
                angle = angleBetweenPoints360(arrow2, arrow1) - 90; // weird bug
                t_color temp = getcolor();
                setcolor(LIGHTGREY);
                drawArrow(cen, angle);
                setlinewidth(width);
                setcolor(temp);
            }
        }   
    }
}

void drawSegmentsText(const vector<unsigned>& segments,
        int fontSize) {
    setcolor(BLACK);
    setfontsize(fontSize);
    
    
    unsigned numOfSegments = segments.size();
    for(unsigned i = 0; i < numOfSegments; i++) {
        
        // get numOfCurvePoints
        unsigned segmentID = segments[i];
        StreetSegmentInfo segInfo = getStreetSegmentInfo(segmentID);
        unsigned numOfCurvePoints = segInfo.curvePointCount;
        
        // get streetName
        string streetName = getStreetName(segInfo.streetID);
        if(streetName == "<unknown>"){
            continue;
        }
        
        // get start and end points
        unsigned start = segInfo.from;
        unsigned end = segInfo.to;
        LatLon startLatLon = getIntersectionPosition(start);
        LatLon endLatLon = getIntersectionPosition(end);
        t_point pointStart = convertLatLonToWorld(startLatLon);
        t_point pointEnd = convertLatLonToWorld(endLatLon);
        
        
        // If there are no curve points
        if(numOfCurvePoints == 0) {
            drawTextBetweenPoints(pointStart, pointEnd, streetName);
        }
        
        // If there are 1 or more curve points
        // Only draw text on largest piece of curve
        
        else{
            
            LatLon LatLonA, LatLonB;
            t_point pointA, pointB;
            t_point bestA, bestB;
            float largestDistance = 0;
            float currentDistance = 0;
            
            // check first part of segment
            pointA = pointStart;
            LatLonB = getStreetSegmentCurvePoint(segmentID, 0);
            pointB = convertLatLonToWorld(LatLonB);
            largestDistance = distanceBetweenPoints(pointA, pointB);
            bestA = pointA;
            bestB = pointB;
            
            // check last part of segment
            LatLonA = getStreetSegmentCurvePoint(segmentID, numOfCurvePoints - 1);
            pointA = convertLatLonToWorld(LatLonA);
            pointB = pointEnd;
            currentDistance = distanceBetweenPoints(pointA, pointB);
            if(currentDistance > largestDistance){
                largestDistance = currentDistance;
                bestA = pointA;
                bestB = pointB;
            }
                    
            // check parts in between
            unsigned curveID = 0;
            for (curveID = 0;
                    curveID < (numOfCurvePoints - 1);
                    curveID++) {
                LatLonA = getStreetSegmentCurvePoint(segmentID, curveID);
                pointA = convertLatLonToWorld(LatLonA);
                LatLonB = getStreetSegmentCurvePoint(segmentID, curveID + 1);
                pointB = convertLatLonToWorld(LatLonB);
                currentDistance = distanceBetweenPoints(pointA, pointB);
                
                if(currentDistance > largestDistance){
                    largestDistance = currentDistance;
                    bestA = pointA;
                    bestB = pointB;
                }
            }
            
            // draw text on largest part
            drawTextBetweenPoints(pointA, pointB, streetName);
        }
    }
}


////////////////////////////////////////////////////////////////////////////////
// DRAW POIs
////////////////////////////////////////////////////////////////////////////////

void drawPOIs() {
    // Do not draw anything if we are more zoomed out than house level,
    // as the map would become too cluttered
    float windowWidth = getWidthOfCurrentWindow();
    if(windowWidth > HOUSE)
        return;
    
    unsigned numOfPOIs = getNumberOfPointsOfInterest();
    
    // Diameter 2% of current window. Window width is in km, while world
    // coordinates are in m. Therefore, multiplying my 10 is the same thing
    // as converting km to m and then taking 1% of it for the radius.
    float symbolRadius = SYMBOL_SCALE * windowWidth; 
    
    for(unsigned poiID = 0; poiID < numOfPOIs; poiID++) {
        string name = getPointOfInterestName(poiID);
        LatLon latlon = getPointOfInterestPosition(poiID);
        t_point screenPos = convertLatLonToWorld(latlon);
        string symbol;
        int symbolSize = 0;
        float symbolCutoff, textCutoff;
        t_color symbolColor;
        
        string poiType = typeForPOI(poiID);
        if(poiType == "<unknown>") {
            continue;
        }
        else if(poiType == "automotive") {
            symbolCutoff = LOW_POI_SYMBOL;
            textCutoff = LOW_POI_TEXT;
            symbolColor = RED;           
        }
        else if(poiType == "religious") {
            symbolCutoff = MED_POI_SYMBOL;
            textCutoff = MED_POI_TEXT;
            symbolColor = PURPLE;
        }
        else if(poiType == "education") {
            symbolCutoff = TOP_POI_SYMBOL;
            textCutoff = TOP_POI_TEXT;
            symbolColor = BLUE;
        }
        else if(poiType == "public place") {
            symbolCutoff = TOP_POI_SYMBOL;
            textCutoff = TOP_POI_TEXT;
            symbolColor = DARKGREEN;
        }
        else if(poiType == "arts") {
            symbolCutoff = MED_POI_SYMBOL;
            textCutoff = MED_POI_TEXT;
            symbolColor = PINK;
        }
        else if(poiType == "food") {
            symbolCutoff = LOW_POI_SYMBOL;
            textCutoff = LOW_POI_TEXT;
            symbolColor = ORANGE;
        }
        else if(poiType == "ferry") {
            symbolCutoff = TOP_POI_SYMBOL;
            textCutoff = TOP_POI_TEXT;
            symbolColor = CYAN;
        }
        else if(poiType == "hospital") {
            symbolCutoff = TOP_POI_SYMBOL;
            textCutoff = TOP_POI_TEXT;
            symbolColor = CORAL;
            symbol = "H";
            symbolSize = 12;
        }
        else if(poiType == "pharmacy") {
            symbolCutoff = LOW_POI_SYMBOL;
            textCutoff = LOW_POI_TEXT;
            symbolColor = GREEN;
            symbol = "+";
            symbolSize = 28;
        }
        else if(poiType == "recreation") {
            symbolCutoff = MED_POI_SYMBOL;
            textCutoff = MED_POI_TEXT;
            symbolColor = DARKSLATEBLUE;
            
        }
        else if(poiType == "entertainment") {
            symbolCutoff = LOW_POI_SYMBOL;
            textCutoff = LOW_POI_TEXT;
            symbolColor = MAGENTA;
            symbol = "E!";
            symbolSize = 12;
        }
        else if(poiType == "bank") {
            symbolCutoff = MED_POI_SYMBOL;
            textCutoff = MED_POI_TEXT;
            symbolColor = LIMEGREEN;
            symbol = "$";
            symbolSize = 12;
        }
        else if(poiType == "tourism") {
            symbolCutoff = TOP_POI_SYMBOL;
            textCutoff = TOP_POI_TEXT;
            symbolColor = LIGHTSKYBLUE;
        }
        else if(poiType == "emergency") {
            symbolCutoff = MED_POI_SYMBOL;
            textCutoff = MED_POI_TEXT;
            symbolColor = FIREBRICK;
            symbol = "+";
            symbolSize = 28;
        }
        else if(poiType == "casino") {
            symbolCutoff = TOP_POI_SYMBOL;
            textCutoff = TOP_POI_TEXT;
            symbolColor = DARKKHAKI;
            symbol = "$";
            symbolSize = 12;
        }
        else if(poiType == "parking") {
            symbolCutoff = LOW_POI_SYMBOL;
            textCutoff = LOW_POI_TEXT;
            symbolColor = DARKGREY;
            symbol = "P";
            symbolSize = 12;
        }
        else if(poiType == "lodging") {
            symbolCutoff = MED_POI_SYMBOL;
            textCutoff = MED_POI_TEXT;
            symbolColor = THISTLE;
        }
        else if(poiType == "shop") {
            symbolCutoff = LOW_POI_SYMBOL;
            textCutoff = LOW_POI_TEXT;
            symbolColor = TURQUOISE;
        }
        else if(poiType == "bus station") {
            symbolCutoff = MED_POI_SYMBOL;
            textCutoff = MED_POI_TEXT;
            symbolColor = SADDLEBROWN;
        }
        else {
            continue;
        }
        
        drawPOI(windowWidth, symbolCutoff, textCutoff, symbolColor, symbolRadius,
                poiType, name, symbol, symbolSize, screenPos);
    }
}


void drawPOI(float windowWidth, float symbolCutoff, float textCutoff,
        t_color symbolColor, float symbolRadius, string poiType, 
        string name, string symbol, int symbolSize, t_point pos) {
    // Draw the symbol
    if(windowWidth < symbolCutoff) {
        setcolor(WHITE);
        fillarc(pos, symbolRadius, 0, 360);
        setcolor(symbolColor);
        if (poiType == "religious")
            drawReligious(pos, symbolRadius);
        else if (poiType == "education")
            drawEducation(pos, symbolRadius);
        else if (poiType == "shop")
            drawShop(pos, symbolRadius);
        else if (poiType == "food")
            drawFood(pos, symbolRadius);
        else if (poiType == "bus station")
            drawBusStation(pos, symbolRadius);
        else if (poiType == "automotive")
            drawAutomotive(pos, symbolRadius);
        else if (poiType == "ferry")
            drawFerry(pos, symbolRadius);
        else if (poiType == "tourism") 
            drawTourism(pos, symbolRadius);
        else if (poiType == "arts")
            drawArts(pos, symbolRadius);
        else if (poiType == "lodging")
            drawLodging(pos, symbolRadius);
        else if (poiType ==  "public place" || poiType == "recreation")
            drawMisc(pos, symbolRadius);
        else {
            settextattrs(symbolSize, 0);
            drawtext(pos, symbol, FLT_MAX, FLT_MAX);
        }
    }
    
    // Draw the name
    if(windowWidth < textCutoff) {
        setcolor(BLACK);
        settextattrs(10, 0);
        // 1.5% below the symbol (which has radius 1% of the current window)
        t_point textCenter(pos.x, pos.y-symbolRadius-10*windowWidth);
        drawtext(textCenter, name, FLT_MAX, FLT_MAX);
    }
}

void drawReligious(t_point pos, float symbolRadius) {
    int numOfPoints = 12;
    t_point *points = new t_point[numOfPoints];
    t_point point0, point1, point2, point3, point4, point5, point6, point7,
            point8, point9, point10, point11;
    
    point0.x = pos.x - 0.2 * symbolRadius;
    point0.y = pos.y - 0.9 * symbolRadius;
    points[0] = point0;
    
    point1.x = pos.x - 0.2 * symbolRadius;
    point1.y = pos.y + 0.2 * symbolRadius;
    points[1] = point1;
    
    point2.x = pos.x - 0.9 * symbolRadius;
    point2.y = pos.y + 0.2 * symbolRadius;
    points[2] = point2;
    
    point3.x = pos.x - 0.9 * symbolRadius;
    point3.y = pos.y + 0.5 * symbolRadius;
    points[3] = point3;
    
    point4.x = pos.x - 0.2 * symbolRadius;
    point4.y = pos.y + 0.5 * symbolRadius;
    points[4] = point4;
    
    point5.x = pos.x - 0.2 * symbolRadius;
    point5.y = pos.y + 0.9 * symbolRadius;
    points[5] = point5;
    
    point6.x = pos.x + 0.2 * symbolRadius;
    point6.y = pos.y + 0.9 * symbolRadius;
    points[6] = point6;
    
    point7.x = pos.x + 0.2 * symbolRadius;
    point7.y = pos.y + 0.5 * symbolRadius;
    points[7] = point7;
    
    point8.x = pos.x + 0.9 * symbolRadius;
    point8.y = pos.y + 0.5 * symbolRadius;
    points[8] = point8;
    
    point9.x = pos.x + 0.9 * symbolRadius;
    point9.y = pos.y + 0.2 * symbolRadius;
    points[9] = point9;
    
    point10.x = pos.x + 0.2 * symbolRadius;
    point10.y = pos.y + 0.2 * symbolRadius;
    points[10] = point10;
    
    point11.x = pos.x + 0.2 * symbolRadius;
    point11.y = pos.y - 0.9 * symbolRadius;
    points[11] = point11;
    
    fillpoly(points, numOfPoints);
}

void drawEducation(t_point pos, float symbolRadius) {
    int numOfPoints = 8;
    t_point *points = new t_point[numOfPoints];
    t_point point0, point1, point2, point3, point4, point5, point6, point7;
    
    point0.x = pos.x - 0.8 * symbolRadius;
    point0.y = pos.y - 0.6 * symbolRadius;
    points[0] = point0;
    
    point1.x = pos.x - 0.8 * symbolRadius;
    point1.y = pos.y + 0.4 * symbolRadius;
    points[1] = point1;
    
    point2.x = pos.x - 0.4 * symbolRadius;
    point2.y = pos.y + 0.4 * symbolRadius;
    points[2] = point2;
    
    point3.x = pos.x - 0.4 * symbolRadius;
    point3.y = pos.y + 0.9 * symbolRadius;
    points[3] = point3;
    
    point4.x = pos.x + 0.4 * symbolRadius;
    point4.y = pos.y + 0.9 * symbolRadius;
    points[4] = point4;
    
    point5.x = pos.x + 0.4 * symbolRadius;
    point5.y = pos.y + 0.4 * symbolRadius;
    points[5] = point5;
    
    point6.x = pos.x + 0.8 * symbolRadius;
    point6.y = pos.y + 0.4 * symbolRadius;
    points[6] = point6;
    
    point7.x = pos.x + 0.8 * symbolRadius;
    point7.y = pos.y - 0.6 * symbolRadius;
    points[7] = point7;
    
    fillpoly(points, numOfPoints);
}

void drawShop(t_point pos, float symbolRadius) {
    int numOfPoints = 4;
    float width = 0.6;
    t_point *points = new t_point[numOfPoints];
    t_point point0, point1, point2, point3;
    setlinewidth(width);
    
    point0.x = pos.x - 0.7 * symbolRadius;
    point0.y = pos.y - 0.8 * symbolRadius;
    points[0] = point0;
    
    point1.x = pos.x - 0.7 * symbolRadius;
    point1.y = pos.y;
    points[1] = point1;
    
    point2.x = pos.x + 0.7 * symbolRadius;
    point2.y = pos.y;
    points[2] = point2;
    
    point3.x = pos.x + 0.7 * symbolRadius;
    point3.y = pos.y - 0.8 * symbolRadius;
    points[3] = point3;
    
    fillpoly(points, numOfPoints);
    
    drawline(pos.x - 0.5 * symbolRadius, pos.y, pos.x, pos.y + 0.5 * symbolRadius);
    drawline(pos.x + 0.5 * symbolRadius, pos.y, pos.x, pos.y + 0.5 * symbolRadius);
    
}

void drawFood(t_point pos, float symbolRadius) {
    int numOfPoints = 16;
    t_point *points = new t_point[numOfPoints];
    t_point point0, point1, point2, point3, point4, point5, point6, point7,
            point8, point9, point10, point11, point12, point13, point14, point15;
    
    point0.x = pos.x - 0.2 * symbolRadius; //0.1
    point0.y = pos.y - 0.8 * symbolRadius; 
    points[0] = point0;
    
    point1.x = pos.x - 0.2 * symbolRadius; //0.1
    point1.y = pos.y;
    points[1] = point1;
    
    point2.x = pos.x - 0.6 * symbolRadius; //0.3
    point2.y = pos.y;
    points[2] = point2;
    
    point3.x = pos.x - 0.6 * symbolRadius; //0.3
    point3.y = pos.y + 0.7 * symbolRadius;
    points[3] = point3;
    
    point4.x = pos.x - 0.4 * symbolRadius; //0.2
    point4.y = pos.y + 0.7 * symbolRadius;
    points[4] = point4;
    
    point5.x = pos.x - 0.4 * symbolRadius; //0.2
    point5.y = pos.y + 0.2 * symbolRadius;
    points[5] = point5;
    
    point6.x = pos.x - 0.2 * symbolRadius; //0.1
    point6.y = pos.y + 0.2 * symbolRadius;
    points[6] = point6;
    
    point7.x = pos.x - 0.2 * symbolRadius; //0.1
    point7.y = pos.y + 0.7 * symbolRadius;
    points[7] = point7;
    
    point8.x = pos.x + 0.2 * symbolRadius; //0.1
    point8.y = pos.y + 0.7 * symbolRadius;
    points[8] = point8;
    
    point9.x = pos.x + 0.2 * symbolRadius; //0.1
    point9.y = pos.y + 0.2 * symbolRadius;
    points[9] = point9;
    
    point10.x = pos.x + 0.4 * symbolRadius; //0.2
    point10.y = pos.y + 0.2 * symbolRadius;
    points[10] = point10;
    
    point11.x = pos.x + 0.4 * symbolRadius; //0.2
    point11.y = pos.y + 0.7 * symbolRadius;
    points[11] = point11;

    point12.x = pos.x + 0.6 * symbolRadius; //0.3
    point12.y = pos.y + 0.7 * symbolRadius;
    points[12] = point12;
    
   
    point13.x = pos.x + 0.6 * symbolRadius; //0.3
    point13.y = pos.y;
    points[13] = point13;
    
    point14.x = pos.x + 0.2 * symbolRadius; //0.1
    point14.y = pos.y;
    points[14] = point14;
    
    point15.x = pos.x + 0.2 * symbolRadius; //0.1
    point15.y = pos.y - 0.8 * symbolRadius;
    points[15] = point15;
    
    fillpoly(points, numOfPoints);
}

void drawBusStation(t_point pos, float symbolRadius) {
    int numOfPoints = 12;
    t_point *points = new t_point[numOfPoints];
    t_point point0, point1, point2, point3, point4, point5, point6, point7,
            point8, point9, point10, point11;
    
    point0.x = pos.x - 0.4 * symbolRadius;
    point0.y = pos.y - 0.7 * symbolRadius;
    points[0] = point0;
    
    point1.x = pos.x - 0.4 * symbolRadius;
    point1.y = pos.y - 0.5 * symbolRadius;
    points[1] = point1;
    
    point2.x = pos.x - 0.1 * symbolRadius;
    point2.y = pos.y - 0.5 * symbolRadius;
    points[2] = point2;
    
    point3.x = pos.x - 0.1 * symbolRadius;
    point3.y = pos.y + 0.2 * symbolRadius;
    points[3] = point3;
    
    point4.x = pos.x - 0.3 * symbolRadius;
    point4.y = pos.y + 0.2 * symbolRadius;
    points[4] = point4;
    
    point5.x = pos.x - 0.3 * symbolRadius;
    point5.y = pos.y + 0.8 * symbolRadius;
    points[5] = point5;
    
    point6.x = pos.x + 0.3 * symbolRadius;
    point6.y = pos.y + 0.8 * symbolRadius;
    points[6] = point6;
    
    point7.x = pos.x + 0.3 * symbolRadius;
    point7.y = pos.y + 0.2 * symbolRadius;
    points[7] = point7;
    
    point8.x = pos.x + 0.1 * symbolRadius;
    point8.y = pos.y + 0.2 * symbolRadius;
    points[8] = point8;
    
    point9.x = pos.x + 0.1 * symbolRadius;
    point9.y = pos.y - 0.5 * symbolRadius;
    points[9] = point9;
    
    point10.x = pos.x + 0.4 * symbolRadius;
    point10.y = pos.y - 0.5 * symbolRadius;
    points[10] = point10;
    
    point11.x = pos.x + 0.4 * symbolRadius;
    point11.y = pos.y - 0.7 * symbolRadius;
    points[11] = point11;
    
    fillpoly(points, numOfPoints);
}

void drawAutomotive(t_point pos, float symbolRadius) {
    int numOfPoints = 4;
    t_point *boxPoints = new t_point[numOfPoints];
    t_point *hosePoints = new t_point[numOfPoints];
    t_point point0, point1, point2, point3;
    
    point0.x = pos.x - 0.4 * symbolRadius;
    point0.y = pos.y - 0.7 * symbolRadius;
    boxPoints[0] = point0;
    
    point1.x = pos.x - 0.4 * symbolRadius;
    point1.y = pos.y + 0.2 * symbolRadius;
    boxPoints[1] = point1;
    
    point2.x = pos.x + 0.4 * symbolRadius;
    point2.y = pos.y + 0.2 * symbolRadius;
    boxPoints[2] = point2;
    
    point3.x = pos.x + 0.4 * symbolRadius;
    point3.y = pos.y - 0.7 * symbolRadius;
    boxPoints[3] = point3;
    
    point0.x = pos.x;
    point0.y = pos.y + 0.2 * symbolRadius;
    hosePoints[0] = point0;
    
    point1.x = pos.x + 0.2 * symbolRadius;
    point1.y = pos.y + 0.6 * symbolRadius;
    hosePoints[1] = point1;
    
    point2.x = pos.x + 0.4 * symbolRadius;
    point2.y = pos.y + 0.6 * symbolRadius;
    hosePoints[2] = point2;
    
    point3.x = pos.x + 0.2 * symbolRadius;
    point3.y = pos.y + 0.2 * symbolRadius;
    hosePoints[3] = point3;
    
    fillpoly(boxPoints, numOfPoints);
    fillpoly(hosePoints, numOfPoints);
}

void drawFerry(t_point pos,float symbolRadius) {
    int numOfPoints = 8;
    t_point *points = new t_point[numOfPoints];
    t_point point0, point1, point2, point3, point4, point5, point6, point7;
    
    point0.x = pos.x - 0.5 * symbolRadius;
    point0.y = pos.y - 0.5 * symbolRadius;
    points[0] = point0;
    
    point1.x = pos.x - 0.8 * symbolRadius;
    point1.y = pos.y;
    points[1] = point1;
    
    point2.x = pos.x - 0.25 * symbolRadius;
    point2.y = pos.y;
    points[2] = point2;
    
    point3.x = pos.x - 0.25 * symbolRadius;
    point3.y = pos.y + 0.55 * symbolRadius;
    points[3] = point3;
    
    point4.x = pos.x + 0.25 * symbolRadius;
    point4.y = pos.y + 0.55 * symbolRadius;
    points[4] = point4;
    
    point5.x = pos.x + 0.25 * symbolRadius;
    point5.y = pos.y;
    points[5] = point5;
    
    point6.x = pos.x + 0.8 * symbolRadius;
    point6.y = pos.y;
    points[6] = point6;
    
    point7.x = pos.x + 0.5 * symbolRadius;
    point7.y = pos.y - 0.5 * symbolRadius;
    points[7] = point7;
    
    fillpoly(points, numOfPoints);
}

void drawTourism(t_point pos, float symbolRadius) {
    float radius = 0.37 * symbolRadius;
    float xLeft = pos.x - 0.3 * symbolRadius;
    float xRight = pos.x + 0.3 * symbolRadius;
    
    fillarc(xLeft, pos.y, radius, 0, 360);
    fillarc(xRight, pos.y, radius, 0, 360);
}

void drawArts(t_point pos, float symbolRadius) {
    int numOfPoints = 4;
    t_point *points = new t_point[numOfPoints];
    t_point *brush = new t_point[numOfPoints];
    t_point point0, point1, point2, point3;
    //float xCen, yCen, xRad, yRad;
    
    point0.x = pos.x - 0.2 * symbolRadius;
    point0.y = pos.y - 0.7 * symbolRadius;
    points[0] = point0;
    
    point1.x = pos.x - 0.2 * symbolRadius;
    point1.y = pos.y + 0.2 * symbolRadius;
    points[1] = point1;
    
    point2.x = pos.x + 0.2 * symbolRadius;
    point2.y = pos.y + 0.2 * symbolRadius;
    points[2] = point2;
    
    point3.x = pos.x + 0.2 * symbolRadius;
    point3.y = pos.y - 0.7 * symbolRadius;
    points[3] = point3;
    
    point0.x = pos.x;
    point0.y = pos.y + 0.2 * symbolRadius;
    brush[0] = point0;
    
    point1.x = pos.x - 0.2 * symbolRadius;
    point1.y = pos.y + 0.5 * symbolRadius;
    brush[1] = point1;
    
    point2.x = pos.x;
    point2.y = pos.y + 0.8 * symbolRadius;
    brush[2] = point2;
    
    point3.x = pos.x + 0.2 * symbolRadius;
    point3.y = pos.y + 0.5 * symbolRadius;
    brush[3] = point3;
    
    
    fillpoly(points, numOfPoints);
    fillpoly(brush, numOfPoints);
}

void drawLodging(t_point pos, float symbolRadius) {
    int numOfPoints = 10;
    t_point *points = new t_point[numOfPoints];
    t_point point0, point1, point2, point3, point4, point5, point6, point7,
            point8, point9;
    
    point0.x = pos.x - 0.4 * symbolRadius;
    point0.y = pos.y - 0.1 * symbolRadius;
    points[0] = point0;
    
    point1.x = pos.x - 0.4 * symbolRadius;
    point1.y = pos.y - 0.3 * symbolRadius;
    points[1] = point1;
    
    point2.x = pos.x - 0.6 * symbolRadius;
    point2.y = pos.y - 0.3 * symbolRadius;
    points[2] = point2;
    
    point3.x = pos.x - 0.6 * symbolRadius;
    point3.y = pos.y + 0.5 * symbolRadius;
    points[3] = point3;
    
    point4.x = pos.x - 0.4 * symbolRadius;
    point4.y = pos.y + 0.5 * symbolRadius;
    points[4] = point4;
    
    point5.x = pos.x - 0.4 * symbolRadius;
    point5.y = pos.y + 0.25 * symbolRadius;
    points[5] = point5;
    
    point6.x = pos.x + 0.6 * symbolRadius;
    point6.y = pos.y + 0.25 * symbolRadius;
    points[6] = point6;
    
    point7.x = pos.x + 0.6 * symbolRadius;
    point7.y = pos.y - 0.3 * symbolRadius;
    points[7] = point7;
    
    point8.x = pos.x + 0.4 * symbolRadius;
    point8.y = pos.y - 0.3 * symbolRadius;
    points[8] = point8;
    
    point9.x = pos.x + 0.4 * symbolRadius;
    point9.y = pos.y - 0.1 * symbolRadius;
    points[9] = point9;
    
    fillpoly(points, numOfPoints);
}

void drawMisc(t_point pos, float symbolRadius) {
    int numOfPoints = 4;
    t_point *points = new t_point[numOfPoints];
    t_point point0, point1, point2, point3;
    
    point0.x = pos.x - 0.5 * symbolRadius;
    point0.y = pos.y - 0.5 * symbolRadius;
    points[0] = point0;
    
    point1.x = pos.x - 0.5 * symbolRadius;
    point1.y = pos.y + 0.5 * symbolRadius;
    points[1] = point1;
    
    point2.x = pos.x + 0.5 * symbolRadius;
    point2.y = pos.y + 0.5 * symbolRadius;
    points[2] = point2;
    
    point3.x = pos.x + 0.5 * symbolRadius;
    point3.y = pos.y - 0.5 * symbolRadius;
    points[3] = point3;
    
    fillpoly(points, numOfPoints);
}


////////////////////////////////////////////////////////////////////////////////
// DRAWING HIGHLIGTED FEATURES
////////////////////////////////////////////////////////////////////////////////

void drawHighlightedIntersections() {
    setcolor(HIGHLIGHTED);
    // Radius 1% of current window
    float radius = getWidthOfCurrentWindow() * 7.5;
    
    unsigned numOfHighlightedIntersections = highlightedIntersections.size();
    for(unsigned i = 0; i < numOfHighlightedIntersections; i++) {
        t_point intersectionPos = getPointFromIntersectionID(highlightedIntersections[i]);
        fillarc(intersectionPos, radius, 0, 360);
    }
}

void drawHighlightedStreetSegments() {
    int coreWidth = STREETWIDTH_HIGHLIGHTED;
    
    float currentWidth = getWidthOfCurrentWindow();
    if(currentWidth < HOUSE) {
        if(currentWidth < 0.1)
            coreWidth = 67;
        else
            coreWidth = 1/currentWidth * 8;
    }
   
    drawSegments(highlightedSegments, HIGHLIGHTED, coreWidth);
}

void drawHighlightedPOIs() {
    float windowWidth = getWidthOfCurrentWindow();
    
    unsigned numOfPOIs = highlightedPOIs.size();
    
    // Diameter 2% of current window. Window width is in km, while world
    // coordinates are in m. Therefore, multiplying my 10 is the same thing
    // as converting km to m and then taking 1% of it for the radius.
    float symbolRadius = SYMBOL_SCALE * windowWidth; 
    
    for(unsigned i = 0; i < numOfPOIs; i++) {
        unsigned poiID = highlightedPOIs[i];
        string name = getPointOfInterestName(poiID);
        LatLon latlon = getPointOfInterestPosition(poiID);
        t_point screenPos = convertLatLonToWorld(latlon);
        string symbol;
        int symbolSize = 0;
        float symbolCutoff = FLT_MAX, textCutoff = 0.0;
        t_color symbolColor;
        
        string poiType = typeForPOI(poiID);
        if(poiType == "<unknown>") {
            textCutoff = TOP_POI_TEXT;
            symbolColor = DARKGREY;
        }
        else if(poiType == "automotive") {
            symbolColor = RED;
        }
        else if(poiType == "religious") {
            symbolColor = PURPLE;
        }
        else if(poiType == "education") {
            symbolColor = BLUE;
        }
        else if(poiType == "public place") {
            symbolColor = DARKGREEN;
        }
        else if(poiType == "arts") {
            symbolColor = PINK;
        }
        else if(poiType == "food") {
            symbolColor = ORANGE;
        }
        else if(poiType == "ferry") {
            symbolColor = CYAN;
        }
        else if(poiType == "hospital") {
            symbolColor = CORAL;
            symbol = "H";
            symbolSize = 12;
        }
        else if(poiType == "pharmacy") {
            symbolColor = GREEN;
            symbol = "+";
            symbolSize = 28;
        }
        else if(poiType == "recreation") {
            symbolColor = DARKSLATEBLUE;
        }
        else if(poiType == "entertainment") {
            symbolColor = MAGENTA;
            symbol = "E!";
            symbolSize = 12;
        }
        else if(poiType == "bank") {
            symbolColor = LIMEGREEN;
            symbol = "$";
            symbolSize = 12;
        }
        else if(poiType == "tourism") {
            symbolColor = LIGHTSKYBLUE;
        }
        else if(poiType == "emergency") {
            symbolColor = FIREBRICK;
            symbol = "+";
            symbolSize = 28;
        }
        else if(poiType == "casino") {
            symbolColor = DARKKHAKI;
            symbol = "$";
            symbolSize = 12;
        }
        else if(poiType == "parking") {
            symbolColor = DARKGREY;
            symbol = "P";
            symbolSize = 12;
        }
        else if(poiType == "lodging") {
            symbolColor = THISTLE;
        }
        else if(poiType == "shop") {
            symbolColor = TURQUOISE;
        }
        else if(poiType == "bus station") {
            symbolColor = SADDLEBROWN;
        }
        else {
            continue;
        }
        
        drawPOI(windowWidth, symbolCutoff, textCutoff,
                symbolColor, symbolRadius, poiType, name, 
                symbol, symbolSize, screenPos);
    }
}

// if there is a path loaded,
//      draws from intersection
//      draws to intersection
//      draws path between them
void drawPathFinding(){
    // Radius 1% of current window
    float radius = getWidthOfCurrentWindow() * 7.5;
    t_point intersectionPos;
    
    
    if(!(pathSegments.empty())){
        int coreWidth = STREETWIDTH_HIGHLIGHTED;

        float currentWidth = getWidthOfCurrentWindow();
        if (currentWidth < HOUSE) {
            if (currentWidth < 0.1)
                coreWidth = 67;
            else
                coreWidth = 1 / currentWidth * 8;
        }

        drawSegments(pathSegments, PATH, coreWidth);
    }
    
    if(!(fromIntersection.empty())){
        setcolor(PATH_INTR_START);
        intersectionPos = getPointFromIntersectionID(fromIntersection[0]);
        fillarc(intersectionPos, radius, 0, 360);
    }
    
    
    if(!(toIntersection.empty())){
        setcolor(PATH_INTR_END);
        intersectionPos = getPointFromIntersectionID(toIntersection[0]);
        fillarc(intersectionPos, radius, 0, 360);
    }
}


////////////////////////////////////////////////////////////////////////////////
// USER INTERFACE BUTTONS / INPUT PARSING
////////////////////////////////////////////////////////////////////////////////

// greys out the screen, prompts user to use terminal 
void promptTerminal(){
    t_bound_box worldCoord = getWorldCoordinates();
    float offset = 10000000;
    worldCoord.left() -= offset;
    worldCoord.bottom() -= offset;
    worldCoord.right() += offset;
    worldCoord.top() += offset;
    setcolor(PROMPT);
    fillrect(worldCoord);
    t_bound_box windowCoord = get_visible_world();
    float xdiff = windowCoord.right() - windowCoord.left();
    float ydiff = windowCoord.top() - windowCoord.bottom();
    float xcen = windowCoord.left() + xdiff/2;
    float ycen = windowCoord.bottom() + ydiff/2;
    settextattrs(20,0);
    setcolor(WHITE);
    drawtext(xcen, ycen, "Please enter input at terminal", FLT_MAX, FLT_MAX);
}

// user interaction with the map
void act_on_mouse_click(float x, float y, t_event_buttonPressed event) {
    
    // track whether map needs to be updated or not
    bool updateNeeded = true;
    
    // check which button user clicked
    unsigned int button = event.button;
    
    // get click info
    t_point clickPoint(x,y);
    bool clickedIntersection = false;
    t_point intersectionPoint;
    unsigned intersectionID;
    getClickInfo
        (
            clickPoint,
            clickedIntersection,
            intersectionPoint,
            intersectionID
        );
    
    // MODE: NORMAL
    if(!pathFindingMode){
        
        // only respond to left click
        if(button == CLICK_LEFT){
            highlightedSegments.clear();
            highlightedPOIs.clear();
            
            // if something is already highlighted, clear
            if( 
                    !highlightedIntersections.empty() ||
                    !highlightedSegments.empty() ||
                    !highlightedPOIs.empty() ||
                    !fromIntersection.empty() ||
                    !toIntersection.empty() ||
                    !pathSegments.empty()
                    ){
                highlightedIntersections.clear();
                highlightedSegments.clear();
                highlightedPOIs.clear();
                fromIntersection.clear();
                toIntersection.clear();
                pathSegments.clear();
            }
            
            // else, find intersection that user clicked on
            else{
                if(clickedIntersection){
                    highlightedIntersections.push_back(intersectionID);
                    cout << "Intersection of: \n";
                    outputIntersection(intersectionID);
                    cout << endl << endl;
                }
                else{
                    updateNeeded = false;
                }
            }
        }
    }
    // MODE: PATHFINDING
    else{
        
        // again, only respond to left click
        // first click sets FROM intersection
        // second click sets TO intersection
        // third click clears
        if(button == CLICK_LEFT){
            
            // if highlighted segments are still highlighted
            // (from searching)
            // clear those first
            if( 
                    !highlightedIntersections.empty() ||
                    !highlightedSegments.empty() ||
                    !highlightedPOIs.empty() ||
                    !toIntersection.empty() ||
                    !pathSegments.empty()
                    ){
                highlightedIntersections.clear();
                highlightedSegments.clear();
                highlightedPOIs.clear();
                fromIntersection.clear();
                toIntersection.clear();
                pathSegments.clear();
            }
            
            // if FROM isn't set, set FROM
            else if( (fromIntersection.empty()) ){
                pathSegments.clear();
                
                if(clickedIntersection){
                    fromIntersection.push_back(intersectionID);
                    
                    cout << "Starting Intersection Set:" << endl;
                    outputIntersection(intersectionID);
                    cout << endl << endl;
                }
                else{
                    updateNeeded = false;
                }
            }
            
            // else, FROM is already set, so set TO
            else{
                pathSegments.clear();
                
                if(clickedIntersection){
                    toIntersection.push_back(intersectionID);
                    
                    cout << "Destination Intersection Set:" << endl;
                    outputIntersection(intersectionID);
                    cout << endl << endl;
                }
                else{
                    updateNeeded = false;
                }
            }
            
        }
        
        // update the path
        if(
                !(fromIntersection.empty()) &&
                !(toIntersection.empty()) &&
                pathSegments.empty()){
            
            pathSegments = find_path_between_intersections(fromIntersection[0], toIntersection[0]);
        }
    }
    
    if(updateNeeded) draw_map_a();
}

// sets intersection info based on user click
void getClickInfo
        (
        const t_point &clickPoint,
        bool &clickedIntersection,
        t_point &intersectionPoint,
        unsigned &intersectionID
        ){
    
    // get closest intersectionID
    LatLon clickLatLon = convertWorldToLatLon(clickPoint);
    intersectionID = find_closest_intersection(clickLatLon);
    
    // get intersection world coords
    LatLon intersectionLatLon = getIntersectionPosition(intersectionID);
    intersectionPoint = convertLatLonToWorld(intersectionLatLon);
    
    // check if click was close enough
    float dist = find_distance_between_two_points(clickLatLon, intersectionLatLon);
    if(dist <= CLICK_THRESHOLD)
        clickedIntersection = true;
    else
        clickedIntersection = false;
}


void button_help(void (*drawscreen_ptr) (void)){
    cout
            << endl
            << "-----------------------------------------------------------------------------------" << endl
            << endl
            << "                       Topological Knowledge of Earth Kernel                       " << endl
            << endl
            << "-----------------------------------------------------------------------------------" << endl
            << endl
            << "Search:" << endl
            << "    - Single Element Search:" << endl
            << "          - Enter intersection name, street name, or point of interest name." << endl
            << "          - When searching for an intersection, simply type the two street names" << endl
            << "            separated by the word \"and\"." << endl
            << "          - EXAMPLES:" << endl
            << "                Yonge and Dundas" << endl
            << "                Bay Street" << endl
            << "                Tim Hortons" << endl
            << "    - Directions:" << endl
            << "          - Search directions from intersection to intersection" << endl
            << "            or from intersection to point of interest." << endl
            << "          - Enter the starting intersection intersection (two street names" << endl
            << "            separated by the word \"and\") and either the destination intersection" << endl
            << "            (two street names separated by the word \"and\") or the destination" << endl
            << "            point of interest." << endl
            << "            The start and destination should be separated by the word \"to\"." << endl
            << "          - EXAMPLES:" << endl
            << "                Yonge and Dundas to Bay and College" << endl
            << "                Bay and College to Tim Hortons" << endl
            << "    - AutoComplete:" << endl
            << "          - To use the auto-complete feature, each street name should be in quotes." << endl
            << "          - Use the tab key to auto-complete the word." << endl
            << "          - When there are more than one auto-complete results, the word will not" << endl
            << "            automatically auto-complete. Double-tap the tab key to view all" << endl
            << "            auto-complete results." << endl
            << "          - EXAMPLES:" << endl
            << "                \"Yonge Str                       #<tab>" << endl
            << "                \"Yonge Street\"                   #the street name is auto-completed" << endl
            << "                \"Yonge Street\" and \"Dundas Str   #<tab>" << endl
            << "                \"Yonge Street\" and \"Dundas Street\"" << endl
            << endl
            << endl
            << "Normal Mode:" << endl
            << "    - LEFT CLICK an intersection to get the street names of that intersection." << endl
            << endl
            << endl
            << "Path Mode:" << endl
            << "    - 1) LEFT CLICK to set the starting intersection." << endl
            << "    - 2) LEFT CLICK to set the destination intersection." << endl
            << "         Path and directions will display automatically." << endl
            << "    - 3) LEFT CLICK to clear the selected intersections." << endl
            << endl
            << "-----------------------------------------------------------------------------------" << endl << endl;
    
}

void button_search(void (*drawscreen_ptr) (void)){
    
    // prompt user to use terminal
    promptTerminal();
    
    // clear all highlighted features
    highlightedIntersections.clear();
    highlightedSegments.clear();
    highlightedPOIs.clear();
    fromIntersection.clear();
    toIntersection.clear();
    POInames.clear();
    pathSegments.clear();
    
    string userInput;
    bool intersectionsFound = false;
    bool streetsFound = false;
    bool poisFound = false;
    bool directionsFound = false;
    
    // Set up readline here
    // Use tab for auto completion
    rl_bind_key('\t', rl_complete);
    // Use our function for auto-complete
    rl_attempted_completion_function = command_completion; 
    // Tell readline to handle double and single quotes for us
    rl_completer_quote_characters = strdup("\"\'"); 

    char* buf; //Buffer of line entered by user
    buf = readline("Search: ");
    if(strcmp(buf, "") != 0)
        add_history(buf);
    userInput = buf;
    free(buf);
    
    userInput.erase(userInput.find_last_not_of(" \n\r\t")+1);
    
    auto erasePosition = userInput.find('"');
    if (erasePosition != std::string::npos){
        userInput.erase(remove(userInput.begin(), userInput.end(), '"'), userInput.end());
    }

    if(userInput.find("to") != std::string::npos){
        directionsFound = findDirections(userInput);
    }

    // if a single element was entered (no "to" in input)
    // (if there was a "to" in the input but no directions were found,
    //  look for a single element instead)
    if(!directionsFound){
        intersectionsFound = findIntersections(userInput, highlightedIntersections);
        streetsFound = findStreets(userInput);
        poisFound = findPOIs(userInput, highlightedPOIs);

        if(intersectionsFound) {
            cout << "Intersection(s) found" << endl;
        }
        if(streetsFound) {
            cout << "Street(s) found" << endl;
        }
        if(poisFound) {
            cout << "Points of interest found" << endl;
        }
        
        // Clear highlighted objects if none of that category were found
        // but objects in other categories were found
        if(!intersectionsFound &&
                (streetsFound || poisFound))
            highlightedIntersections.clear();
        if(!streetsFound &&
                (intersectionsFound || poisFound))
            highlightedSegments.clear();
        if(!poisFound &&
                (streetsFound || poisFound))
            highlightedPOIs.clear();

        if(!intersectionsFound &&
                !streetsFound &&
                !poisFound) {
            cout << "No results found" << endl << endl;
        }
        else {
            cout << endl;
            draw_map_a();
        }
    }
    // else, find directions
    else{
        // narrow down input for starting intersection
        if(fromIntersection.size() > 1){
            cout
                    << endl
                    << "Multiple matches for starting location." << endl
                    << "Which intersection did you mean? (enter the corresponding number)" << endl;
            for(unsigned i = 0; i < fromIntersection.size(); i++){
                cout.width(5);
                cout << i+1 << ". ";
                outputIntersection(fromIntersection[i]);
                cout << endl;
            }
            bool valid = false;
            unsigned input;
            do{
                cout << "> " << flush;
                cin >> input;
                if(cin.fail()){
                    cin.clear(); // clear fail bit
                    cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // skip over bad input
                    cout << "Input was not a valid number. Please enter a valid number." << endl;
                    continue;
                }
                if((input <= 0) || (input > fromIntersection.size())){
                    cout << "Input was not a valid number. Please enter a valid number." << endl;
                    continue;
                }
                valid = true;
            }while(!valid);
            unsigned correctIntersection = fromIntersection[input-1];
            fromIntersection.clear();
            fromIntersection.push_back(correctIntersection);
        }
        
        // sort list of POI ids into list of unique names
        for(unsigned i = 0; i < destPOI.size(); i++){
            string currentName = getPointOfInterestName(destPOI[i]);
            POInames.push_back(currentName);
        }
        removeDoubles(POInames);
        sort(POInames.begin(), POInames.end());
        
        
        // narrow down input for destination intersection
        if(
                toIntersection.size() > 1 ||
                POInames.size() > 1 ||
                (toIntersection.size() == 1 && POInames.size() == 1)
                ){
            cout
                    << endl
                    << "Multiple matches for destination." << endl
                    << "Which destination did you mean? (enter the corresponding number)" << endl;
            for(unsigned i = 0; i < toIntersection.size(); i++){
                cout.width(5);
                cout << i+1 << ". ";
                outputIntersection(toIntersection[i]);
                cout << endl;
            }
            for(unsigned i = 0; i < POInames.size(); i++){
                cout.width(5);
                cout << toIntersection.size()+i+1 << ". " << POInames[i+1];
                cout << endl;
            }
            bool valid = false;
            unsigned input;
            do{
                cout << "> " << flush;
                cin >> input;
                if(cin.fail()){
                    cin.clear(); // clear fail bit
                    cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // skip over bad input
                    cout << "Input was not a valid number. Please enter a valid number." << endl;
                    continue;
                }
                if((input <= 0) || (input > toIntersection.size() + POInames.size())){
                    cout << "Input was not a valid number. Please enter a valid number." << endl;
                    continue;
                }
                valid = true;
            }while(!valid);
            if(input <= toIntersection.size()){
                unsigned correctIntersection = toIntersection[input-1];
                toIntersection.clear();
                POInames.clear();
                toIntersection.push_back(correctIntersection);
            }
            else{
                string correctName = POInames[input - 1 - toIntersection.size()];
                toIntersection.clear();
                POInames.clear();
                POInames.push_back(correctName);
            }
        }
        // find the path from start to either:
        //      destination intersection
        //      OR
        //      closest POI found
        if(!toIntersection.empty()){
            pathSegments = find_path_between_intersections(fromIntersection[0], toIntersection[0]);
            draw_map_a();
        }
        else{
            pathSegments = find_path_to_point_of_interest(fromIntersection[0], POInames[0]);
            
            // if path size is zero, then the starting point is the finish point
            if (pathSegments.size() == 0)
                toIntersection.push_back(fromIntersection[0]);
            // if path size is one, then the finish point is the point
            // of the street segment that is not stored in fromIntersection
            else if (pathSegments.size() == 1) {
                StreetSegmentInfo segment;
                segment = getStreetSegmentInfo(pathSegments[pathSegments.size()-1]);
                if (segment.from == fromIntersection[0])
                    toIntersection.push_back(segment.to);
                else
                    toIntersection.push_back(segment.from);
            }
            // otherwise, look at the last two segments in the path
            // and compare the end points of each segment to figure out 
            // the end point of the path
            else {
                StreetSegmentInfo segment1, segment2;
                unsigned seg1From, seg1To, seg2From, seg2To;
                segment1 = getStreetSegmentInfo(pathSegments[pathSegments.size()-1]);
                segment2 = getStreetSegmentInfo(pathSegments[pathSegments.size()-2]);
                seg1From = segment1.from;
                seg1To = segment1.to;
                seg2From = segment2.from;
                seg2To = segment2.to;
                
                if (seg1From!= seg2To && seg1From != seg2From)
                    toIntersection.push_back(seg1From);
                else if (seg1To != seg2To && seg1To != seg2From)
                    toIntersection.push_back(seg1To);
            }
            POInames.clear();
            draw_map_a();
        }
    }
}


// separates input into "from" and "to" strings
// fills found intersections for each
// fills destination POIs if found
bool findDirections(string searchField){
    string from, to;
    bool toFlag = false;
    
    stringstream ss(searchField);
    while(!ss.eof()) {
        string word;
        ss >> word;
        if (word == "to" || word == "To") {
            toFlag = true;
            continue;
        }
        if (!toFlag)
            from += word + " ";
        else
            to += word + " ";
    }
    
    if (from.size() != 0)
        from.pop_back();
    if (to.size() != 0) {
        to.pop_back();
    }
    
    findIntersections(from, fromIntersection);
    findIntersections(to, toIntersection);
    findPOIs(to, destPOI);
    
    if(
            fromIntersection.empty() ||
            (toIntersection.empty() && destPOI.empty())
            ){
        return false;
    }
    return true;
}

//Given the stem 'stem_text' perform auto completion. 
//It returns an array of strings that are potential completions
//
//Note:
// 'start' and 'end' denote the location of 'stem_text' in the global
// 'rl_line_buffer' variable which contains the users current input line.
// If you need more context information to determine how to complete 'stem_text'
// you could look at 'rl_line_buffer'.
char** command_completion(const char* stem_text, int start, int end) {
    char ** matches = NULL;
    //if(start != 0) {
        //Only generate completions if stem_text' 
        //is not the first thing in the buffer
        matches = rl_completion_matches(stem_text, name_generator);
    //}

    return matches;
}

char* name_generator(const char* stem_text, int state) {
    //Static here means a variable's value persists across function invocations
    static int count;

    if(state == 0) { 
        //We initialize the count the first time we are called
        //with this stem_text
        count = -1;
    }

    int text_len = strlen(stem_text);
    vector <const char*> allNames = getAllNames();
    //Search through intersection_names until we find a match
    while(count < (int) allNames.size()-1) {
        count++;
        if(strncmp(allNames[count], stem_text, text_len) == 0) {
            //Must return a duplicate, Readline will handle
            //freeing this string itself.
            return strdup(allNames[count]);
        }
    }

    //No more matches
    return NULL;
}

void button_pathMode(void (*drawscreen_ptr) (void)){
        
    highlightedIntersections.clear();
    highlightedSegments.clear();
    highlightedPOIs.clear();
    
    fromIntersection.clear();
    toIntersection.clear();
    pathSegments.clear();
    
    if(!pathFindingMode){
        change_button_text("Path Mode", "Normal Mode");
        cout << endl << "Switched to Path Mode" << endl << endl;
    }
    else{
        change_button_text("Normal Mode", "Path Mode");
        cout << endl << "Switched to Normal Mode" << endl << endl; 
    }
    
    pathFindingMode = !pathFindingMode;
    draw_map_a();
}

bool findIntersections(string searchField, vector<unsigned> &intersections) {
    string street1, street2;
    bool andFlag = false;
    vector<unsigned> foundIntersections;
    
    // parse given searchField into two streets
    // split by the word "and"
    stringstream ss(searchField);
    while(!ss.eof()) {
        string word;
        ss >> word;
        if (word == "and" || word == "And") {
            andFlag = true;
            continue;
        }
        if (!andFlag)
            street1 += word + " ";
        else
            street2 += word + " ";
    }
    
    if (street1.size() != 0)
        street1.pop_back();
    if (street2.size() != 0)
        street2.pop_back();
    // look for intersections with the street names
    foundIntersections = find_intersection_ids_from_street_names(street1, street2);
    
    string street1Upper = capitalizeWords(street1);
    string street2Upper = capitalizeWords(street2);
    if(!(street1 == street1Upper && street2 == street2Upper)) {
        vector<unsigned> upperIntersections = 
            find_intersection_ids_from_street_names(street1Upper, street2Upper);
        foundIntersections.insert(foundIntersections.end(), 
                upperIntersections.begin(), upperIntersections.end());
    }
    
    // If nothing has been found, try searching
    // the names to see if the search field string
    // is part of a longer name
    if(foundIntersections.empty()) {
        vector<unsigned> partMatches = searchIntersectionByPartsOfName(street1, street2);
        foundIntersections.insert(foundIntersections.end(),
                partMatches.begin(), partMatches.end());
    }
    
    if (!foundIntersections.empty()){
        intersections.clear();
        
        intersections = foundIntersections;
        return true;
    }
    
    return false;
}

// helper function for outputting intersection given 
// an intersection id
void outputIntersection(unsigned intersectionID){
    vector<string> streetNames = find_intersection_street_names(intersectionID);
    removeDoubles(streetNames);

    for (unsigned i = 0; i < streetNames.size(); i++) {
        cout << streetNames[i];
        if(i != (streetNames.size()-1))
            cout << " and ";
    }
}

// helper function for getting the t_point position
// from intersection id
t_point getPointFromIntersectionID(unsigned intersectionID) {
    LatLon intersectionLatLon = getIntersectionPosition(intersectionID);
    t_point intersectionPos = convertLatLonToWorld(intersectionLatLon);
    return intersectionPos;
}

bool findStreets(string searchField) {
    vector<unsigned> foundStreets = find_street_ids_from_name(searchField);
    
    string streetUpper = capitalizeWords(searchField);
    if(streetUpper != searchField) {
        vector<unsigned> upperStreets = find_street_ids_from_name(streetUpper);
        foundStreets.insert(foundStreets.begin(),
                upperStreets.begin(), upperStreets.end());
    }
    
    // If nothing has been found, try searching
    // the names to see if the search field string
    // is part of a longer name
    if(foundStreets.empty()) {
        vector<unsigned> partMatches = searchStreetByPartOfName(searchField);
        foundStreets.insert(foundStreets.end(),
                partMatches.begin(), partMatches.end());
    }
    
    if(!foundStreets.empty()) {
        highlightedSegments.clear();
        
        for(unsigned i = 0; i < foundStreets.size(); i++) {
            unsigned streetID = foundStreets[i];
            vector<unsigned> segments = find_street_street_segments(streetID);
            highlightedSegments.insert(highlightedSegments.end(),
                    segments.begin(), segments.end());
        }
        
        return true;
    }
    
    return false;
}

bool findPOIs(string searchField, vector<unsigned> &POIs) {
    vector<unsigned> foundPOIs;
    
    // Search by category
    string tag = tagForAlias(searchField);
    if(tag != "<unknown>") {
        unsigned numOfPOIs = getNumberOfPointsOfInterest();
        for(unsigned poiID = 0; poiID < numOfPOIs; poiID++) {
            if(doesContainTag(poiID, tag))
                foundPOIs.push_back(poiID);
        }
    }
    
    // Search by name
    vector<unsigned> matchingNames = poiIDsFromName(searchField);
    foundPOIs.insert(foundPOIs.end(), 
            matchingNames.begin(), matchingNames.end());
    
    string upperName = capitalizeWords(searchField);
    if(upperName != searchField) {
        vector<unsigned> upperMatches = poiIDsFromName(upperName);
        foundPOIs.insert(foundPOIs.end(), 
            upperMatches.begin(), upperMatches.end());
    }
    
    // If nothing has been found, try searching
    // the names to see if the search field string
    // is part of a longer name
    if(foundPOIs.empty()) {
        vector<unsigned> partMatches = searchPOIByPartOfName(searchField);
        foundPOIs.insert(foundPOIs.end(),
                partMatches.begin(), partMatches.end());
    }
    
    if(!foundPOIs.empty()) {
        POIs = foundPOIs;
        
        return true;
    }
    
    return false;
}


////////////////////////////////////////////////////////////////////////////////
// MISCELLANEOUS DRAWING
////////////////////////////////////////////////////////////////////////////////

t_point convertLatLonToWorld(LatLon point) {
    double lat = point.lat * DEG_TO_RAD;
    double lon = point.lon * DEG_TO_RAD;
    
    double x = lon*cos(avgLatRad) * EARTH_RADIUS_IN_METERS;
    double y = lat * EARTH_RADIUS_IN_METERS;
    
    t_point convertedPoint(x, y);
    return convertedPoint;
}

LatLon convertWorldToLatLon(t_point point) {
    double x = point.x;
    double y = point.y;
    
    double lon = x / EARTH_RADIUS_IN_METERS / cos(avgLatRad);
    double lat = y / EARTH_RADIUS_IN_METERS;
    
    LatLon convertedPoint(lat/DEG_TO_RAD, lon/DEG_TO_RAD);
    return convertedPoint;
}

// Returns the width of the current easyGL window in km
float getWidthOfCurrentWindow() {
    t_bound_box currentView = get_visible_world();
    float currentViewWidth = currentView.get_width();
    
    float kilometersOfView = currentViewWidth / 1000.0;
    return kilometersOfView;
}

t_bound_box getWorldCoordinates() {
    unsigned numOfIntersections = getNumberOfIntersections();
    
    LatLon pos = getIntersectionPosition(0);
    float minLat = pos.lat;
    float maxLat = pos.lat;
    float minLon = pos.lon;
    float maxLon = pos.lon;
    
    for(unsigned i = 1; i < numOfIntersections; i++) {
        pos = getIntersectionPosition(i);
        if(pos.lat < minLat)
            minLat = pos.lat;
        if(pos.lat > maxLat)
            maxLat = pos.lat;
        if(pos.lon < minLon)
            minLon = pos.lon;
        if(pos.lon > maxLon)
            maxLon = pos.lon;
    }
    
    avgLatRad = (minLat + maxLat) / 2.0 * DEG_TO_RAD;
    
    t_point bottomLeft = convertLatLonToWorld(LatLon(minLat, minLon));
    t_point topRight = convertLatLonToWorld(LatLon(maxLat, maxLon));
    
    t_bound_box worldCoord(bottomLeft, topRight);
    return worldCoord;
}

// Returns the angle (in degrees) between two points
float angleBetweenPoints(t_point p1, t_point p2) {
    float x = p1.x - p2.x;
    float y = p1.y - p2.y;
    float rad = atan(y/x);
    float deg = rad / DEG_TO_RAD;
    return deg;
}

// A helper function for removing double entries in a vector
void removeDoubles(vector<string>& vect) {
    for(auto i = vect.begin();
            i < vect.end();
            i++)
    {
        for(auto j = i;
                j < vect.end();
                j++)
        {
            if(*i == *j && i != j)
                j = vect.erase(j) - 1;
        }
    }
}

// assumes font size and color are already set
void drawTextBetweenPoints(t_point p1, t_point p2, string name){
    
    // some adjacent points are on top of each other for some reason
    // if this happens, do nothing
    if(p1.x == p2.x && p1.y == p2.y)
        return;
    
    // set text location
    t_point center((p1.x+p2.x)/2.0, (p1.y+p2.y)/2.0);
    
    // set text angle
    float textAngle = angleBetweenPoints(p1, p2);
    settextrotation(textAngle);
    
    // set text bound box
    float bound = distanceBetweenPoints(p1, p2);
    float xbound = bound * abs(cos(textAngle*DEG_TO_RAD));
    float ybound = bound * abs(sin(textAngle*DEG_TO_RAD));
    
    // Create minimum bound to make sure height of text fits.
    // For near horizontal or near vertical roads,
    // there must be minimum bounds so text will still draw
    // textAngle is parallel to text
    // textAngle + 90 is perpendicular to text, so make sure the bound box
    // fits the height of text at this angle
    float windowWidth = get_visible_world().get_width();
    float textHeightOffset = windowWidth / 60; // trial and error constant
    float minxbound = textHeightOffset * abs(sin(textAngle*DEG_TO_RAD));
    float minybound = textHeightOffset * abs(cos(textAngle*DEG_TO_RAD));
    if(xbound < minxbound) xbound = minxbound;
    if(ybound < minybound) ybound = minybound;
    
    // draw text
    drawtext(center, name, xbound, ybound);
}

float distanceBetweenPoints(t_point p1, t_point p2){
    return sqrt((p1.x - p2.x)*(p1.x - p2.x) + (p1.y - p2.y)*(p1.y - p2.y));
}

void drawArrow (t_point cen, float angle){
    float currentWidth = getWidthOfCurrentWindow();
    float core = (1/currentWidth/8 + 4)/1.2; // trial and error constant
    
    float xcen = cen.x;
    float ycen = cen.y;
    
    // create the points of the arrow
    // (top, left, right, stem)
    t_point top(xcen, ycen + core/2);
    t_point l(xcen - core/4, ycen);
    t_point r(xcen + core/4, ycen);
    t_point s(xcen, ycen - core);
    
    // rotate the points accordingly
    rotatePointAboutCen(cen, angle, top);
    rotatePointAboutCen(cen, angle, l);
    rotatePointAboutCen(cen, angle, r);
    rotatePointAboutCen(cen, angle, s);
    
    // draw the lines that make the arrows
    setcolor(LIGHTGREY);
    setlinewidth(2);
    drawline(top, l);
    drawline(top, r);
    drawline(top, s);
}

void rotatePointAboutCen(t_point cen, float rotAngle, t_point &point){
    
    float s = sin(rotAngle*DEG_TO_RAD);
    float c = cos(rotAngle*DEG_TO_RAD);
    
    // translate point from center
    point.x -= cen.x;
    point.y -= cen.y;
    
    // rotate the point
    float xnew = point.x * c - point.y * s;
    float ynew = point.x * s + point.y * c;
    
    // translate back to center
    point.x = xnew + cen.x;
    point.y = ynew + cen.y;
}

// same as angleBetweenPoints, but return an angle from point 1 to point 2
// range is [0,359]] rather than [-90, 90]
float angleBetweenPoints360(t_point p1, t_point p2) {
    float x = p1.x - p2.x;
    float y = p1.y - p2.y;
    float rad = atan(y/x);
    float deg = rad / DEG_TO_RAD;
    
    // convert angle from range [-90,90] to [0,360]
    if(x > 0 && y == 0)         deg = 0;
    else if(x < 0 && y == 0)    deg = 180;
    else if(x == 0 && y > 0)    deg = 90;
    else if(x == 0 && y < 0)    deg = 270;
    else if(x > 0 && y > 0)     ;// do nothing
    else if(x < 0 && y > 0)     deg += 180;
    else if(x < 0 && y < 0)     deg += 180;
    else if(x > 0 && y < 0)     deg += 360;
    
    return deg;
}

t_point centerOfPoints(t_point p1, t_point p2){
    t_point cen((p1.x+p2.x)/2.0, (p1.y+p2.y)/2.0);
    return cen;
}
