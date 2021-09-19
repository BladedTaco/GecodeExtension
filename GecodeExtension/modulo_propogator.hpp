#include <gecode/int.hh>
#include <gecode/int/linear.hh>
#include <gecode/iter.hh>

#include <vector>
#include <tuple>

#include "PrettyText.h"


#define DEBUG true
#define SHORT_CIRCUIT false
#define LIMIT_DOMAIN false
#define ADV_MOD true

// enums weren't doing highlighting, sooooo....
// 0 == set
// 1 == domain
#define DOM_TYPE 1

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
    // struct for mod domains
    struct ModDomain {
        int off;
        int mod;
        ModDomain(int _off, int _mod) : off(_off), mod(_mod) {};
        ModDomain() : off(0), mod(1) {};
    };


    class ModTerm : public Int::Linear::Term<Int::IntView> {
    public:
        using Int::Linear::Term<Int::IntView>::x;
        using Int::Linear::Term<Int::IntView>::p;
        using Int::Linear::Term<Int::IntView>::a;
        ModDomain modDom;
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
            a = y.a;
            p = y.p;
            modDom = y.modDom;
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
        ModInfo(TView& _ax, int _g) : ax(&_ax), g(_g) {};
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
            return (m - off) % mod == 0
                ? m
                : ((m - off + md) / mod) * mod + off - mod;
        }
        int nextMod(int m) const {
            // get next multiple of mod from m
            return (m - off) % mod == 0
                ? m
                : ((m - off + md) / mod) * mod + off;
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
            n = nextMod(_min);
            start = n;
            end = lastMod(_max);
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

        // cost function
        virtual PropCost cost(const Space& home, const ModEventDelta& med) const override;
    };

    // cost, lie to make this go first
    PropCost Modulo::cost(const Space&, const ModEventDelta&) const {
        return PropCost::unary(PropCost::LO);
    }

    // Copy
    Actor* Modulo::copy(Space& home) {
        return new (home) Modulo(home, *this);
    }

    // Post
    ExecStatus Modulo::post(Space& home, TArray& ax, IntRelType irt, int c) {
        // Fail on empty terms
        if (ax.size() == 0)
            return ES_FAILED;


#if LIMIT_DOMAIN
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
#endif

        // test if no propagator needs to be posted
        if (!ax.assigned()) {
            // post propagator
            (void) new (home) Modulo(home, ax, c);
        }

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
            // reduce RHS by newly assigned vars
            if (ax_i.x.assigned()) {
                if (ax_i.x.assigned() && ax_i.a != 0) {
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
            // if variable not set
            } else {
                // update gcd of old terms
                for (ModInfo& _l : l) {
                    _l.g = gcd(_l.g, ax_i.a);
                }

                // add current
                l.push_back(ModInfo(ax_i, g));

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
                for (ModInfo const & _l : l) {
                    std::cout << "(" << _l.ax->a << " * x" << _l.ax->p << ", " << _l.g << ") ";
                }
                std::cout << "]" << std::endl;
#endif
            }
        }
        
        // check for failure
        if (g == 1) return ES_FIX;
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

            // no point doing anything with % 1, we already know we are working with integers
            if (bg != 1) {
#if DEBUG
                // _l.a _l.x = s    [ under % _l.g; ]
                std::cout << std::endl << a << " * x" << _l.ax->p << " == " << c << " % " << b << COL_1
                    << "x" << _l.ax->p << " == " << ucg << " % " << bg << std::endl; 

                // domain before restriction
                std::cout << _l.ax->x << " -> ";
#endif
               
#if ADV_MOD
                ModDomain md = _l.ax->modDom;
                if (md.mod != bg && md.mod != 1) {
#if DEBUG
                    std::cout << std::endl;
                    PP("Advanced Modulo Propagation", { TextF::C_GREEN });
                    // x = n /\ x = m  [under mod a, b]
                    std::cout << std::endl << "x" << _l.ax->p << " == " 
                        << md.off << " % " << md.mod << " && "
                        << COL_1 << ucg << " % " << bg;
#endif
                    // set/get variables
                    int a, m, n;
                    a = md.off;
                    //b = _l.g;
                    m = md.mod;
                    n = bg;
                    std::tie(g, u, v) = ::extended_gcd(a, b);

                    if (a == a * b % g) {
                        // out ModInfo
                        const int bg = m * n / g;
                        const int ucg = pmod((a * v * n + b * u * m) / g, bg);

#if DEBUG
                        // _l.x = s    [ under % _l.g; ]
                        std::cout << std::endl 
                            << "x" << _l.ax->p << " == " << ucg << " % " << bg << std::endl;

                        // domain before restriction
                        PP("End Advanced Modulo Propagation", { TextF::DC_GREEN });
                        std::cout << std::endl;
#endif
                    } else {
                        PP(" No Intersection ", { C_RED });
                        std::cout << std::endl;
                        return ES_FAILED;
                    }
                }
#endif

                // intersect domain with modulus constraint
                auto i = ModInter<Int::IntView>(ucg, bg, _l.ax->x.min(), _l.ax->x.max());
                //std::cout << i.min() << " " << i.max() << std::endl;
#if DOM_TYPE == 0
                _l.ax->x.inter_v(home, i, true);
                mod(home, _l.ax->x, IntVar(home, ucg, ucg), IntVar(home, bg, bg));
#elif DOM_TYPE == 1
                dom(home, _l.ax->x, i.min(), i.max());
#endif

                _l.ax->modDom = ModDomain(ucg, bg);


#if DEBUG
                // domain after restriction
                std::cout << COL_1  <<  _l.ax->x << std::endl;
#endif
            }
        }

        // return solution found if all x_i are assigned
        if (x.assigned()) return ES_OK;

