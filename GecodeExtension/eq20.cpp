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
static int next_id = 0;

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
    /// Variables
    IntVarArray x;
    std::vector<std::vector<int>> coefficients;
    int id = 0;
public:
    enum {
        PROP_LINEAR,  ///< Use regular constraints
        PROP_MODULO    ///< Use custom constraint
    };

    /// The actual problem
    Eq20(const Options& opt)
        : Script(opt) {
        // copy statics
        id = next_id;
        coefficients = a_is;

        const int x_n = coefficients[0].size() - 1;
        x = IntVarArray(*this, x_n, domains[0], domains[1]);
        for (auto ai : coefficients) {
            IntArgs c(x_n, &ai[1]);
            if (opt.propagation() == PROP_MODULO) {
                modulo(*this, c, x, ai[0], opt.ipl());
            } else { //if (opt.propagation() == PROP_LINEAR) {
                linear(*this, c, x, IRT_EQ, ai[0], opt.ipl());
            }
        }
        branch(*this, x, INT_VAR_NONE(), INT_VAL_MIN());
    }

    /// Constructor for cloning \a s
    Eq20(Eq20& s) : Script(s) {
        x.update(*this, s.x);
        this->coefficients = s.coefficients;
        this->id = s.id;
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


void run_tests(Options& opt) {
    //for (auto test_suite : { BASIC, XOR, RANDOM }) {
    for (auto test_suite : { RANDOM }) {
        std::vector<std::vector<int>> tests = generate_tests(test_suite, 250);

        // init vars
        int eq_num = 1;
        int eq_counter = 0;
        int term_num = 0;
        int reset_num = 0;
        // run the tests
        for (auto const& test : tests) {
            switch (test.size()) {
                // reset id
            case 0:
                next_id = 0;
                reset_num++;
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

                    for (auto const b : { Eq20::PROP_MODULO }) {
                    //for (auto const b : { Eq20::PROP_MODULO, Eq20::PROP_LINEAR }) {
                    //for (auto const b : { Eq20::PROP_LINEAR, Eq20::PROP_MODULO }) {

                        // get the output filename
                        std::stringstream filename;
                        filename << "Out/LOG"
                            << "_" << opt.solutions()
                            << "_" << (test_suite == BASIC ? "B" : test_suite == XOR ? "X" : "R")
                            << "_" << reset_num
                            << "_" << next_id
#if ADV_MOD:
                            << "_" << (b == Eq20::PROP_LINEAR ? "Linear" : "AdvModulo")
#else
                            << "_" << (b == Eq20::PROP_LINEAR ? "Linear" : "Modulo")
#endif
                            << ".txt";
                        // Out/LOG_<solutions>_<TestType>_<domain increases>_<id>_<propagator>.txt

                        opt.log_file(filename.str().c_str());

                        opt.propagation(b);
                        Script::run<Eq20, DFS, Options>(opt);
                    }

                    a_is.clear();
                }

                next_id++;
                break;
            }
        }
    }
}

/** \brief Main-function
 *  \relates Eq20
 */
int
main(int argc, char* argv[]) {
    for (int i = 1; i <= 1000; i <<= 2) {
            Options opt("Eq20");
            opt.time(10000); // 10 seconds timeout
            opt.solutions(i);
            opt.iterations(1000);
            opt.parse(argc, argv);
            run_tests(opt);
    }
    return 0;
}



// STATISTICS: example-any

