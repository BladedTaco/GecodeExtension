#include <gecode/int.hh>
#include <gecode/int/linear.hh>
#include <gecode/iter.hh>

#include <vector>
#include <tuple>

#include "PrettyText.h"

#define DEBUG true

// function courtesy of https://www.techiedelight.com/extended-euclidean-algorithm-implementation/
// Recursive function to demonstrate the extended Euclidean algorithm.
// It returns multiple values using tuple in C++.
std::tuple<int, int, int> extended_gcd(int a, int b)
{
    if (a == 0) {
        return std::make_tuple(b, 0, 1);
    }

    int gcd, x, y;

    // unpack tuple returned by function into variables
    std::tie(gcd, x, y) = extended_gcd(b % a, a);

    return std::make_tuple(gcd, (y - (b / a) * x), x);
}

using namespace Gecode;

int gcd(int a, int b) {
    if (a == INT_MAX) return b;
    if (b == INT_MAX) return a;
    return Int::Linear::gcd(a, b);
};

inline int pmod(int a, int b) {
    return ((a % b) + b) % b;
}

namespace Mod {
    class ModTerm : public Int::Linear::Term<Int::IntView> {
    public:
        // View
        void subscribe(Space& home, Propagator& p, PropCond pc, bool schedule = true) {
            x.subscribe(home, p, pc, schedule);
        }
        void subscribe(Space& home, Advisor& a, bool fail = false) {
            x.subscribe(home, a, fail);
        }
        void cancel(Space& home, Propagator& p, PropCond pc) {
            x.cancel(home, p, pc);
        }
        void cancel(Space& home, Advisor& a, bool fail = false) {
            x.cancel(home, a, fail);
        }
        void reschedule(Space& home, Propagator& p, PropCond pc) {
            x.reschedule(home, p, pc);
        }
        void update(Space& home, ModTerm& y) {
            x.update(home, y.x);
        }

        // optional
        bool assigned() {
            return x.assigned();
        }
    };
};

using TView = Mod::ModTerm;
using TArray = ViewArray<Mod::ModTerm>;
using NProp = NaryPropagator<TView, Int::PC_INT_VAL>;

namespace Mod {

    // struct for modulo information
    struct ModInfo {
        TView* ax;
        int g;
        ModInfo(TView* _ax, int _g) : ax(_ax), g(_g) {};
    };


    //// class for mod info view
    //class ModView : public Int::IntView {

    //};


    // class for modulo domain restriction
    template <class I>
    class ModInter {
    protected: 
        int mod;
        int md;
        int off;
    private:
        int lastMod(int m) const {
            // get last multiple of mod from m
            return ((m + md) / mod) * mod - mod;
            //return (m | md) - md; powers of 2 only
        }
        int nextMod(int m) const {
            // get next multiple of mod from m
            return ((m + md) / mod) * mod;
            //return (m | md) + 1; powers of 2 only
        }


    protected:
        /// Value iterator used
        int n;
        int end;
        int start;
    public:
        /// \name Constructors and initialization
        //@{
        /// Default constructor
        ModInter(void);
        /// x = n % m
        ModInter(int _off, int _mod, int _min, int _max);
        ModInter(const ModInter &other);
        /// Initialize with value iterator \a i
        void init(const I& i0);
        //@}

        /// \name Iteration control
        //@{
        /// Test whether iterator is still at a value or done
        bool operator ()(void) const;
        /// Move iterator to next unique value (if possible)
        void operator ++(void);
        //@}

        /// \name Value access
        //@{
        /// Return current value
        int val(void) const;
        //@}


        // ranges
        int max() { return end; }
        int min() { return start; }
    };

    template <class I>
    forceinline
    ModInter<I>::ModInter(void) {}

    template <class I>
    forceinline
        ModInter<I>::ModInter(int _off, int _mod, int _min, int _max)
        :   off(_off),
            mod(_mod),
            md(_mod - 1)
        {
            n = lastMod(_min) + _off;
            start = n;
            end = nextMod(_max);
        }

    template <class I>
    forceinline
        ModInter<I>::ModInter(const ModInter &other)
        :   off(other.off),
            mod(other.mod),
            md(other.md),
            n(other.n),
            start(other.start),
            end(other.end)
    {}

    template <class I>
    forceinline void
    ModInter<I>::init(const I& i0) {
        n = lastMod(i0.min()) + off;
        start = n;
        end = nextMod(i0.max());
    }

    template <class I>
    forceinline void
    ModInter<I>::operator ++(void) {
        n += mod;
    }
    template <class I>
    forceinline bool
    ModInter<I>::operator ()(void) const {
        return n <= end;
    }

    template <class I>
    forceinline int
    ModInter<I>::val(void) const {
        return n;
    }

    //             Array | a*x terms | Propagate on Domain Change
    class Modulo : public NProp {
    protected:
        using NProp::x;
        int RHS;

        // Constructors
        // Construct Propagator
        Modulo(Home home, TArray ax, int y)
            : NProp(home, ax), RHS(y) {}
        // Clone Propagator
        Modulo(Space& home, Modulo& p)
            : NProp(home, p), RHS(p.RHS) {}
    public:
        // Constructor for rewriting p during cloning
        Modulo(Space& home, Propagator& p, TArray& ax, int y)
            : NProp(home, p, ax), RHS(y) {};

        // Copy propagator during cloning
        virtual Actor* copy(Space& home);
        // Perform propagation
        virtual ExecStatus propagate(Space& home, const ModEventDelta& med);
        // Post propagator
        static  ExecStatus post(Space& home, TArray& ax, IntRelType irt, int c);
    };


