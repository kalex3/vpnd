#include <iostream>
#include <chrono>
#include "vpnd.h"
using namespace std;
using namespace vpnd;
typedef long long ll;
typedef long double ld;

int main() {
    Random rand;
    rand.seedFromFile();
    auto start = chrono::steady_clock::now();
    // auto nextint = []() {int x; cin >> x; return x; };
    auto nextll = []() {ll x; cin >> x; return x; };

    int n, m, t;
    cin >> n >> m;
    ProblemInstance vpnd(n, m);
    for (int i = 0; i < m; i++) {
        int u, v;
        ld w;
        cin >> u >> v >> w;
        vpnd.addEdge(u, v, w);
    }

    cin >> t;
    vector<int> terminals(t);
    for (int &i : terminals) cin >> i;
    for (int i : terminals) vpnd.addReceivers(i, nextll());
    for (int i : terminals) vpnd.addSenders(i, nextll());

    cout << "# n = " << vpnd.getN() << ", ";
    cout << "m = " << vpnd.getM() << ", ";
    cout << "t = " << vpnd.getT() << '\n';
    start = chrono::steady_clock::now();
    vpnd.constructMetricClosure();
    cout << "# Metric closure time: " << chrono::duration<double>{chrono::steady_clock::now() - start}.count() << '\n';

    auto run = [&]() {
        vector<chrono::duration<double>> times;

        // if new terminals were needed
        vpnd.constructMetricClosure();

        cout << vpnd.rsRatio() << ' ';

        start = chrono::steady_clock::now();
        cout << vpnd.algorithm1() << ' ';
        times.push_back({chrono::steady_clock::now() - start});

        // start = chrono::steady_clock::now();
        // cout << vpnd.algorithm2(50, 10, rand) << ' ';
        // times.push_back({chrono::steady_clock::now() - start});
        //
        // start = chrono::steady_clock::now();
        // cout << vpnd.algorithm2(100, 10, rand) << ' ';
        // times.push_back({chrono::steady_clock::now() - start});
        //
        // start = chrono::steady_clock::now();
        // cout << vpnd.algorithm2(500, 10, rand) << ' ';
        // times.push_back({chrono::steady_clock::now() - start});

        start = chrono::steady_clock::now();
        cout << vpnd.algorithm2(1000, 10, rand) << ' ';
        times.push_back({chrono::steady_clock::now() - start});

        start = chrono::steady_clock::now();
        cout << vpnd.lowerBound(1000, rand) << ' ';
        times.push_back({chrono::steady_clock::now() - start});

        for (auto i : times) cout << i.count() << ' ';
        cout << '\n';
    };

    run();
    int tid;
    ll rec;
    while (cin >> tid >> rec) {
        if (tid >= 0)
            vpnd.addReceivers(tid, rec);
        else
            run();
    }

    // auto nextRec = [&vpnd]() {
    //     int terminal_id; ll r;
    //     if (cin >> terminal_id >> r) {
    //         vpnd.addReceivers(terminal_id, r);
    //         return 1;
    //     } else return 0;
    // };

    // do {

    // } while (nextRec());
}