#if SHORT_CIRCUIT
        bool short_circuit = true;
        int zero_or_fulfill = -1;
        int fulfill_only = -1;
        // if there is term that can be set to make RHS 0, and everything else can be set to 0
        // for each term
        for (ModTerm& ax_i : x) {
            // if its unassigned
            if (!ax_i.x.assigned()) {
                // if it can make RHS 0
                if (RHS % ax_i.a == 0 && RHS / ax_i.a <= ax_i.x.max()) {
                    // if it can also be 0
                    if (ax_i.x.in(0)) {
                        zero_or_fulfill = ax_i.p;
                    // can only fulfill
                    } else {
                        // if something else can also only fulfill
                        if (fulfill_only != -1) {
                            // fail short circuiting
                            short_circuit = false;
                            break;
                        } else {
                            // only one fulfill_only allowed
                            fulfill_only = ax_i.p;
                        }
                    }
                // cannot make RHS 0, or be 0
                } else if (!ax_i.x.in(0)) {
                    // fail short circuiting
                    short_circuit = false;
                    break;
                }
            }
        }
        if (short_circuit) {
            // get variable to assign
            auto i = fulfill_only != -1
                ? fulfill_only
                : zero_or_fulfill;
                
            // assign it
            auto _ax = &x[i];
            _ax->x.eq(home, RHS / _ax->a);
               
            // set all others to 0
            for (ModTerm& ax_i : x) {
                if (!ax_i.x.assigned()) {
                     ax_i.x.eq(home, 0);
                }
            }
            // return finished
            return ES_OK;
        }
#endif
        PP("End Propagation", {TextF::DC_CYAN});
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
    TArray ax(home, j+1);
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

    // post linear propagator
    linear(home, a, x, IRT_EQ, c);

    // THIS IS A HACK, assign a variable to instantly start propagation before branching
    ax[j].x = IntVar(home, 0, 0);

    // Post Propagator
    GECODE_ES_FAIL(Mod::Modulo::post(home, ax, IRT_EQ, c));

}


