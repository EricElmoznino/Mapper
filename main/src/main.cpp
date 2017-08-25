#include "StreetsDatabaseAPI.h"
#include "OSMDatabaseAPI.h"
#include "mapping_constants.h"
#include "m1.h"
#include "m2.h"
#include <string>

using namespace std;

int main() {
    bool loadSuccess;
    
    // map file names for loading
    const unsigned numOfMaps = 7;
    const string CAIRO      = "cairo_egypt";
    const string HAMILTON   = "hamilton_canada";
    const string LONDON     = "london_england";
    const string MOSCOW     = "moscow";
    const string NEWYORK    = "newyork";
    const string ST_HELENA  = "saint_helena";
    const string TORONTO    = "toronto";
    
    // map parser
    do {
        cout 
                << endl
                << "-----------------------------------------------------------------------------------" << endl
                << endl
                << "                       Topological Knowledge of Earth Kernel                       " << endl
                << endl
                << "-----------------------------------------------------------------------------------" << endl
                << endl
                << "                               Welcome to TopKEK" << endl << endl
                << "Which Map did you want to load? (enter the corresponding number)" << endl;
        
        unsigned padlength = 5;
        cout.width(5);
        cout << "1." << " Cairo, Egypt" << endl;
        cout.width(padlength);
        cout << "2." << " Hamilton, Canada" << endl;
        cout.width(padlength);
        cout << "3." << " London, England" << endl;
        cout.width(padlength);
        cout << "4." << " Moscow, Russia" << endl;
        cout.width(padlength);
        cout << "5." << " New York, United States" << endl;
        cout.width(padlength);
        cout << "6." << " Saint Helena" << endl;
        cout.width(padlength);
        cout << "7." << " Toronto, Canada" << endl;
        
        // Loop until valid input
        bool valid = false;
        unsigned input;
        do{
            cout << "> " << flush;
            cin >> input;
            if(cin.fail()){
                cin.clear(); // clear fail bit
                cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // skip over bad input
                cout << "Input was not a valid number. Please enter a valid number." << endl << endl;
            }
            else if((input <= 0) || (input > numOfMaps)){
                cout << "Input was not a valid number. Please enter a valid number." << endl << endl;
            }
            else {
                valid = true;
                cout << endl << endl;
            }
        }while(!valid);
        
        // Set the city name
        string cityName;
        if(input == 1) cityName = CAIRO;
        else if(input == 2) cityName = HAMILTON;
        else if(input == 3) cityName = LONDON;
        else if(input == 4) cityName = MOSCOW;
        else if(input == 5) cityName = NEWYORK;
        else if(input == 6) cityName = ST_HELENA;
        else if(input == 7) cityName = TORONTO;
        
        // Set the map file name
        string mapFileName = "/cad2/ece297s/public/maps/" + cityName + ".streets.bin";
        loadSuccess = load_map(mapFileName);
        
        // Open up the map
        if(loadSuccess) {
            draw_map();
        }
        
        close_map();
    } while(loadSuccess); // Continue drawing maps until user exits program
    
    annClose(); // Necessary cleanup for external kd_tree 

}