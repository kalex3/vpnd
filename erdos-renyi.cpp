#include <iostream>
#include <lemon/core.h>
#include <lemon/list_graph.h>
#include <lemon/maps.h>
#include <lemon/dijkstra.h>
#include <lemon/connectivity.h>
#include <lemon/random.h>
#include <ext/pb_ds/assoc_container.hpp>
#include <ext/pb_ds/tree_policy.hpp>

using namespace std;
using namespace lemon;
using namespace __gnu_pbds;

#define ordered_set __gnu_pbds::tree<int,null_type,less<int>,rb_tree_tag,tree_order_statistics_node_update>

int main() {
    int n, t, R, S;
    long double p;
    cin >> n >> t >> p >> R >> S;

    rnd.seedFromFile();
    
    for(;;) {
        ListGraph g; 
        for (int i = 0; i < n; i++)
            g.addNode();
            
        for (ListGraph::NodeIt i(g); i != INVALID; ++i)
            for (ListGraph::NodeIt j=i; j != INVALID; ++j)
                if (j!=i && rnd.boolean(p))
                    g.addEdge(i, j);
                    
        if (!connected(g))
            continue;

        cout << n << ' ' << t << ' ' << countEdges(g) << '\n';
        
        ordered_set s;
        for (int i = 0; i < n; i++)
            s.insert(i);
            
        for (int i = 0; i < t; i++) {
            auto it = s.find_by_order(rnd[n-i]);
            cout << *it << ' ';
            s.erase(it);
        }
        cout << '\n';
        
        vector<int> b_in(t,0), b_out(t,0);
        for (int i = 0; i < R; i++) b_in[rnd[t]]++;
        for (int i = 0; i < S; i++) b_out[rnd[t]]++;
        for(int i: b_in) cout << i << ' ';
        cout << '\n';
        for(int i: b_out) cout << i << ' ';
        cout << '\n';

        for (ListGraph::EdgeIt e(g); e != INVALID; ++e)
            cout << g.id(g.source(g.direct(e, true))) << ' ' << g.id(g.target(g.direct(e, true))) << ' ' << rnd[1000]+1 << '\n';
            
        return 0;
    }
}
