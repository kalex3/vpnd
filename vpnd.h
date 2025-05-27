#ifndef VPND_H
#define VPND_H
#include <lemon/list_graph.h>
#include <lemon/full_graph.h>
#include <lemon/connectivity.h>
#include <lemon/dijkstra.h>
#include <lemon/kruskal.h>
#include <lemon/maps.h>
#include <lemon/random.h>

namespace vpnd {
using namespace std;
using namespace lemon;
typedef long long ll;
typedef long double ld;
typedef IterableBoolMap<ListGraph, ListGraph::Node> TerminalMap;
typedef TerminalMap::TrueIt TerminalIt;
typedef FilterEdges<ListGraph, ListGraph::EdgeMap<bool>> Subgraph;

class ProblemInstance {
   private:
    ListGraph g;
    ListGraph::EdgeMap<ld> c;
    TerminalMap terminal;
    ListGraph::NodeMap<ll> b_in;
    ListGraph::NodeMap<ll> b_out;
    ll R, S;
    shared_ptr<ListGraph::EdgeMap<ll>> u;
    FullGraph gm;
    bool gm_uptodate;
    FullGraph::NodeMap<ListGraph::Node> ref;
    ListGraph::NodeMap<FullGraph::Node> rev;
    ListGraph::NodeMap<shared_ptr<ListGraph::NodeMap<ListGraph::Arc>>> predmap;
    ListGraph::NodeMap<shared_ptr<ListGraph::NodeMap<ld>>> distmap;
    FullGraph::EdgeMap<ld> d;

   public:
    ProblemInstance()
        : g(), c(g), terminal(g, false), b_in(g, 0), b_out(g, 0), R(0), S(0), gm(), gm_uptodate{false}, ref(gm, INVALID), rev(g, INVALID), predmap(g), distmap(g), d(gm) {}

    ProblemInstance(int n) : ProblemInstance() {
        g.reserveNode(n);
        for (int i = 0; i < n; ++i)
            g.addNode();
    }

    ProblemInstance(int n, int m) : ProblemInstance(n) {
        g.reserveEdge(m);
    }

    void clear() {
        g.clear();
        gm_uptodate = false;
    }

    void addNodes(int num) {
        for (int i = 0; i < num; ++i) g.addNode();
        if (num > 0) gm_uptodate = false;
    }

    void addEdge(int u, int v, ld w) {
        c[g.addEdge(g.nodeFromId(u), g.nodeFromId(v))] = w;
        gm_uptodate = false;
    }

    void addReceivers(int id, ll b) {
        ListGraph::Node i = g.nodeFromId(id);
        b_in[i] += b;
        bool old = terminal[i];
        terminal[i] = b_in[i] || b_out[i];
        if (terminal[i] != old) gm_uptodate = false;
        R += b;
    }

    void addSenders(int id, ll b) {
        ListGraph::Node i = g.nodeFromId(id);
        b_out[i] += b;
        bool old = terminal[i];
        terminal[i] = b_in[i] || b_out[i];
        if (terminal[i] != old) gm_uptodate = false;
        S += b;
    }

    void multiplyBValues(ll factor) {
        for (TerminalIt i(terminal); i != INVALID; ++i) {
            b_in[i] *= factor;
            b_out[i] *= factor;
        }
        R *= factor;
        S *= factor;
    }

    int getN() {
        return countNodes(g);
    }

    int getM() {
        return countEdges(g);
    }

    ll getR() {
        return R;
    }

    ll getS() {
        return S;
    }

    ll getT() {
        return terminal.trueNum();
    }

    ld rsRatio() {
        return (ld)R / S;
    }

    void printU() {
        for (ListGraph::EdgeIt e(g); e != INVALID; ++e) cout << g.id(g.u(e)) << ' ' << g.id(g.u(e)) << ' ' << (*u)[e] << '\n';
    }

    void constructMetricClosure();

    ld algorithm1();

    ld algorithm2(unsigned int REPEAT, unsigned int BESTOF, Random &rand);

    ld lowerBound(unsigned int REPEAT, Random &rand);
};

}  // namespace vpnd
#endif
