#include "m4.h"
#include "m3.h"
#include <set>
#include <chrono>
#include <math.h>
using namespace std;
unsigned wideChanges = 0;
unsigned narrowChanges = 0;
bool DEBUG = false;


#define TIME_LIMIT 30
#define TIME_LIMIT_RATIO 0.96
#define LOW_GREEDY 1
#define MED_GREEDY 2
#define HIGH_GREEDY 4
#define VERY_HIGH_GREEDY 5
#define LOW_TWOOP 5
#define HIGH_TWOOP 6
#define LOW_ANNEALING 5
#define HIGH_ANNEALING 6
#define LOCAL_MOVE 3
#define START_TEMP 10

// HELPER FUNCTIONS
void optimizerGreedy(chrono::high_resolution_clock::time_point startTime, CourierPath& courierPath, unsigned strength);
void optimizerTwoOp(chrono::high_resolution_clock::time_point startTime, CourierPath& courierPath, unsigned maxStrength);
void optimizerAnnealing(chrono::high_resolution_clock::time_point startTime, CourierPath& courierPath, unsigned maxSrength);
void perturbWithTemp(CourierPath& path, double temp, unsigned maxStrength);
double annealingAggressiveness(double deltaCost, double temp);
double reduceTemperature(double ratioLeft);
void perturbWithRatio(CourierPath& path, double ratio, unsigned strength);

// DEBUG
void printVector(vector<unsigned> vect) {
    cout << "{";
    for(unsigned i=0; i<vect.size(); i++) {
        cout << vect[i];
        if(i != vect.size()-1)
            cout << ", ";
    }
    cout << "}" << endl << endl;
}

bool simpleIsValid(const vector<DeliveryInfo>& delInfo, vector<unsigned> path) {
    for(unsigned i=0; i<delInfo.size(); i++) {
        unsigned pickup = delInfo[i].pickUp;
        unsigned dropoff = delInfo[i].dropOff;
        
        bool foundPickup = false;
        for(unsigned j = 1; j < path.size()-1; j++) {
            if(path[j] == pickup)
                foundPickup = true;
            if(path[j] == dropoff) {
                if(!foundPickup)
                    return false;
            }
        }
    }
    
    return true;
}



void printCostMap(costMap map) {
    cout << "{" << endl;
    for(auto i = map.begin(); i != map.end(); i++) {
        unsigned inter1 = i->first;
        unordered_map<unsigned, double>& subMap = i->second;
        for(auto j = subMap.begin(); j != subMap.end(); j++) {
            unsigned inter2 = j->first;
            double distance = j->second;
            cout << "\t" << inter1 << "\t" << inter2 << "\t" << distance << endl;
        }
    }
    cout << "}" << endl << endl;
}

