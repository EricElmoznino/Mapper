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


SUITE(extreme_london_england_public) {
    TEST(extreme_london_england) {
        std::vector<DeliveryInfo> deliveries;
        std::vector<unsigned> depots;
        std::vector<unsigned> result_path;
        bool is_legal;
        
        deliveries = {DeliveryInfo(372057, 286150), DeliveryInfo(158582, 291858), DeliveryInfo(297338, 326077), DeliveryInfo(51247, 404215), DeliveryInfo(411409, 408719), DeliveryInfo(283149, 59351), DeliveryInfo(41858, 345731), DeliveryInfo(435349, 38894), DeliveryInfo(424868, 103296), DeliveryInfo(41499, 244074), DeliveryInfo(113984, 337165), DeliveryInfo(359794, 152477), DeliveryInfo(428833, 397570), DeliveryInfo(392435, 256168), DeliveryInfo(392850, 324278), DeliveryInfo(6713, 52438), DeliveryInfo(190387, 317889), DeliveryInfo(335309, 97358), DeliveryInfo(391814, 294971), DeliveryInfo(404326, 392841), DeliveryInfo(29948, 309079), DeliveryInfo(417550, 163204), DeliveryInfo(435575, 61455), DeliveryInfo(416040, 182796), DeliveryInfo(193714, 126001), DeliveryInfo(275244, 431078), DeliveryInfo(141150, 279690), DeliveryInfo(264234, 313554), DeliveryInfo(402188, 318737), DeliveryInfo(253722, 355915), DeliveryInfo(291456, 73057), DeliveryInfo(37002, 397311), DeliveryInfo(5918, 367396), DeliveryInfo(64077, 61831), DeliveryInfo(65163, 172007), DeliveryInfo(269236, 333490), DeliveryInfo(409675, 346281), DeliveryInfo(231894, 403880), DeliveryInfo(400842, 143089), DeliveryInfo(196578, 169958), DeliveryInfo(390036, 18216), DeliveryInfo(114688, 401926), DeliveryInfo(65526, 351642)};
        depots = {98};
        {
        	ECE297_TIME_CONSTRAINT(30000);
        	
        	result_path = traveling_courier(deliveries, depots);
        }
        
        is_legal = courier_path_is_legal(deliveries, depots, result_path);
        CHECK(is_legal);
        
        if(is_legal) {
        	double path_cost = compute_path_travel_time(result_path);
        	std::cout << "QoR extreme_london_england: " << path_cost << std::endl;
        } else {
        	std::cout << "QoR extreme_london_england: INVALID" << std::endl;
        }
        
    } //extreme_london_england

    TEST(extreme_multi_london_england) {
        std::vector<DeliveryInfo> deliveries;
        std::vector<unsigned> depots;
        std::vector<unsigned> result_path;
        bool is_legal;
        
        deliveries = {DeliveryInfo(78162, 77645), DeliveryInfo(290453, 432699), DeliveryInfo(78162, 12044), DeliveryInfo(37886, 321044), DeliveryInfo(401177, 238030), DeliveryInfo(165433, 436576), DeliveryInfo(128727, 229056), DeliveryInfo(78162, 201135), DeliveryInfo(130686, 109040), DeliveryInfo(430995, 361973), DeliveryInfo(353417, 5953), DeliveryInfo(78162, 57943), DeliveryInfo(426735, 436576), DeliveryInfo(80065, 174659), DeliveryInfo(37886, 182356), DeliveryInfo(78162, 69889), DeliveryInfo(78162, 361973), DeliveryInfo(245503, 377677), DeliveryInfo(191584, 288753), DeliveryInfo(165433, 125841), DeliveryInfo(276847, 352295), DeliveryInfo(71884, 351414), DeliveryInfo(260359, 342891), DeliveryInfo(153022, 404136), DeliveryInfo(281331, 299167), DeliveryInfo(353417, 342891), DeliveryInfo(165433, 182108), DeliveryInfo(13095, 366840), DeliveryInfo(78162, 369622), DeliveryInfo(355219, 344910), DeliveryInfo(171966, 436576), DeliveryInfo(223633, 298989), DeliveryInfo(102511, 152107), DeliveryInfo(391375, 43534), DeliveryInfo(37886, 152107), DeliveryInfo(165433, 203356), DeliveryInfo(353417, 342891), DeliveryInfo(225984, 344910), DeliveryInfo(78162, 376094), DeliveryInfo(176999, 402439), DeliveryInfo(37886, 152107), DeliveryInfo(353417, 361973), DeliveryInfo(431859, 57943), DeliveryInfo(37886, 201135), DeliveryInfo(165433, 436576), DeliveryInfo(165433, 57943), DeliveryInfo(396082, 344932), DeliveryInfo(254020, 238811), DeliveryInfo(330284, 393288), DeliveryInfo(37886, 404136), DeliveryInfo(409263, 252478), DeliveryInfo(252405, 61940), DeliveryInfo(353417, 330129), DeliveryInfo(413889, 404136), DeliveryInfo(353417, 426737), DeliveryInfo(117317, 181687), DeliveryInfo(353417, 152107), DeliveryInfo(165433, 161531), DeliveryInfo(308178, 179525), DeliveryInfo(300870, 344910), DeliveryInfo(37886, 152107), DeliveryInfo(300402, 358520), DeliveryInfo(78162, 342891), DeliveryInfo(432441, 342891), DeliveryInfo(275146, 404136), DeliveryInfo(78162, 344910)};
        depots = {108};
        {
        	ECE297_TIME_CONSTRAINT(30000);
        	
        	result_path = traveling_courier(deliveries, depots);
        }
        
        is_legal = courier_path_is_legal(deliveries, depots, result_path);
        CHECK(is_legal);
        
        if(is_legal) {
        	double path_cost = compute_path_travel_time(result_path);
        	std::cout << "QoR extreme_multi_london_england: " << path_cost << std::endl;
        } else {
        	std::cout << "QoR extreme_multi_london_england: INVALID" << std::endl;
        }
        
    } //extreme_multi_london_england

} //extreme_london_england_public
 
*/
