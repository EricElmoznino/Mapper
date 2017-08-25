/*#include <random>
#include <iostream>
#include <unittest++/UnitTest++.h>

#include "StreetsDatabaseAPI.h"
#include "m1.h"
#include "m3.h"
#include "m4.h"

#include "unit_test_util.h"
#include "courier_verify.h"

using ece297test::relative_error;
using ece297test::courier_path_is_legal;

std::string map_name = "/cad2/ece297s/public/maps/saint_helena.streets.bin";

int main(int argc, char** argv) {
    bool load_success = load_map(map_name);

    if(!load_success) {
        std::cout << "ERROR: Could not load map file: '" << map_name << "'!";
        std::cout << " Subsequent tests will likely fail." << std::endl;
        //Don't abort tests, since we still want to show that all
        //tests fail.
    }

    //Run the unit tests
    int num_failures = UnitTest::RunAllTests();

    close_map();

    return num_failures;
}

SUITE(valgrind) {
    TEST(valgrind_saint_helena) {
        std::vector<DeliveryInfo> deliveries;
        std::vector<unsigned> depots;
        std::vector<unsigned> result_path;
        
        deliveries = {DeliveryInfo(197, 292)};
        depots = {168};
        result_path = traveling_courier(deliveries, depots);
        CHECK(courier_path_is_legal(deliveries, depots, result_path));
        
        deliveries = {DeliveryInfo(243, 167), DeliveryInfo(326, 295), DeliveryInfo(167, 96), DeliveryInfo(273,62), DeliveryInfo(314,324)};
        depots = {307, 101, 189};
        result_path = traveling_courier(deliveries, depots);
        CHECK(courier_path_is_legal(deliveries, depots, result_path));
    }
}*/
