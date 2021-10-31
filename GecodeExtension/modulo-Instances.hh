/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */

#include <vector>
#include <time.h>

namespace {
    const int minTerms = 4;

    // settings for tests generation
    extern enum {
        BASIC,          ///< Use basic tests
        XOR,            ///< Use 'xor' tests
        RANDOM,         ///< Use random tests
    };

    // any 2 element is a new domain bounds definition
    // any 1 element is a new equation series defintion
    // any 0 element is signifying a repeat with increased domain
    std::vector<std::vector<int>> generate_tests(int testType, int num = 100) {
        // create the return vector
        std::vector<std::vector<int>> ret(num);
        // no. of eq per post.
        ret.push_back({ 1 });
        // choose tests to return
        switch (testType) {
        case BASIC:
            for (int i = 0; i <= 10; i++) {
                // large minimal domain
                //     domain_min, domain_max
                ret.push_back({ 0, 100000 << i });
                //             c, x0, x1, ...
                ret.push_back({ 1000005, 200, -190, 180, -170, 160, -150, 140, -130, 120, -110, 100, -90, 80, -70, 60, -50, 40, -30, 20, -10 });
                ret.push_back({ 100000001, 10000000, 2 });
                ret.push_back({ 136963, 10000, 9999, 5, 3 });
                ret.push_back({ 136963, 5, 3 });
                ret.push_back({ 16383, 3, 7, 15, 31, 63, 127, 255, 511, 1023, 2047, 4095, 8191 });

                // small minimal domain
                //     domain_min, domain_max
                ret.push_back({ 0, 100 << i });
                //             c, x0, x1, ...
                ret.push_back({ 1000, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100 });
                ret.push_back({ 1001, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100 });
                ret.push_back({ 1259, 5, 7, 35, 1225 });
                ret.push_back({ 1259, 12, 35, 1225 });
                ret.push_back({ 7853, 123, 453, 234, 45, 567, 234, 65, 87 });
                ret.push_back({ 7968, 123, 456, 789, 101, 112, 131, 415 });
                // eq20
                ret.push_back({ 876370, -16105, 62397, -6704, 43340, 95100, -68610, 58301 });
                ret.push_back({ 533909, 51637, 67761, 95951, 3834, -96722, 59190, 15280 });
                ret.push_back({ 915683, 1671, -34121, 10763, 80609, 42532, 93520, -33488 });
                ret.push_back({ 129768, 71202, -11119, 73017, -38875, -14413, -29234, 72370 });
                ret.push_back({ 752447, 8874, -58412, 73947, 17147, 62335, 16005, 8632 });
                ret.push_back({ 90614, 85268, 54180, -18810, -48219, 6013, 78169, -79785 });
                ret.push_back({ 1198280, -45086, 51830, -4578, 96120, 21231, 97919, 65651 });
                ret.push_back({ 18465, -64919, 80460, 90840, -59624, -75542, 25145, -47935 });
                ret.push_back({ 1503588, -43277, 43525, 92298, 58630, 92590, -9372, -60227 });
                ret.push_back({ 1244857, -16835, 47385, 97715, -12640, 69028, 76212, -81102 });
                ret.push_back({ 1410723, -60301, 31227, 93951, 73889, 81526, -72702, 68026 });
                ret.push_back({ 25334, 94016, -82071, 35961, 66597, -30705, -44404, -38304 });
                ret.push_back({ 277271, -67456, 84750, -51553, 21239, 81675, -99395, -4254 });
                ret.push_back({ 249912, -85698, 29958, 57308, 48789, -78219, 4657, 34539 });
                ret.push_back({ 373854, 85176, -95332, -1268, 57898, 15883, 50547, 83287 });
                ret.push_back({ 740061, -10343, 87758, -11782, 19346, 70072, -36991, 44529 });
                ret.push_back({ 146074, 49149, 52871, -7132, 56728, -33576, -49530, -62089 });
                ret.push_back({ 251591, -60113, 29475, 34421, -76870, 62646, 29278, -15212 });
                ret.push_back({ 22167, 87059, -29101, -5513, -21219, 22128, 7276, 57308 });
                ret.push_back({ 821228, -76706, 98205, 23445, 67921, 24111, -48614, -41906 });
                // outsourced
                ret.push_back({ 2010, 1, 20, 4, 6, 18, 12 });
                    // multi-equation
                ret.push_back({ 2 }); // no. of eq per post.
                ret.push_back({ 1320, 10, 2, 6, 12, 11 });  ret.push_back({ 1323, 10, 2, 6, 12, 11 });
                ret.push_back({ 700, 2, 4, 8, 2, 7 });      ret.push_back({ 908, 5, 7, 4, 6, 9 });
                ret.push_back({ 700, 2, 4, 8, 2, 7 });      ret.push_back({ 708, 2, 4, 8, 2, 7 });
                ret.push_back({ 1 }); // no. of eq per post.
                    // result divisible by both gcd and single coefficient
                ret.push_back({ 2565, 3, 6, 9, 12, 5 });
                ret.push_back({ 1395, 3, 6, 9, 12, 5 });
                ret.push_back({ 855, 3, 6, 9, 12, 5 });
                ret.push_back({ 405, 3, 6, 9, 12, 5 });
                ret.push_back({ 1320, 10, 2, 6, 12, 11 });
                ret.push_back({ 2505, 3, 12, 6, 9, 5 });
                ret.push_back({ 700, 2, 4, 8, 2, 7 });
                    // result only divisible by single coefficient
                ret.push_back({ 2035, 3, 6, 9, 12, 5 });
                ret.push_back({ 505, 3, 6, 9, 12, 5 });
                ret.push_back({ 1255, 3, 6, 9, 12, 5 });
                    // result divisible by most common gcd
                ret.push_back({ 2034, 3, 6, 9, 15, 5 });
                ret.push_back({ 657, 3, 6, 9, 12, 5 });
                ret.push_back({ 1580, 10, 2, 6, 12, 11 });
                // signfiy domain change repeat
                ret.push_back({});
            }
            break;
        case XOR:
            for (int dom = 0; dom < 10; dom++) {
                for (int base = 2; base <= 5; base++) {
                    //     domain_min, domain_max
                    ret.push_back({ 0, base + dom });
                    for (int terms = minTerms; terms <= 12 - 3 * (base == 5); terms++) {
                        // create test vector
                        std::vector<int> row;
                        row.reserve(terms + 1);
                        row.push_back(0);
                        // loop and add terms
                        int n = 1;
                        for (int j = 0; j < terms; j++) {
                            row.push_back(n);
                            n *= base;
                        }
                        // set first element to c
                        row[0] = n - 1;
                        row.shrink_to_fit();
                        // add to tests;
                        ret.push_back(row);
                    }
                }
                // signfiy domain change repeat
                ret.push_back({});
            }
            break;
        case RANDOM:
            // seed random
            //srand(time(NULL));
            srand(1000);
            int dom_min, dom_max, terms, c, n = 0;
            for (int dom = 0; dom < 10; dom++) {
                for (int i = 0; i < num; i++) {
                    dom_min = 1;
                    dom_max = 10 + rand() % (100 << dom);
                    //           domain_min, domain_max
                    ret.push_back({ dom_min, dom_max });
                    // create test vector
                    terms = (rand() % 10) + minTerms;
                    std::vector<int> row;
                    row.reserve(terms + 1);
                    row.push_back(0);
                    c = 0;
                    // create terms, updating c
                    for (int j = 0; j < terms; j++) {
                        n = rand() % 1000;
                        row.push_back(n);
                        c += n * (dom_min + (rand() % (dom_max - dom_min)));
                    }
                    // set first element to c
                    row[0] = c;
                    row.shrink_to_fit();
                    // add to tests;
                    ret.push_back(row);
                }
                // signfiy domain change repeat
                ret.push_back({});
            }
            break;
        }
        return ret;
    }
}

// STATISTICS: example-any
