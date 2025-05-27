#include <iostream>
#include <lemon/list_graph.h>
#include <lemon/random.h>
#include <lemon/unionfind.h>
#include <ext/pb_ds/assoc_container.hpp>
#include <ext/pb_ds/tree_policy.hpp>
using namespace std;
using namespace lemon;
using namespace __gnu_pbds;
typedef long long ll;
typedef long double ld;

#define ordered_set                                          \
    __gnu_pbds::tree<int, null_type, less<int>, rb_tree_tag, \
                     tree_order_statistics_node_update>

int main(int argc, char *argv[]) {
    Random rand;
    rand.seedFromFile();

    // read command line arguments
    if (argc < 8) {
        cerr << "Usage: " << argv[0] << " <n> <p> <t> <R> <S> <step> <repeat>\n";
        return 1;
    }

    int n = atoi(argv[1]), t = atoi(argv[3]);
    ld p = strtold(argv[2], NULL);
    ll R = atoll(argv[4]), S = atoll(argv[5]);
    ll step = atoll(argv[6]);
    ll repeat = atoll(argv[7]);

    // generate graph
    ListGraph g;
    ListGraph::NodeMap<int> _uf_int_map(g);
    UnionFind<ListGraph::NodeMap<int>> uf(_uf_int_map);
    for (int i = 0; i < n; i++) uf.insert(g.addNode());

    for (ListGraph::NodeIt i(g); i != INVALID; ++i)
        for (ListGraph::NodeIt j(g); j != INVALID; ++j)
            if (g.id(i) < g.id(j) && rand.boolean(p)) {
                g.addEdge(i, j);
                uf.join(i, j);
            }

    // choose the largest connected component
    int maxsize = -1;
    ListGraph::Node maxcomp = INVALID;
    for (ListGraph::NodeIt i(g); i != INVALID; ++i) {
        int sz = uf.size(i);
        if (sz > maxsize) {
            maxsize = sz;
            maxcomp = i;
        }
    }
    int maxcompid = uf.find(maxcomp);
    ListGraph::NodeMap<int> newid(g, 0);
    int cnt = 0;
    for (ListGraph::NodeIt i(g); i != INVALID; ++i)
        if (uf.find(i) == maxcompid) newid[i] = ++cnt;

    // exit if n < t
    if (n < t)
        return cerr << "Error: n < t\n", 1;
    if (cnt < t)
        return cerr << "Error: Not enough nodes in the largest component.\n", 1;
    n = cnt;

    // output the graph
    cout << n << ' ' << countEdges(g) << '\n';
    for (ListGraph::EdgeIt e(g); e != INVALID; ++e)
        if (newid[g.u(e)])
            cout << newid[g.u(e)] - 1 << ' ' << newid[g.v(e)] - 1 << ' '
                 << rand(100.0) << '\n';

    // choose t termninals randomly
    cout << t << '\n';
    vector<int> terminals;
    terminals.reserve(t);
    ordered_set s;
    for (int i = 0; i < n; i++) s.insert(i);

    for (int i = 0; i < t; i++) {
        auto it = s.find_by_order(rand[n - i]);
        cout << *it << ' ';
        terminals.push_back(*it);
        s.erase(it);
    }
    s.clear();
    cout << '\n';

    // distribute R receivers and S senders randomly between the terminals
    vector<ll> b_in(t, 0), b_out(t, 0);
    for (int i = 0; i < R; i++) b_in[rand[t]]++;
    for (int i = 0; i < S; i++) b_out[rand[t]]++;

    // output the b_in and b_out values
    for (int i : b_in) cout << i << ' ';
    cout << '\n';
    for (int i : b_out) cout << i << ' ';
    cout << '\n';

    for (int _ = 0; _ < repeat; _++) {
        vector<ll> add(t, 0);
        for (int i = 0; i < step; i++) add[rand[t]]++;
        for (int i = 0; i < t; i++)
            if (add[i]) cout << terminals[i] << ' ' << add[i] << '\n';
        cout << "-1 0\n";
        R += step;
    }
}