std::vector<unsigned> traveling_courier(const std::vector<DeliveryInfo>& deliveries, const std::vector<unsigned>& depots) {
    
    // Initialize timing variables for iterating within time limit
    auto startTime = chrono::high_resolution_clock::now();
    auto currentTime = chrono::high_resolution_clock::now();
    auto wallClock = chrono::duration_cast<chrono::duration<double>> (currentTime - startTime);
    
    // Precompute the shortest distance between each delivery location/depot,
    // as well as the closest depot to each delivery intersection
    Proximities proximities(deliveries, depots);
    // The before/after conditions for the deliveries
    DeliveryConditions conditions(deliveries);
    
    if(DEBUG){
        currentTime = chrono::high_resolution_clock::now();
        wallClock = chrono::duration_cast<chrono::duration<double>> (currentTime - startTime);
        cout << endl << "Djikstra Time: " << wallClock.count() << endl;
    }
    
    // Create an array of paths for multi threading
    vector<CourierPath> paths;
    for(unsigned i=0; i<NUM_THREADS; i++){
        unsigned start;
        if(i < deliveries.size())
            start = deliveries[i].pickUp;
        else
            start = deliveries[0].pickUp;
        CourierPath startPath(&conditions, &proximities, start);
        paths.push_back(startPath);
    }
    
    if(DEBUG){
        currentTime = chrono::high_resolution_clock::now();
        wallClock = chrono::duration_cast<chrono::duration<double>> (currentTime - startTime);
        cout << "Initial Path Time: " << wallClock.count() << endl;
    }
    
    // Create threads themselves
    thread threads[NUM_THREADS-1]; // -1 accounts for the main thread
    
    
    // Simulated annealing, 2-opt, and random swapping with multi threading.
    // Each thread makes a change to best path with different aggressiveness
    
    // Run optimizer on additional threads
    threads[0] = thread(optimizerTwoOp, startTime, ref(paths[1]), HIGH_TWOOP);
    threads[1] = thread(optimizerGreedy, startTime, ref(paths[2]), LOW_GREEDY);
    threads[2] = thread(optimizerGreedy, startTime, ref(paths[3]), MED_GREEDY);
    threads[3] = thread(optimizerGreedy, startTime, ref(paths[4]), HIGH_GREEDY);
    threads[4] = thread(optimizerGreedy, startTime, ref(paths[5]), VERY_HIGH_GREEDY);
    threads[5] = thread(optimizerAnnealing, startTime, ref(paths[6]), LOW_ANNEALING);
    threads[6] = thread(optimizerAnnealing, startTime, ref(paths[7]), HIGH_ANNEALING);
    // Run optimizer on main thread
    optimizerTwoOp(startTime, ref(paths[0]), LOW_TWOOP);
    
    // Wait for all threads to finish
    for(unsigned i=0; i<NUM_THREADS-1; i++){
        threads[i].join();
    }
    
    unsigned indexOfBest = 0;
    double bestTime = DBL_MAX;
    // Find best result
    for(unsigned i=0; i<NUM_THREADS; i++){
        
        if(DEBUG){
            cout << "Thread " << i << ": " << paths[i].getDistanceCost() << endl;
        }
        if(paths[i].getDistanceCost() < bestTime) {
            indexOfBest = i;
            bestTime = paths[i].getDistanceCost();
        }
    }
    CourierPath bestPath = paths[indexOfBest];
    
    
    // Construct the final path of street segments
    vector<unsigned> pathOfDestinations = bestPath.getPath();
    vector<unsigned> finalPath;
    unsigned numOfDestinations = pathOfDestinations.size();
    for(unsigned i = 0; i < (numOfDestinations-1); i++) {
        unsigned start = pathOfDestinations[i];
        unsigned end = pathOfDestinations[i+1];
        
        // Check if the path is connected. If not, return an empty vector
        if(FLT_MAX == proximities.costBetween(start, end))
            return vector<unsigned>(0);
        
        vector<unsigned> partialPath = find_path_between_intersections(start, end);
        finalPath.insert(finalPath.end(), partialPath.begin(), partialPath.end());
    }
    
    if(DEBUG){
        currentTime = chrono::high_resolution_clock::now();
        wallClock = chrono::duration_cast<chrono::duration<double>> (currentTime - startTime);
        cout << "Total Time: " << wallClock.count() << endl;
    }
    
    return finalPath;
}



void optimizerGreedy(chrono::high_resolution_clock::time_point startTime, CourierPath& courierPath, unsigned strength){
    auto currentTime = chrono::high_resolution_clock::now();
    auto wallClock = chrono::duration_cast<chrono::duration<double>> (currentTime - startTime);
    
    do {
        CourierPath testPath = courierPath;
        testPath.sectionChange(strength);
        
        if (testPath.getDistanceCost() < courierPath.getDistanceCost())
            courierPath = testPath;
        
        // Keep optimizing until within X% of time limit
        currentTime = chrono::high_resolution_clock::now();
        wallClock = chrono::duration_cast<chrono::duration<double>> (currentTime - startTime);
    } while (wallClock.count() < TIME_LIMIT_RATIO * TIME_LIMIT);
}

