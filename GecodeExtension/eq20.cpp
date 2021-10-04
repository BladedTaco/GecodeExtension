/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2001
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.org
 *
 *  Permission is hereby granted, free of charge, to any person obtaining
 *  a copy of this software and associated documentation files (the
 *  "Software"), to deal in the Software without restriction, including
 *  without limitation the rights to use, copy, modify, merge, publish,
 *  distribute, sublicense, and/or sell copies of the Software, and to
 *  permit persons to whom the Software is furnished to do so, subject to
 *  the following conditions:
 *
 *  The above copyright notice and this permission notice shall be
 *  included in all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 *  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 *  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 *  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include <gecode/driver.hh>
#include <gecode/int.hh>

#include "modulo_propogator.hpp"
#include "modulo-Instances.hh"

using namespace Gecode;

static int domains[2];
static std::vector<std::vector<int>> a_is;

/**
 * \brief %Example: Solving 20 linear equations
 *
 * Standard benchmark for solving linear equations.
 *
 * \ingroup Example
 *
 */
class Eq20 : public Script {
private:
    /// Number of variables
    //static const int x_n = 7;
    //static const int x_n = 12; //bin
    //static const int x_n = 3; 
    static const int x_n = 4;
    //static const int x_n = 20;
    /// Number of equations
    //static const int e_n = 20;
    //static const int e_n = 11;
    static const int e_n = 1;
    /// Variables
    IntVarArray x;
    IntArgs a;
    int id = 0;

    static std::vector<Eq20*> instances;
public:
    enum {
        PROP_LINEAR,  ///< Use regular constraints
        PROP_MODULO    ///< Use custom constraint
    };

    /// The actual problem
    Eq20(const Options& opt)
        : Script(opt) {
        // Coefficients and constants for the equations
        //int eqs[e_n][x_n + 1] = {
        //  //z    =  a*x1 + b*x2 + c*x3 + d*x4 + e*x5 + f*x6 + g*x7
        //    {1000005, 200, -190, 180, -170, 160, -150, 140, -130, 120, -110, 100, -90, 80, -70, 60, -50, 40, -30, 20, -10},
        //    {100000000001, 10000000, 2},
        //    {136963, 10000, 9999, 5, 3},
        //    {136963, 5, 3},
        //    {16383, 3, 7, 15, 31, 63, 127, 255, 511, 1023, 2047, 4095, 8191},
        //    {1000, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100},
        //    {1001, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100},
        //    {1259, 5, 7, 35, 1225},
        //    {1259, 12, 35, 1225},
        //    {7853, 123, 453, 234, 45, 567, 234, 65, 87},
        //    {7968, 123, 456, 789, 101, 112, 131, 415}
        //  /*{876370, -16105, 62397, -6704, 43340, 95100, -68610, 58301},
        //  {533909, 51637, 67761, 95951, 3834, -96722, 59190, 15280},
        //  {915683, 1671, -34121, 10763, 80609, 42532, 93520, -33488},
        //  {129768, 71202, -11119, 73017, -38875, -14413, -29234, 72370},
        //  {752447, 8874, -58412, 73947, 17147, 62335, 16005, 8632},
        //  {90614, 85268, 54180, -18810, -48219, 6013, 78169, -79785},
        //  {1198280, -45086, 51830, -4578, 96120, 21231, 97919, 65651},
        //  {18465, -64919, 80460, 90840, -59624, -75542, 25145, -47935},
        //  {1503588, -43277, 43525, 92298, 58630, 92590, -9372, -60227},
        //  {1244857, -16835, 47385, 97715, -12640, 69028, 76212, -81102},
        //  {1410723, -60301, 31227, 93951, 73889, 81526, -72702, 68026},
        //  {25334, 94016, -82071, 35961, 66597, -30705, -44404, -38304},
        //  {277271, -67456, 84750, -51553, 21239, 81675, -99395, -4254},
        //  {249912, -85698, 29958, 57308, 48789, -78219, 4657, 34539},
        //  {373854, 85176, -95332, -1268, 57898, 15883, 50547, 83287},
        //  {740061, -10343, 87758, -11782, 19346, 70072, -36991, 44529},
        //  {146074, 49149, 52871, -7132, 56728, -33576, -49530, -62089},
        //  {251591, -60113, 29475, 34421, -76870, 62646, 29278, -15212},
        //  {22167, 87059, -29101, -5513, -21219, 22128, 7276, 57308},
        //  {821228, -76706, 98205, 23445, 67921, 24111, -48614, -41906}*/
        //};
        // Post equation constraints

        // [0..2]
        //int b[x_n+1] = {4095, 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048};
        //int b[x_n+1] = {9, 19, 15, 14, 12, 8};
        //int b[x_n + 1] = { 26, 5, 6, 3 };
        //int b[x_n + 1] = { 1000, 2, 4, 8, 16, 222 };
        //int b[x_n + 1] = { 999, 15, 3, 30, 60};
        //IntArgs c(x_n, &b[1]);
        //if (opt.propagation() == PROP_MODULO) {
        //    modulo(*this, c, x, b[0], opt.ipl());
        //} else { //if (opt.propagation() == PROP_LINEAR) {
        //    linear(*this, c, x, IRT_EQ, b[0], opt.ipl());
        //}
        
        std::cout << a_is.size() << std::endl;
        std::cout << domains << std::endl;


        const int x_n = a_is[0].size() - 1;
        x = IntVarArray(*this, x_n, domains[0], domains[1]);
        for (auto ai : a_is) {
            IntArgs c(x_n, &ai[1]);
            if (opt.propagation() == PROP_MODULO) {
                modulo(*this, c, x, ai[0], opt.ipl());
            } else { //if (opt.propagation() == PROP_LINEAR) {
                linear(*this, c, x, IRT_EQ, ai[0], opt.ipl());
            }
        }
        branch(*this, x, INT_VAR_NONE(), INT_VAL_MIN());


        //auto const tests = generate_tests(RANDOM, 1);

        //// init vars
        //int eq_num = 1;
        //int eq_counter = 1;
        //int domains[2] = { 0, 100 };
        //int term_num = 0;
        //std::vector<int> test_results;
        //// run the tests
        //for (auto const& test : tests) {
        //    switch (test.size()) {
        //        // reset id
        //    case 0:
        //        id = 0;
        //        break;
        //        // update equation number per post
        //    case 1:
        //        eq_num = test[0];
        //        break;
        //        // update domain
        //    case 2:
        //        domains[0] = test[0];
        //        domains[1] = test[1];
        //        break;
        //        // new post
        //    default:
        //        // get number of terms
        //        term_num = test.size() - 1;

        //        // if new equation set
        //        if (eq_counter == 1) {
        //            // generate new x var array
        //            x = IntVarArray(*this, term_num, domains[0], domains[1]);
        //        }

        //        // get unique parameters for posting
        //        a = IntArgs(term_num, &test[1]);
        //        if (opt.propagation() == PROP_MODULO) {
        //            modulo(*this, a, x, test[0], opt.ipl());
        //        } else { //if (opt.propagation() == PROP_LINEAR) {
        //            linear(*this, a, x, IRT_EQ, test[0], opt.ipl());
        //        }
        //        branch(*this, x, INT_VAR_NONE(), INT_VAL_MIN());


        //        // handle equation set counting
        //        eq_num == eq_counter
        //            ? eq_counter = 1
        //            : eq_counter++;
        //        // handle id
        //        id++;
        //        break;
        //    }
        //}

        //for (int i = e_n; i--; ) {
        //    IntArgs c(x_n, &eqs[i][1]);
        //    if (opt.propagation() == PROP_MODULO) {
        //        modulo(*this, c, x, eqs[i][0], opt.ipl());
        //    } else { //if (opt.propagation() == PROP_LINEAR) {
        //        linear(*this, c, x, IRT_EQ, eqs[i][0], opt.ipl());
        //    }
        //}
    }

