#include <gecode/int.hh>
#include <gecode/int/linear.hh>
#include <gecode/iter.hh>

#include <vector>
#include <tuple>

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
using NProp = NaryPropagator<TView, Int::PC_INT_BND>;

namespace Mod {

    // struct for modulo information
    struct ModInfo {
        TView* ax;
        int g;
        ModInfo(TView _ax, int _g) : ax(&_ax), g(_g) {};
    };

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
        ModInter(int n, int m);
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
        ModInter<I>::ModInter(int _off, int _mod)
        :   off(_off),
            mod(_mod),
            md(_mod - 1),
            n(_off),
            start(_off),
            end(_off)
        {}

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

        // Do initial propagation
        //  Restrict all domains
        //   check if any coefficients are negative
        // bool all_pos = true;
        //for (auto const &ax_i : ax) if (ax_i.a < 0) { all_pos = false; break; }
        // restrict domains
        /*for (auto ax_i : ax) {
            GECODE_ME_CHECK(ax_i.x.gq(home, 0));
            if (all_pos) GECODE_ME_CHECK(ax_i.x.lq(home, c / ax_i.a));
        }*/

        // test if no propagator needs to be posted
        if (!ax.assigned())
            // post propagator
            (void) new (home) Modulo(home, ax, c);

        // return completion
        return ES_OK;
    }


    // Propagate
    ExecStatus Modulo::propagate(Space& home, const ModEventDelta& modEv) {
        std::cout << "RHS == " << RHS << std::endl;
        // init vars
        int g = INT_MAX;
        //ArgArray<ModInfo> l;
        std::vector<ModInfo> l;
        int n = x.size();
        // for each variable
        for (ModTerm &ax_i : x) {
            // if variable set
            if (ax_i.x.assigned() || ax_i.a == 0) {
                if (ax_i.a != 0) {
                    std::cout << "x" << ax_i.p << " assigned to " << ax_i.x << std::endl;
                    // reduce right side by coefficient * variable
                    RHS -= ax_i.a * ax_i.x.val();
                    ax_i.a = 0;
                }
            } else {
                // add in variables modulo information
                std::vector<ModInfo> l2;
                l2.push_back(ModInfo(ax_i, g));
                // reappend all modulo information where gcd > 1
                for (ModInfo &_l : l) {
                    _l.g = gcd(_l.g, ax_i.a);
                    if (_l.g > 1) l2.push_back(_l);
                }
                l = l2;

                // update gcd
                g = gcd(g, ax_i.a);

                std::cout << g << ", " << std::endl;;
            }
        }
        
        // check for failure
        if (RHS % g != 0) return ES_FAILED; //fail
        
        // propagate
        for (ModInfo const &_l : l) {
            // _l.a _l.x = s    [ under % _l.g; ]
            std::cout << _l.ax->a << "*" << _l.ax->x << "== 0 %" << _l.g << std::endl;

            g = _l.g;
            const int c = ((RHS % g) + g) % g;

            int b, u, v;
            std::tie(b, u, v) = ::extended_gcd(_l.ax->a, c);

            //if (c % g != 0) return ES_FAILED; // already checked

            auto i = ModInter<Int::IntView>((u * c / g), (b / g));
            _l.ax->x.inter_r(home, i, false);
               

            std::cout << _l.ax->x << "== " << (u * c / g) << " %" << (b / g) << std::endl;;
        }

        //if (modEv > -10) {
        //   return propagate(home, modEv - 1);
        //} 

        //if (x.assigned()) return ES_OK;

        //if (l.size() == x.size()) return ES_FIX;

        ////// Post linear for when mod fails
        ////IntArgs _a;
        ////IntVarArgs _x;
        ////for (auto ax_i : x) {
        ////    _a << ax_i.a;
        ////    _x << ax_i.x;
        ////}
        ////linear(home, _a, _x, IRT_EQ, c);

        //return home.ES_SUBSUMED(*this);

        //if (l.size() == 0) return ES_NOFIX;

        //return ES_NOFIX;

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

    ax[0].x.eq(home, 1);
    ax[2].x.eq(home, 2);
    ax[4].x.eq(home, 2);


    // Post Propagator
    GECODE_ES_FAIL(Mod::Modulo::post(home, ax, IRT_EQ, c));
}


