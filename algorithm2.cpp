#include <bits/stdc++.h>
#include <lemon/core.h>
#include <lemon/list_graph.h>
#include <lemon/maps.h>
#include <lemon/dijkstra.h>
#include <lemon/connectivity.h>
#include <lemon/path.h>
#include <lemon/random.h>

using namespace lemon;
using namespace std;

typedef IterableBoolMap<ListGraph, ListGraph::Node>::TrueIt TerminalIt;
typedef IterableBoolMap<ListGraph, ListGraph::Edge>::TrueIt TreeEdgeIt;

void findSteinerTree(const ListGraph& g, const IterableBoolMap<ListGraph, ListGraph::Node>& terminal, const ListGraph::EdgeMap<long double>& c, ListGraph::EdgeMap<bool>& tree) {
    ListGraph g1;
    ListGraph::NodeMap<ListGraph::Node> node_ref(g1);
    ListGraph::EdgeMap<ListGraph::Edge> edge_ref(g1);
    ListGraph::EdgeMap<long double> c1(g1);

    for (ListGraph::NodeIt n(g); n != INVALID; ++n)
        node_ref[g1.addNode()] = n;
    
    for (ListGraph::EdgeIt e(g); e != INVALID; ++e) {
        ListGraph::Edge new_edge = g1.addEdge(node_ref[g.u(e)], node_ref[g.v(e)]);
        edge_ref[new_edge] = e;
        c1[new_edge] = c[e];
    }

    Dijkstra<ListGraph, ListGraph::EdgeMap<long double>> dijkstra(g1, c1);

    ListGraph::NodeMap<bool> in_tree(g1, false);
    ListGraph::Node start = node_ref[TerminalIt(terminal)];
    in_tree[start] = true;

    while (true) {
        long double min_dist = numeric_limits<long double>::max();
        ListGraph::Node closest_terminal = INVALID;
        ListGraph::Node closest_node = INVALID;

        for (ListGraph::NodeIt n(g1); n != INVALID; ++n)
            if (in_tree[n]) {
                dijkstra.run(n);
                for (TerminalIt it(terminal); it != INVALID; ++it) {
                    ListGraph::Node m = node_ref[it];
                    if (!in_tree[m] && dijkstra.dist(m) < min_dist) {
                        min_dist = dijkstra.dist(m);
                        closest_terminal = m;
                        closest_node = n;
                    }
                }
            }

        if (closest_terminal == INVALID)
            break;

        ListGraph::Node i = closest_terminal;
        while (i != closest_node) {
            tree[dijkstra.predArc(i)] = true;
            in_tree[i] = true;
            i = dijkstra.predNode(i);
        }
    }
}

int main() {
    int n, t, m;
    cin >> n >> t >> m;

    ListGraph g;
    g.reserveNode(n);
    for (int i = 0; i < n; ++i)
        g.addNode();

    IterableBoolMap<ListGraph, ListGraph::Node> terminal(g, false);
    ListGraph::NodeMap<int> _b_in(g, 0), _b_out(g, 0);

    for (int i = 0; i < t; ++i) {
        int id; cin >> id;
        terminal[g.nodeFromId(id)] = true;
    }

    int r = 0;
    for (TerminalIt it(terminal); it != INVALID; ++it) {
        cin >> _b_in[it];
        r += _b_in[it];
    }

    int s = 0;
    for (TerminalIt it(terminal); it != INVALID; ++it) {
        cin >> _b_out[it];
        s += _b_out[it];
    }

    ListGraph::NodeMap<int>& b_in = (r < s ? _b_out : _b_in);
    ListGraph::NodeMap<int>& b_out = (r < s ? _b_in : _b_out);
    if (r < s) swap(r, s);

    g.reserveEdge(m);
    ListGraph::EdgeMap<long double> c(g), u(g, 0);
    for (int i = 0; i < m; ++i) {
        int u_id, v_id; long double w;
        cin >> u_id >> v_id >> w;
        c[g.addEdge(g.nodeFromId(u_id), g.nodeFromId(v_id))] = w;
    }

    Random rnd;
    rnd.seedFromFile();
    vector<unique_ptr<ListGraph::NodeMap<int>>> subsets(s);
    for (int i = 0; i < s; i++)
        subsets[i] = make_unique<ListGraph::NodeMap<int>>(g, 0);
    
    vector<bool> is_empty(s, true);
    for (TerminalIt it(terminal); it != INVALID; ++it)
        for (int i = 0; i < b_in[it]; ++i) {
            int subset_index = rnd[s];
            (*subsets[subset_index])[it] += 1;
            is_empty[subset_index] = false;
        }

    int random_index;
    do random_index = rnd[s];
    while (is_empty[random_index]);

    ListGraph::NodeMap<int>& R0_map = *(subsets[random_index]);
    IterableBoolMap<ListGraph, ListGraph::Node> steiner_terminal(g, false);
    for (ListGraph::NodeIt i(g); i != INVALID; ++i)
        if (R0_map[i]) 
            steiner_terminal[i] = true;
    
    for (TerminalIt s(terminal); s != INVALID; ++s) {
        if(!b_out[s]) continue;

        ListGraph::EdgeMap<bool> tree(g, false);
        if(steiner_terminal[s]){
            findSteinerTree(g, steiner_terminal, c, tree);
        }else{
            steiner_terminal[s] = true;
            findSteinerTree(g, steiner_terminal, c, tree);
            steiner_terminal[s] = false;
        }

        for (ListGraph::EdgeIt e(g); e != INVALID; ++e)
            if (tree[e])
                u[e] += b_out[s];
    }

    Dijkstra<ListGraph, ListGraph::EdgeMap<long double>> dijkstra(g, c);
    for (TerminalIt r(terminal); r != INVALID; ++r) {
        if(!b_in[r]) continue;

        dijkstra.run(r);
        ListGraph::Node closest = INVALID;
        long double min_dist = numeric_limits<long double>::max();
        for (TerminalIt r0(steiner_terminal); r0 != INVALID; ++r0)
            if (dijkstra.dist(r0) < min_dist) {
                min_dist = dijkstra.dist(r0);
                closest = r0;
            }
        
        if (closest != INVALID) {
            ListGraph::Node i = closest;
            while (i != r) {
                u[dijkstra.predArc(i)] += b_in[r];
                i = dijkstra.predNode(i);
            }
        }
    }

    long double total_cost = 0;
    for (ListGraph::EdgeIt e(g); e != INVALID; ++e) {
        if (u[e] > 0)
            cout << "u(" << g.id(g.u(e)) << ", " << g.id(g.v(e)) << ") = " << u[e] << '\n';
        total_cost += u[e] * c[e];
    }
    cout << "total cost: " << total_cost << '\n';

    return 0;
}