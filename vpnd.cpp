#include "vpnd.h"
#include <queue>
#include <assert.h>

namespace vpnd {
using namespace lemon;
using namespace std;
typedef IterableBoolMap<FullGraph, FullGraph::Node> R0Map;
typedef R0Map::TrueIt R0It;

void ProblemInstance::constructMetricClosure() {
    // exit if not connected
    // if (!connected(g)) throw runtime_error("Graph not connected");
    if (gm_uptodate) return;
    gm.resize(terminal.trueNum());
    Dijkstra<ListGraph, ListGraph::EdgeMap<ld>> dij(g, c);
    // Dijkstra<ListGraph, ListGraph::EdgeMap<ld>>::SetStandardHeap<QuadHeap<ld, ListGraph::NodeMap<int>>>::Create dij(g, c);
    for (TerminalIt i(terminal); i != INVALID; ++i) {
        predmap[i] = make_shared<ListGraph::NodeMap<ListGraph::Arc>>(g, INVALID);
        distmap[i] = make_shared<ListGraph::NodeMap<ld>>(g, 0);
        dij.predMap(*predmap[i]).distMap(*distmap[i]).run(i);
    }
    FullGraph::NodeIt gm_it(gm);
    TerminalIt t_it(terminal);
    while (gm_it != INVALID) {
        ref[gm_it] = t_it;
        rev[t_it] = gm_it;
        ++gm_it;
        ++t_it;
    }
    for (FullGraph::NodeIt i(gm); i != INVALID; ++i)
        for (FullGraph::NodeIt j(gm); j != INVALID; ++j)
            if (gm.index(i) < gm.index(j))
                d[gm.edge(i, j)] = (*distmap[ref[i]])[ref[j]];
    gm_uptodate = 1;
}

ld ProblemInstance::algorithm1() {
    // exit if not connected
    // if (!connected(g)) throw runtime_error("Graph not connected");

    // construct metric closure if not up to date
    if (!gm_uptodate) constructMetricClosure();

    // exit if no receivers or no senders
    if (R == 0 || S == 0) return 0;

    // reset capacities
    u = make_shared<ListGraph::EdgeMap<ll>>(g, 0);

    // calculate sum of distances for each node
    ListGraph::NodeMap<ld> distsum(g, 0);
    auto b = addMap(b_in, b_out);

    for (TerminalIt i(terminal); i != INVALID; ++i) {
        const ListGraph::NodeMap<ld> &dist = *distmap[i];
        for (ListGraph::NodeIt j(g); j != INVALID; ++j)
            distsum[j] += dist[j] * b[i];
    }

    // select node with minimal distance sum
    ListGraph::Node v = mapMin(g, distsum);

    // add capacities to the shortest path edges
    for (TerminalIt i(terminal); i != INVALID; ++i) {
        ListGraph::Node j = v;
        const ListGraph::NodeMap<ListGraph::Arc> &parent = *predmap[i];
        while (j != i) {
            ListGraph::Arc e = parent[j];
            (*u)[e] += b[i];
            j = g.source(e);
        }
    }

    return distsum[v];
}

// algorithm 2
ld ProblemInstance::algorithm2(unsigned int REPEAT, unsigned int BESTOF, Random &rand) {
    // exit if not connected
    // if (!connected(g)) throw runtime_error("Graph not connected");

    // construct metric closure if not up to date
    if (!gm_uptodate) constructMetricClosure();

    // exit if no receivers or no senders
    if (R == 0 || S == 0) return 0;

    // swap receivers and senders if R < S
    const ll &R = (this->R < this->S ? this->S : this->R);
    const ll &S = (this->R < this->S ? this->R : this->S);
    const ListGraph::NodeMap<ll> &b_in =
        (this->R < this->S ? this->b_out : this->b_in);
    const ListGraph::NodeMap<ll> &b_out =
        (this->R < this->S ? this->b_in : this->b_out);

    // reset capacities
    u.reset();

    // sort edges in advance for kruskal
    vector<ListGraph::Edge> sorted_edges;
    sorted_edges.reserve(countEdges(g));
    for (ListGraph::EdgeIt e(g); e != INVALID; ++e) sorted_edges.push_back(e);
    sort(sorted_edges.begin(), sorted_edges.end(),
         [&](ListGraph::Edge e1, ListGraph::Edge e2) {
             return c[e1] < c[e2];
         });

    priority_queue<tuple<ld, ld, shared_ptr<R0Map>, shared_ptr<vector<FullGraph::Edge>>>> upperbounds;
    ld avg2 = 0, avg3 = 0;

    // calculate upper bound for REPEAT R'-s
    for (unsigned int _ = 0; _ < REPEAT; _++) {
        // step 1: generate R'
        shared_ptr<R0Map> pR0 = make_shared<R0Map>(gm);
        R0Map &R0 = *pR0;
        int _rand_R = rand[R];
        for (FullGraph::NodeIt i(gm); i != INVALID; ++i) {
            _rand_R -= b_in[ref[i]];
            if (_rand_R < 0) {
                R0[i] = true;
                break;
            }
        }
        for (FullGraph::NodeIt i(gm); i != INVALID; ++i)
            if (b_in[ref[i]])
                R0[i] = R0[i] || !rand.boolean(pow(1.0 - 1.0 / S, b_in[ref[i]]));

        // phase 2:
        ld phase2_upperbound = 0;

        // kruskal:
        shared_ptr<vector<FullGraph::Edge>> pF = make_shared<vector<FullGraph::Edge>>();
        vector<FullGraph::Edge> &F = *pF;
        F.reserve(R0.trueNum() - 1);
        ld F_cost = kruskal(filterNodes(gm, R0), d, back_inserter(F));
        assert(F.size() == R0.trueNum() - 1);
        for (unsigned int i = 0; i + 1 < F.size(); i++) assert(d[F[i]] <= d[F[i + 1]]);
        // sort(F.begin(), F.end(),
        //      [&](FullGraph::Edge e1, FullGraph::Edge e2) {
        //          return d[e1] < d[e2];
        //      });
        ll F_cnt = 0;

        // kruskal after adding a new node
        for (FullGraph::NodeIt s(gm); s != INVALID; ++s) {
            ll senders = b_out[ref[s]];
            if (!senders) continue;
            if (R0[s]) {
                F_cnt += senders;
                continue;
            }
            vector<FullGraph::Edge> delta;
            delta.reserve(R0.trueNum());
            for (R0It i(R0); i != INVALID; ++i)
                delta.push_back(gm.edge(s, i));
            sort(delta.begin(), delta.end(),
                 [&](FullGraph::Edge e1, FullGraph::Edge e2) {
                     return d[e1] < d[e2];
                 });
            FullGraph::NodeMap<int> _uf_int_map(gm);
            UnionFind<FullGraph::NodeMap<int>> uf(_uf_int_map);
            for (R0It i(R0); i != INVALID; ++i) uf.insert(i);
            uf.insert(s);
            auto i = F.begin();
            auto j = delta.begin();
            ld new_mst_cost = 0;
            auto incIt = [&](auto &it) {
                auto e = *it;
                if (uf.find(gm.u(e)) != uf.find(gm.v(e))) {
                    uf.join(gm.u(e), gm.v(e));
                    new_mst_cost += d[e];
                }
                it++;
            };
            while (i != F.end() && j != delta.end()) {
                if (d[*i] < d[*j])
                    incIt(i);
                else
                    incIt(j);
            }
            while (i != F.end()) incIt(i);
            while (j != delta.end()) incIt(j);
            phase2_upperbound += senders * new_mst_cost;
        }

        phase2_upperbound += F_cnt * F_cost;

        // phase 3
        ld phase3 = 0;

        for (TerminalIt r(terminal); r != INVALID; ++r) {
            ld mindist = INFINITY;
            const ListGraph::NodeMap<ld> &dist = *distmap[r];
            for (R0It r0(R0); r0 != INVALID; ++r0)
                mindist = min(mindist, dist[ref[r0]]);
            phase3 += b_in[r] * mindist;
        }

        upperbounds.emplace(phase2_upperbound + phase3, phase3, pR0, pF);
        if (upperbounds.size() > BESTOF) upperbounds.pop();

        avg2 += phase2_upperbound;
        avg3 += phase3;
    }

    avg2 /= REPEAT;
    avg3 /= REPEAT;
    cout << avg2 << ' ' << avg3 << ' ' << avg2 + avg3 << ' ';

    ld mincost = INFINITY;

    // calculate end result for the best R'-s
    for (; !upperbounds.empty(); upperbounds.pop()) {
        ld phase3 = get<1>(upperbounds.top());
        R0Map &R0 = *get<2>(upperbounds.top());
        vector<FullGraph::Edge> &F = *get<3>(upperbounds.top());
        ll F_cnt = 0;
        shared_ptr<ListGraph::EdgeMap<ll>> tempu = make_shared<ListGraph::EdgeMap<ll>>(g, 0);

        // kruskal after adding a new node
        for (FullGraph::NodeIt s(gm); s != INVALID; ++s) {
            ll senders = b_out[ref[s]];
            if (!senders) continue;
            if (R0[s]) {
                F_cnt += senders;
                continue;
            }

            vector<FullGraph::Edge> delta;
            delta.reserve(R0.trueNum());
            for (R0It i(R0); i != INVALID; ++i)
                delta.push_back(gm.edge(s, i));
            sort(delta.begin(), delta.end(),
                 [&](FullGraph::Edge e1, FullGraph::Edge e2) {
                     return d[e1] < d[e2];
                 });
            FullGraph::NodeMap<int> _uf_int_map(gm);
            UnionFind<FullGraph::NodeMap<int>> uf(_uf_int_map);
            for (R0It i(R0); i != INVALID; ++i) uf.insert(i);
            uf.insert(s);
            ListGraph::NodeMap<int> _uf_int_map_g(g);
            UnionFind<ListGraph::NodeMap<int>> uf_g(_uf_int_map_g);
            for (ListGraph::NodeIt i(g); i != INVALID; ++i) uf_g.insert(i);
            IterableBoolMap<ListGraph, ListGraph::Edge> steiner(g, false);

            auto i = F.begin();
            auto j = delta.begin();
            auto incIt = [&](auto &it) {
                auto e = *it;
                auto u = gm.u(e);
                auto gu = ref[u];
                auto v = gm.v(e);
                auto gv = ref[v];
                if (uf.find(u) != uf.find(v)) {
                    uf.join(u, v);
                    auto &parent = *predmap[gu];
                    while (gv != gu) {
                        auto ge = parent[gv];
                        steiner[ge] = true;
                        gv = g.source(ge);
                    }
                }
                it++;
            };
            while (i != F.end() && j != delta.end()) {
                if (d[*i] < d[*j])
                    incIt(i);
                else
                    incIt(j);
            }
            while (i != F.end()) incIt(i);
            while (j != delta.end()) incIt(j);

            // for (IterableBoolMap<ListGraph, ListGraph::Edge>::TrueIt e(steiner); e != INVALID; ++e) {
            for (auto e : sorted_edges) {
                if (!steiner[e]) continue;
                auto u = g.u(e), v = g.v(e);
                if (uf_g.find(u) != uf_g.find(v)) {
                    uf_g.join(u, v);
                    (*tempu)[e] += senders;
                }
            }
        }

        ListGraph::EdgeMap<bool> steiner(g);
        ListGraph::NodeMap<int> _uf_int_map_g(g);
        UnionFind<ListGraph::NodeMap<int>> uf_g(_uf_int_map_g);
        for (ListGraph::NodeIt i(g); i != INVALID; ++i) uf_g.insert(i);
        for (auto e : F) {
            auto u = ref[gm.u(e)];
            auto v = ref[gm.v(e)];
            auto &parent = *predmap[u];
            while (v != u) {
                auto e = parent[v];
                steiner[e] = true;
                v = g.source(e);
            }
        }
        // for (IterableBoolMap<ListGraph, ListGraph::Edge>::TrueIt e(steiner); e != INVALID; ++e) {
        for (auto e : sorted_edges) {
            if (!steiner[e]) continue;
            auto u = g.u(e), v = g.v(e);
            if (uf_g.find(u) != uf_g.find(v)) {
                uf_g.join(u, v);
                (*tempu)[e] += F_cnt;
            }
        }

        ld cost = 0;
        for (ListGraph::EdgeIt e(g); e != INVALID; ++e) cost += (*tempu)[e] * c[e];

        cost += phase3;
        if (cost < mincost) {
            mincost = cost;

            // TODO:
            // phase 3
            u = tempu;
        }
    }

    return mincost;
}

ld ProblemInstance::lowerBound(unsigned int REPEAT, Random &rand) {
    // exit if not connected
    // if (!connected(g)) throw runtime_error("Graph not connected");

    // construct metric closure if not up to date
    if (!gm_uptodate) constructMetricClosure();

    ld maxlb = 0;
    NullMap<FullGraph::Edge, bool> _devnull;

    for (unsigned int _ = 0; _ < REPEAT; _++) {
        ld lb = 0;

        // swap receivers and senders if R < S
        ll R = this->R, S = this->S;
        if (R < S) swap(R, S);
        ListGraph::NodeMap<ll> b_in(g), b_out(g);
        if (this->R < this->S)
            mapCopy(g, this->b_in, b_out), mapCopy(g, this->b_out, b_in);
        else
            mapCopy(g, this->b_in, b_in), mapCopy(g, this->b_out, b_out);

        while (R && S) {
            ll _rand_S = rand[S], _rand_R = rand[R];
            R0Map R0(gm);
            FullGraph::Node s = INVALID;
            for (FullGraph::NodeIt i(gm); i != INVALID; ++i) {
                _rand_S -= b_out[ref[i]];
                if (_rand_S < 0) {
                    s = i;
                    break;
                }
            }
            for (FullGraph::NodeIt i(gm); i != INVALID; ++i) {
                _rand_R -= b_in[ref[i]];
                if (_rand_R < 0) {
                    R0[i] = true;
                    break;
                }
            }
            for (FullGraph::NodeIt i(gm); i != INVALID; ++i)
                if (b_in[ref[i]])
                    R0[i] = R0[i] || !rand.boolean(pow(1.0 - 1.0 / S, b_in[ref[i]]));

            ll cnt = b_out[ref[s]];
            for (R0It r0(R0); r0 != INVALID; ++r0) cnt = min(cnt, b_in[ref[r0]]);
            b_out[ref[s]] -= cnt;
            S -= cnt;
            for (R0It r0(R0); r0 != INVALID; ++r0) b_in[ref[r0]] -= cnt;
            R -= R0.trueNum() * cnt;
            R0[s] = true;
            lb += kruskal(filterNodes(gm, R0), d, _devnull) * cnt;
        }
        maxlb = max(maxlb, lb);
    }
    return maxlb / 2;
}

}  // namespace vpnd