void optimizerTwoOp(chrono::high_resolution_clock::time_point startTime, CourierPath& courierPath, unsigned strength) {
    auto currentTime = chrono::high_resolution_clock::now();
    auto wallClock = chrono::duration_cast<chrono::duration<double>> (currentTime - startTime);
    double ratioLeft = 1;
    
    do {
        CourierPath testPath = courierPath;
        perturbWithRatio(testPath, ratioLeft, strength);
        if(testPath.getDistanceCost() < courierPath.getDistanceCost())
            courierPath = testPath;
        
        // Keep optimizing until within X% of time limit
        currentTime = chrono::high_resolution_clock::now();
        wallClock = chrono::duration_cast<chrono::duration<double>> (currentTime - startTime);
        
        double totalTimeAllowed = TIME_LIMIT_RATIO * TIME_LIMIT;
        double timeLeft = totalTimeAllowed - wallClock.count();
        ratioLeft = timeLeft / totalTimeAllowed;
        if(ratioLeft < 0) ratioLeft = 0;
    } while (wallClock.count() < TIME_LIMIT_RATIO * TIME_LIMIT); 
}

void optimizerAnnealing(chrono::high_resolution_clock::time_point startTime, CourierPath& courierPath, unsigned maxStrength) {
    auto currentTime = chrono::high_resolution_clock::now();
    auto wallClock = chrono::duration_cast<chrono::duration<double>> (currentTime - startTime);
    
    double temperature = START_TEMP;
    CourierPath currentPath = courierPath;
    
    do {
        CourierPath testPath = currentPath;
        
        perturbWithTemp(testPath, temperature, maxStrength);
        double changeInCost = testPath.getDistanceCost() - currentPath.getDistanceCost();
        float randZeroToOne = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
        
        if(changeInCost < 0) {
            currentPath = testPath;
            if(currentPath.getDistanceCost() < courierPath.getDistanceCost())
                courierPath = currentPath;
        }
        else if(randZeroToOne < annealingAggressiveness(changeInCost, temperature)) {
            wideChanges++;
            currentPath = testPath;
        }
        
        narrowChanges++;
        // Keep optimizing until within X% of time limit
        currentTime = chrono::high_resolution_clock::now();
        wallClock = chrono::duration_cast<chrono::duration<double>> (currentTime - startTime);
        
        double totalTimeAllowed = TIME_LIMIT_RATIO * TIME_LIMIT;
        double timeLeft = totalTimeAllowed - wallClock.count();
        double ratioLeft = timeLeft / totalTimeAllowed;
        if(ratioLeft < 0) ratioLeft = 0;
        temperature = reduceTemperature(ratioLeft);
        
    } while (wallClock.count() < TIME_LIMIT_RATIO * TIME_LIMIT);
}

void perturbWithTemp(CourierPath& path, double temp, unsigned maxStrength) {
    double decreaseFactor = temp / START_TEMP;
    int initialStrength = decreaseFactor * maxStrength + 1;
    int maxSectionChange = initialStrength - 1;
    int strengthChange = rand() % (maxSectionChange + 1);
    if(rand() % 2)
        strengthChange *= -1;
    unsigned strength = initialStrength + strengthChange;
    
    if(strength <= LOCAL_MOVE) {  // Small perturbation
        unsigned twoOptSize = (rand() % 4) + 2; // 2 to 5
        path.reverseSection(twoOptSize);
    }
    else {  // Large perturbation
        if(rand() % 2)
            path.sectionChange(strength);
        else
            path.swapReverseSection(strength);
    }
}

double annealingAggressiveness(double deltaCost, double temp) {
    double aggressiveness = exp(-1 * (deltaCost/temp));
    
    return aggressiveness;
}

double reduceTemperature(double ratioLeft) {
    double newTemperature = ratioLeft * START_TEMP;
    return newTemperature;
}

void perturbWithRatio(CourierPath& path, double ratio, unsigned maxStrength) {
    int initialStrength = ratio * maxStrength + 1;
    int maxSectionChange = initialStrength - 1;
    int strengthChange = rand() % (maxSectionChange + 1);
    if(rand() % 2)
        strengthChange *= -1;
    unsigned strength = initialStrength + strengthChange;
    
    if(strength <= LOCAL_MOVE) {  // Small perturbation
        unsigned twoOptSize = (rand() % 4) + 2; // 2 to 5
        path.reverseSection(twoOptSize);
    }
    else {  // Large perturbation
        if(rand() % 2)
            path.sectionChange(strength);
        else
            path.swapReverseSection(strength);
    }
}