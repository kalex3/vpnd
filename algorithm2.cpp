#include <bits/stdc++.h>
#include <lemon/core.h>
#include <lemon/list_graph.h>
#include <lemon/maps.h>
#include <lemon/dijkstra.h>
#include <lemon/connectivity.h>
#include <lemon/path.h>
#include <lemon/random.h>
#include <lemon/adaptors.h>
#include <lemon/kruskal.h>
#include <lemon/full_graph.h>
#include <lemon/dfs.h>

using namespace lemon;
using namespace std;

typedef IterableBoolMap<ListGraph, ListGraph::Node>::TrueIt TerminalIt;
typedef IterableBoolMap<ListGraph, ListGraph::Edge>::TrueIt TreeEdgeIt;
typedef FilterNodes<ListGraph, ListGraph::NodeMap<bool>> FilteredGraph;
typedef FilterEdges<ListGraph, ListGraph::EdgeMap<bool>> TreeEdges;

int dfs(const TreeEdges &g, ListGraph::EdgeMap<long double> &u, const ListGraph::NodeMap<int> &subtree, int bout_s, ListGraph::NodeMap<bool> &visited, ListGraph::Node v) {
    visited[v] = true;
    int r0_sum = subtree[v];
    for (TreeEdges::OutArcIt e(g, v); e != INVALID; ++e) {
        if (visited[g.target(e)]) continue;
        int r0_subtree = dfs(g, u, subtree, bout_s, visited, g.target(e));
        u[e] += min(bout_s, r0_subtree);
        r0_sum += r0_subtree;
    }
    return r0_sum;
}

int main(){
    int n, t, m;
    cin >> n >> t >> m;

    ListGraph g;
    g.reserveNode(n);
    for (int i = 0; i < n; ++i)
        g.addNode();

    ListGraph::NodeMap<int> _b_in(g,0), _b_out(g,0);
    IterableBoolMap<ListGraph, ListGraph::Node> terminal(g, false);

    for (int i = 0; i < t; ++i){
        int id; cin >> id;
        terminal[g.nodeFromId(id)] = true;
    }

    int R = 0, S = 0;

    for (TerminalIt it(terminal); it != INVALID; ++it) {
        cin >> _b_in[it];
        R += _b_in[it];
    }

    for (TerminalIt it(terminal); it != INVALID; ++it){
        cin >> _b_out[it];
        S += _b_out[it];
    }

    g.reserveEdge(m);
    ListGraph::EdgeMap<long double> c(g), u(g, 0);
    for (int i = 0; i < m; ++i) {
        int u_id, v_id; long double w;
        cin >> u_id >> v_id >> w;
        ListGraph::Edge e = g.addEdge(g.nodeFromId(u_id), g.nodeFromId(v_id));
        c[e] = w;
    }

    if(R==0 || S==0) return 0;
    
    if(!connected(g)){
        cout << "error: not connected\n";
        return 1;
    }

    ListGraph::NodeMap<int>& b_in = (R < S ? _b_out : _b_in);
    ListGraph::NodeMap<int>& b_out = (R < S ? _b_in : _b_out);
    if (R < S) swap(R, S);
    
    rnd.seedFromFile();
    ListGraph::NodeMap<vector<int>> subset(g,vector<int>(S,0));
    
    set<int> nonempty_subsets;
    for (TerminalIt it(terminal); it != INVALID; ++it)
        for (int i = 0; i < b_in[it]; ++i) {
            int subset_index = rnd[S];
            subset[it][subset_index]++;
            nonempty_subsets.insert(subset_index);
        }

    int random_index = rnd[nonempty_subsets.size()];
    auto nonempty_subset_it = nonempty_subsets.begin();
    for(int i=0;i<random_index;++i) nonempty_subset_it++;
    int R0 = *nonempty_subset_it;

    FullGraph closure(n);
    FullGraph::EdgeMap<long double> dist(closure,1e18);
    ListGraph::NodeMap<FullGraph::Node> ref(g);
    FullGraph::NodeMap<ListGraph::Node> rev(closure);
    for(int i=0;i<n;++i){
        ref[g.nodeFromId(i)]=closure.nodeFromId(i);       
        rev[closure.nodeFromId(i)]=g.nodeFromId(i);
    } 
    IterableBoolMap<ListGraph, ListGraph::Node> steiner(g,false);
    ListGraph::NodeMap<vector<ListGraph::Edge>> parent(g,vector<ListGraph::Edge>(n,INVALID));
    Dijkstra<ListGraph, ListGraph::EdgeMap<long double>> dijkstra(g, c);
    FullGraph::NodeMap<bool> closure_no_nodes(closure, false);
    ListGraph::EdgeMap<bool> g_no_edges(g,false);
    FilterNodes<FullGraph, FullGraph::NodeMap<bool>> gt(closure, closure_no_nodes);
    ListGraph::NodeMap<int> subtree(g,0);

    dijkstra.init();
    for(TerminalIt i(terminal); i != INVALID; ++i)
        if(subset[i][R0]){
            gt.enable(ref[i]);
            steiner[i]=true;
            dijkstra.addSource(i);
        }

    dijkstra.start();
    
    for (TerminalIt r(terminal); r != INVALID; ++r) {
        if(!b_in[r]) continue;
        ListGraph::Node i = r;
        if (!dijkstra.reached(i)) continue;
        IterableBoolMap<ListGraph, ListGraph::Edge> tree(g,false);
        while (!subset[i][R0] && dijkstra.predArc(i) != INVALID) {
            u[dijkstra.predArc(i)] += b_in[r];
            i = dijkstra.predNode(i);
        }
        subtree[i] += b_in[r];
    }

    for(TerminalIt i(terminal); i != INVALID; ++i){
        Dijkstra<ListGraph, ListGraph::EdgeMap<long double>> dijkstra1(g, c);
        dijkstra1.run(i);
        for(ListGraph::NodeIt j(g); j != INVALID; ++j){
            if(j==i) continue;
            FullGraph::Edge e = closure.edge(ref[i],ref[j]);
            dist[e] = min(dist[e],dijkstra1.dist(j));
            parent[i][g.id(j)] = dijkstra1.predArc(j);               
        }
    } 

    for(TerminalIt s(terminal); s != INVALID; ++s){
        if(!b_out[s]) continue;
        if(!subset[s][R0]) gt.enable(ref[s]);
        vector<FullGraph::Edge> closure_edges; 
        closure_edges.reserve(n);
        kruskal(gt, dist, back_inserter(closure_edges));
        if(!subset[s][R0]) gt.disable(ref[s]);
        TreeEdges tree(g, g_no_edges);
        for(auto e: closure_edges){
            auto i = rev[closure.v(e)];
            auto u = rev[closure.u(e)];
            while(i != u && parent[u][g.id(i)] != INVALID){
                tree.enable(parent[u][g.id(i)]);
                i = g.oppositeNode(i,parent[u][g.id(i)]);
            }
        }
        TreeEdges::NodeMap<bool> visited(tree, false);
        dfs(tree, u, subtree, b_out[s], visited, s);
        // for(TreeEdgeIt e(tree_edges); e != INVALID; ++e)
        //     u[e] += b_out[s];
    }

    long double total_cost = 0;
    for (ListGraph::EdgeIt e(g); e != INVALID; ++e) {
        if(u[e] > 0) {
            cout << "u(" << g.id(g.u(e)) << ", " << g.id(g.v(e)) << ") = " << u[e] << '\n';
            total_cost += u[e] * c[e];
        }
    }

    cout << "total cost: " << total_cost << '\n';
}
