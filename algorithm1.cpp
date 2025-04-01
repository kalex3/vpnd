#include <iostream>
#include <lemon/list_graph.h>
#include <lemon/maps.h>
#include <lemon/dijkstra.h>
#include <lemon/connectivity.h>

using namespace std;
using namespace lemon;

typedef IterableBoolMap<ListGraph, ListGraph::Node>::TrueIt TerminalIt;
typedef FilterEdges<ListGraph, ListGraph::EdgeMap<bool>> FilteredGraph;

pair<int, int> dfs(const FilteredGraph &g, const ListGraph::EdgeMap<long double> &u, const ListGraph::NodeMap<int> &b_in, const ListGraph::NodeMap<int> &b_out, int r, int s, ListGraph::NodeMap<bool> &visited, bool &valid, ListGraph::Node v) {
    visited[v] = true;
    int s_sum = b_out[v], r_sum = b_in[v];
    for (FilteredGraph::OutArcIt e(g, v); e != INVALID; ++e) {
        if (visited[g.target(e)]) continue;
        int r1, s1;
        tie(r1, s1) = dfs(g, u, b_in, b_out, r, s, visited, valid, g.target(e));
        if(min(r1, s-s1) + min(r-r1, s1) > u[e]) valid = false;
        r_sum += r1;
        s_sum += s1;
    }
    return make_pair(r_sum, s_sum);
}

int main() {
    int n, t, m;
    cin >> n >> t >> m;

    ListGraph g;
    g.reserveNode(n);
    for (int i = 0; i < n; ++i)
        g.addNode();

    ListGraph::NodeMap<int> b_in(g,0), b_out(g,0);
    IterableBoolMap<ListGraph, ListGraph::Node> terminal(g, false);

    for (int i = 0; i < t; ++i){
        int id; cin >> id;
        terminal[g.nodeFromId(id)] = true;
    }

    int r = 0, s = 0;

    for (TerminalIt it(terminal); it != INVALID; ++it) {
        cin >> b_in[it];
        r += b_in[it];
    }

    for (TerminalIt it(terminal); it != INVALID; ++it){
        cin >> b_out[it];
        s += b_out[it];
    }

    g.reserveEdge(m);
    ListGraph::EdgeMap<long double> c(g), u(g, 0);

    for (int i = 0; i < m; ++i) {
        int u_id, v_id; long double w;
        cin >> u_id >> v_id >> w;
        c[g.addEdge(g.nodeFromId(u_id), g.nodeFromId(v_id))] = w;
    }

    if(r==0 || s==0) return 0;

    if(!connected(g)){
        cout << "error: not connected\n";
        return 1;
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

    ListGraph::EdgeMap<bool> positive_edges(g, false);
    for (ListGraph::EdgeIt e(g); e != INVALID; ++e)
        positive_edges[e] = (u[e] > 0);
    FilteredGraph G(g, positive_edges);
    
    long double total_cost = 0;
    for (FilteredGraph::EdgeIt e(G); e != INVALID; ++e) {
        cout << "u(" << G.id(G.u(e)) << ", " << G.id(G.v(e)) << ") = " << u[e] << '\n';
        total_cost += u[e] * c[e];
    }

    cout << "total cost:\n" << total_cost << '\n';

    FilteredGraph::NodeMap<bool> visited(G, false); bool valid=true;
    dfs(G, u, b_in, b_out, r, s, visited, valid, root);
    if(!valid) return 1;
}