    /// Constructor for cloning \a s
    Eq20(Eq20& s) : Script(s) {
        x.update(*this, s.x);
    }
    /// Perform copying during cloning
    virtual Space*
        copy(void) {
        return new Eq20(*this);
    }
    /// Print solution
    virtual void
        print(std::ostream& os) const {
        os << "\tx[] = " << x << std::endl;
    }

};


void run_tests(const Options& opt) {
    auto const tests = generate_tests(RANDOM, 1);

    // init vars
    int eq_num = 1;
    int eq_counter = 0;
    int term_num = 0;
    // run the tests
    for (auto const& test : tests) {
        switch (test.size()) {
            // reset id
        case 0:
            //id = 0;
            break;
            // update equation number per post
        case 1:
            eq_num = test[0];
            break;
            // update domain
        case 2:
            domains[0] = test[0];
            domains[1] = test[1];
            break;
            // new post
        default:
            // get number of terms
            term_num = test.size() - 1;

            a_is.push_back(test);
            // if new equation set
            if (eq_num == ++eq_counter) {
                eq_counter = 0;

                Script::run<Eq20, DFS, Options>(opt);

                a_is.clear();
            }

            break;
        }
    }
}

/** \brief Main-function
 *  \relates Eq20
 */
int
main(int argc, char* argv[]) {
    for (int i = 1; i <= 1000; i <<= 2) {
        for (auto const b : { Eq20::PROP_MODULO, Eq20::PROP_LINEAR }) {
        //for (auto const b : { Eq20::PROP_LINEAR, Eq20::PROP_MODULO }) {
            std::cout << (b == Eq20::PROP_LINEAR ? "Linear" : "Modulo") << std::endl;
            Options opt("Eq20");
            opt.propagation(b);
            opt.solutions(i);
            opt.iterations(10000);
            opt.parse(argc, argv);
            run_tests(opt);
        }
    }
    return 0;
}



// STATISTICS: example-any

