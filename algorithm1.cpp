#include <iostream>
#include <lemon/core.h>
#include <lemon/list_graph.h>
#include <lemon/maps.h>
#include <lemon/dijkstra.h>
#include <lemon/connectivity.h>

using namespace std;
using namespace lemon;

typedef IterableBoolMap<ListGraph, ListGraph::Node>::TrueIt TerminalIt;

int main() {
    int n, t, m;
    cin >> n >> t >> m;

    ListGraph g;
    g.reserveNode(n);
    for (int i = 0; i < n; ++i)
        g.addNode();

    ListGraph::NodeMap<int> b_in(g), b_out(g);
    IterableBoolMap<ListGraph, ListGraph::Node> terminal(g, false);

    for (int i = 0; i < t; ++i){
        int id; cin >> id;
        terminal[g.nodeFromId(id)] = true;
    }

    for (TerminalIt it(terminal); it != INVALID; ++it)
        cin >> b_in[it];

    for (TerminalIt it(terminal); it != INVALID; ++it)
        cin >> b_out[it];

    g.reserveEdge(m);
    ListGraph::EdgeMap<long double> c(g), u(g, 0);

    for (int i = 0; i < m; ++i) {
        int u_id, v_id; long double w;
        cin >> u_id >> v_id >> w;
        c[g.addEdge(g.nodeFromId(u_id), g.nodeFromId(v_id))] = w;
    }

    ListGraph::NodeMap<long double> distsum(g, 0);
    Dijkstra<ListGraph, ListGraph::EdgeMap<long double>> dijkstra(g, c);

    for (TerminalIt v(terminal); v != INVALID; ++v) {
        dijkstra.run(v);
        for (ListGraph::NodeIt i(g); i != INVALID; ++i)
            distsum[i] += dijkstra.dist(i) * (b_in[v] + b_out[v]);
    }

    ListGraph::Node root = mapMin(g, distsum);
    dijkstra.run(root);

    for (TerminalIt v(terminal); v != INVALID; ++v) {
        ListGraph::Node i = v;
        while (i != root) {
            u[dijkstra.predArc(i)] += b_in[v] + b_out[v];
            i = dijkstra.predNode(i);
        }
    }

    long double total_cost = 0;
    for (ListGraph::EdgeIt e(g); e != INVALID; ++e) {
        if (u[e] > 0)
            cout << "u(" << g.id(g.u(e)) << ", " << g.id(g.v(e)) << ") = " << u[e] << '\n';
        total_cost += u[e] * c[e];
    }

    cout << "total cost: " << total_cost << '\n';
}