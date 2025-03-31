#include <bits/stdc++.h>
#include <lemon/list_graph.h>
#include <lemon/full_graph.h>
#include <lemon/maps.h>
#include <lemon/dijkstra.h>
#include <lemon/connectivity.h>
#include <lemon/matching.h>

using namespace std;
using namespace lemon;

typedef IterableBoolMap<ListGraph, ListGraph::Node>::TrueIt TerminalIt;

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

    vector<vector<long double>> dist(t,vector<long double>(t,0));
    int i_id=0;
    FullGraph SuR(S+R);
    FullGraph::NodeIt it(SuR);
    FullGraph::NodeMap<int> terminal_id(SuR);
    FullGraph::NodeMap<bool> is_sender(SuR,false);
    FullGraph::EdgeMap<long double> l(SuR,numeric_limits<long double>::infinity());
    for(TerminalIt i(terminal); i != INVALID; ++i){
        for(int k=0;k<b_in[i];k++){
            terminal_id[it]=i_id;
            ++it;
        }
        for(int k=0;k<b_out[i];k++) {
            terminal_id[it]=i_id;
            is_sender[it]=true;
            ++it;
        }
        Dijkstra<ListGraph, ListGraph::EdgeMap<long double>> dijkstra(g, c);
        dijkstra.run(i);
        int j_id=0;
        for(TerminalIt j(terminal); j != INVALID; ++j){
            dist[i_id][j_id]=dijkstra.dist(j);
            j_id++;
        }
        i_id++;
    } 
    for(FullGraph::EdgeIt e(SuR); e != INVALID; ++e)
        l[e]=dist[terminal_id[SuR.u(e)]][terminal_id[SuR.v(e)]];

    MaxWeightedMatching<FullGraph,FullGraph::EdgeMap<long double>> mwm(SuR,l); mwm.run();
    long double total_cost = mwm.matchingWeight();
    int s_to_s_num=0; vector<long double> r_to_r; r_to_r.reserve(R/2);
    for(FullGraph::NodeIt v(SuR); v != INVALID; ++v){
        auto u = mwm.mate(v);
        if(u == INVALID) continue;
        if(SuR.id(u) < SuR.id(v)) continue;
        if(is_sender[v] && is_sender[u]) s_to_s_num++;
        else if(!is_sender[v] && !is_sender[u]) r_to_r.push_back(l[mwm.matching(v)]);
    }
    int k=r_to_r.size()-s_to_s_num;
    nth_element(r_to_r.begin(),r_to_r.begin()+k-1,r_to_r.end());
    for(int i=0;i<k;i++) total_cost-=r_to_r[i];
    cout << total_cost;
}