    // Copy
    Actor* Modulo::copy(Space& home) {
        return new (home) Modulo(home, *this);
    }

    // Post
    ExecStatus Modulo::post(Space& home, TArray& ax, IntRelType irt, int c) {
        // Fail on empty terms
        if (ax.size() == 0)
            return ES_FAILED;

        // check if all coefficients and domains are non-negative
        bool all_pos = true;
        for (auto const &ax_i : ax) if (ax_i.a < 0 || ax_i.x.min() < 0) { all_pos = false; break; }
        // restrict domains if there is nothing negative
        if (all_pos) {
            for (auto& ax_i : ax) {
                GECODE_ME_CHECK(ax_i.x.gq(home, 0));
                GECODE_ME_CHECK(ax_i.x.lq(home, c / ax_i.a));
            }
        }

        // test if no propagator needs to be posted
        if (!ax.assigned())
            // post propagator
            (void) new (home) Modulo(home, ax, c);

        // return completion
        return ES_OK;
    }


    // Propagate
    ExecStatus Modulo::propagate(Space& home, const ModEventDelta& modEv) {
#if DEBUG
        // print out inital RHS
        std::cout << std::endl;
        PP("-------------------------", { TextF::INVERTED });
        std::cout << std::endl;
        PP("New Propagation", { TextF::BOLD, TextF::C_CYAN });
        std::cout << COL_1 << "RHS == " << RHS << std::endl;
#endif
        // init vars
        int g = INT_MAX;
        std::vector<ModInfo> l;
        int n = x.size();
        // for each variable
        for (ModTerm &ax_i : x) {
            // if variable set
            if (ax_i.x.assigned() || ax_i.a == 0) {
                if (ax_i.a != 0) {
                    // reduce right side by coefficient * variable
                    RHS -= ax_i.a * ax_i.x.val();
                    ax_i.a = 0;

#if DEBUG
                    // print out assignment
                    std::stringstream os;
                    os << "x" << ax_i.p << " assigned to " << ax_i.x;
                    PP(os.str(), { TextF::BOLD, TextF::C_MAGENTA });
                    // print out RHS
                    std::cout << COL_1 << "RHS == " << RHS << std::endl;
#endif
                }
            } else {

                for (ModInfo& _l : l) {
                    _l.g = gcd(_l.g, ax_i.a);
                }

                // add current
                l.push_back(ModInfo(&ax_i, g));

                // remove those where gcd == 1
                l.erase(
                    std::remove_if(
                        l.begin(),
                        l.end(),
                        [](const ModInfo &element) -> bool {
                            return element.g <= 1;
                        }
                    ),
                    l.end()
                );

                // update gcd
                g = gcd(g, ax_i.a);
#if DEBUG
                // print out GCD
                std::cout << "gcd == " << g << COL_1
                // print out modInfo array
                    << "[";
                for (ModInfo& _l : l) {
                    std::cout << "(" << _l.ax->a << " * x" << _l.ax->p << ", " << _l.g << ") ";
                }
                std::cout << "]" << std::endl;
#endif
            }
        }
        
        // check for failure
        if (RHS % g != 0) return ES_FAILED; //fail
        
        // propagate
        for (ModInfo const &_l : l) {

            if (_l.g == INT_MAX) {
                _l.ax->x.eq(home, RHS / _l.ax->a);
                continue;
            }

            // in ModInfo
            const int a = _l.ax->a;
            const int b = _l.g;
            const int c = pmod(RHS, b);

            // bezouts
            int g, u, v;
            std::tie(g, u, v) = ::extended_gcd(a, b);

            // out ModInfo
            const int bg = b / g;
            const int ucg = pmod(u * c / g, bg);

#if DEBUG
            // _l.a _l.x = s    [ under % _l.g; ]
            std::cout << std::endl << a << " * x" << _l.ax->p << " == " << c << " % " << b << COL_1
                << "x" << _l.ax->p << " == " << ucg << " % " << bg << std::endl; 

            // domain before restriction
            std::cout << _l.ax->x << COL_1;
#endif

            // intersect domain with modulus constraint
            auto i = ModInter<Int::IntView>(ucg, bg, _l.ax->x.min(), _l.ax->x.max());
            _l.ax->x.inter_v(home, i, true);

#if DEBUG
            // domain after restriction
            std::cout << _l.ax->x << std::endl;
#endif
        }

        // return solution found if all x_i are assigned
        if (x.assigned()) return ES_OK;

        // otherwise return a fixpoint, the propagator only needs to run once per variable assignment
        return ES_FIX;
    }
};

void modulo(Home home, const IntArgs& a, const IntVarArgs& x, int c, IntPropLevel ipl) {
    // Ensure a and x are of the same size
    if (a.size() != x.size())
        throw Int::ArgumentSizeMismatch("Int::linear");

    // General Post checks
    GECODE_POST;

    int j = 0;
    for (int i = 0; i < x.size(); i++) {
        if (a[i] != 0) j++;
    }

    // Turn a[] and x[] into ax[]
    //TArray ax(home, x.size());
    TArray ax(home, j);
    j = 0;
    for (int i = 0; i < x.size(); i++) {
        if (a[i] == 0) {
            
            continue;
        }
        ax[j].a = a[i];
        ax[j].x = x[i];
        ax[j].p = j;
        j++;
    }

    // Post Propagator
    GECODE_ES_FAIL(Mod::Modulo::post(home, ax, IRT_EQ, c));
    linear(home, a, x, IRT_EQ, c);
    
}


