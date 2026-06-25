#include <iostream>
#include <unordered_map>
#include <vector>
#include <queue>
#include <climits>
#include <algorithm>
#include <cmath>
#include <sstream>
#include <iomanip>
#include <limits>

using namespace std;

// ─────────────────────────────────────────────────────────────────────────────
// Helpers
// ─────────────────────────────────────────────────────────────────────────────

// "Miyapur~R"  →  "Miyapur ^R"
string formatStation(const string& station)
{
    size_t pos = station.find('~');
    if (pos != string::npos)
        return station.substr(0, pos) + " ^" + station.substr(pos + 1);
    return station;
}

// Cross-platform clear screen (no system("clear") / system("cls"))
void clearScreen()
{
#ifdef _WIN32
    // ANSI escape works on modern Windows 10+ terminals too,
    // but the WinAPI way is safer for older CMD windows.
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(h, &csbi);
    DWORD count = csbi.dwSize.X * csbi.dwSize.Y;
    DWORD written;
    COORD origin = {0, 0};
    FillConsoleOutputCharacter(h, ' ', count, origin, &written);
    SetConsoleCursorPosition(h, origin);
#else
    // ANSI escape sequence – works on every POSIX terminal
    cout << "\033[2J\033[H";
#endif
}

// ─────────────────────────────────────────────────────────────────────────────
// Graph
// ─────────────────────────────────────────────────────────────────────────────
class Graph_M {
public:
    class Vertex {
    public:
        unordered_map<string, int> nbrs;
    };

    static unordered_map<string, Vertex> vtces;

    Graph_M() { vtces.clear(); }

    int numVertex() { return static_cast<int>(vtces.size()); }

    bool containsVertex(const string& vname) { return vtces.count(vname) > 0; }

    void addVertex(const string& vname) {
        if (containsVertex(vname)) return;
        vtces[vname] = Vertex();
    }

    void removeVertex(const string& vname) {
        if (!containsVertex(vname)) return;
        for (auto& entry : vtces[vname].nbrs)
            vtces[entry.first].nbrs.erase(vname);
        vtces.erase(vname);
    }

    int numEdges() {
        int count = 0;
        for (const auto& entry : vtces)
            count += static_cast<int>(entry.second.nbrs.size());
        return count / 2;
    }

    bool containsEdge(const string& v1, const string& v2) {
        return vtces.count(v1) && vtces.count(v2) && vtces.at(v1).nbrs.count(v2);
    }

    void addEdge(const string& v1, const string& v2, int value) {
        if (!vtces.count(v1) || !vtces.count(v2) || vtces[v1].nbrs.count(v2))
            return;
        vtces[v1].nbrs[v2] = value;
        vtces[v2].nbrs[v1] = value;
    }

    void removeEdge(const string& v1, const string& v2) {
        if (!containsEdge(v1, v2)) return;
        vtces[v1].nbrs.erase(v2);
        vtces[v2].nbrs.erase(v1);
    }

    // ─────────────────────────────────────────────────────────────────────────
    // Fuzzy station lookup
    // Given user input like "Miyapur" or "Ameerpet ^R", return the internal
    // key(s) that match.  Rules (in order):
    //   1. Exact match                          → return that one key
    //   2. User typed "Name ^X"  (display form) → convert to "Name~X", exact match
    //   3. Case-insensitive prefix/substring    → collect all matches
    // When there are multiple candidates (e.g. "Ameerpet" matches ~R and ~B),
    // the caller asks the user to pick one.
    // ─────────────────────────────────────────────────────────────────────────
    vector<string> resolveStation(const string& input) const {
        // Trim whitespace
        string s = input;
        while (!s.empty() && (s.front() == ' ' || s.front() == '\t')) s.erase(s.begin());
        while (!s.empty() && (s.back()  == ' ' || s.back()  == '\t')) s.pop_back();

        // 1. Exact internal key
        if (vtces.count(s))
            return {s};

        // 2. Display form  "Name ^X"  →  internal  "Name~X"
        {
            size_t pos = s.find(" ^");
            if (pos != string::npos) {
                string candidate = s.substr(0, pos) + "~" + s.substr(pos + 2);
                if (vtces.count(candidate))
                    return {candidate};
            }
        }

        // 3. Case-insensitive substring match on the human-readable part (before '~')
        string lower = s;
        transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

        vector<string> matches;
        for (const auto& entry : vtces) {
            string key = entry.first;
            size_t tildePos = key.find('~');
            string namePart = (tildePos != string::npos) ? key.substr(0, tildePos) : key;

            string nameLower = namePart;
            transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);

            if (nameLower.find(lower) != string::npos)
                matches.push_back(key);
        }

        sort(matches.begin(), matches.end());
        return matches;
    }

    // Prompt the user to disambiguate when multiple stations match.
    // Returns the chosen internal key, or "" on failure.
    string pickStation(const string& prompt) const {
        while (true) {
            cout << prompt;
            string input;
            getline(cin, input);

            vector<string> candidates = resolveStation(input);

            if (candidates.empty()) {
                cout << "  ✗ No station found matching \"" << input
                     << "\". Please try again.\n";
                continue;
            }

            if (candidates.size() == 1)
                return candidates[0];

            // Multiple matches – ask user to choose
            cout << "  Multiple stations match \"" << input << "\":\n";
            for (size_t i = 0; i < candidates.size(); ++i)
                cout << "    " << (i + 1) << ". " << formatStation(candidates[i]) << "\n";
            cout << "  Enter number (or 0 to re-type): ";
            int pick = 0;
            cin >> pick;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            if (pick >= 1 && pick <= static_cast<int>(candidates.size()))
                return candidates[pick - 1];
            // else loop again
        }
    }

    // ─────────────────────────────────────────────────────────────────────────
    // Display helpers
    // ─────────────────────────────────────────────────────────────────────────
    void display_Map() const
    {
        cout << "\n";
        cout << "----------------------------------------------------------------------\n";
        cout << "                   HYDERABAD METRO CONNECT\n";
        cout << "----------------------------------------------------------------------\n\n";

        vector<string> keys;
        for (const auto& entry : vtces) keys.push_back(entry.first);
        sort(keys.begin(), keys.end());

        for (const string& key : keys)
        {
            cout << formatStation(key) << "\n";
            cout << "--------------------------------------------------\n";

            const Vertex& vtx = vtces.at(key);
            vector<string> vtxnbrs;
            for (const auto& nbr : vtx.nbrs) vtxnbrs.push_back(nbr.first);
            sort(vtxnbrs.begin(), vtxnbrs.end());

            for (const string& nbr : vtxnbrs)
                cout << "    " << left << setw(35) << formatStation(nbr)
                     << right << setw(5) << vtx.nbrs.at(nbr) << " km\n";
            cout << "\n";
        }
        cout << "======================================================================\n";
    }

    void display_Stations() const {
        vector<string> keys;
        for (const auto& entry : vtces) keys.push_back(entry.first);
        sort(keys.begin(), keys.end());

        int i = 1;
        for (const string& key : keys)
            cout << i++ << ". " << formatStation(key) << "\n";
    }

    // ─────────────────────────────────────────────────────────────────────────
    // Dijkstra (distance or time)
    // ─────────────────────────────────────────────────────────────────────────
    class DijkstraPair {
    public:
        string vname;
        int cost;
        bool operator<(const DijkstraPair& o) const { return cost > o.cost; }
    };

    int dijkstra(const string& src, const string& des, bool timeMode) {
        if (!containsVertex(src) || !containsVertex(des)) return -1;

        unordered_map<string, int> dist;
        priority_queue<DijkstraPair> heap;

        for (const auto& e : vtces) dist[e.first] = INT_MAX;
        dist[src] = 0;
        heap.push({src, 0});

        while (!heap.empty()) {
            DijkstraPair rp = heap.top(); heap.pop();
            if (rp.cost != dist[rp.vname]) continue;
            if (rp.vname == des) return rp.cost;

            for (const auto& nbr : vtces[rp.vname].nbrs) {
                int w = timeMode ? (120 + 40 * nbr.second) : nbr.second;
                if (dist[rp.vname] != INT_MAX && dist[rp.vname] + w < dist[nbr.first]) {
                    dist[nbr.first] = dist[rp.vname] + w;
                    heap.push({nbr.first, dist[nbr.first]});
                }
            }
        }
        return -1;
    }

    // ─────────────────────────────────────────────────────────────────────────
    // Shortest path (returns full node list + cost)
    // ─────────────────────────────────────────────────────────────────────────
    struct PathResult {
        vector<string> path;
        int totalCost = -1;
    };

    PathResult shortestPath(const string& src, const string& dst, bool timeWise) {
        PathResult result;
        if (!containsVertex(src) || !containsVertex(dst)) return result;

        unordered_map<string, int>    dist;
        unordered_map<string, string> parent;
        priority_queue<DijkstraPair>  heap;

        for (const auto& e : vtces) dist[e.first] = INT_MAX;
        dist[src]   = 0;
        parent[src] = "";
        heap.push({src, 0});

        while (!heap.empty()) {
            DijkstraPair rp = heap.top(); heap.pop();
            if (rp.cost != dist[rp.vname]) continue;
            if (rp.vname == dst) break;

            for (const auto& nbr : vtces[rp.vname].nbrs) {
                int w = timeWise ? (120 + 40 * nbr.second) : nbr.second;
                if (dist[rp.vname] != INT_MAX && dist[rp.vname] + w < dist[nbr.first]) {
                    dist[nbr.first]   = dist[rp.vname] + w;
                    parent[nbr.first] = rp.vname;
                    heap.push({nbr.first, dist[nbr.first]});
                }
            }
        }

        if (dist[dst] == INT_MAX) return result;

        vector<string> rev;
        string cur = dst;
        while (!cur.empty()) {
            rev.push_back(cur);
            if (cur == src) break;
            cur = parent.count(cur) ? parent[cur] : "";
        }
        if (rev.empty() || rev.back() != src) return result;

        reverse(rev.begin(), rev.end());
        result.path      = rev;
        result.totalCost = dist[dst];
        return result;
    }

    string Get_Minimum_Distance(const string& src, const string& dst) {
        PathResult res = shortestPath(src, dst, false);
        if (res.totalCost < 0) return "  Path not found.";

        ostringstream out;
        for (size_t i = 0; i < res.path.size(); ++i)
            out << "  " << setw(2) << (i + 1) << ". " << formatStation(res.path[i]) << "\n";
        out << "  Total Distance: " << res.totalCost << " KM";
        return out.str();
    }

    string Get_Minimum_Time(const string& src, const string& dst) {
        PathResult res = shortestPath(src, dst, true);
        if (res.totalCost < 0) return "  Path not found.";

        double minutes = ceil(static_cast<double>(res.totalCost) / 60.0);
        ostringstream out;
        for (size_t i = 0; i < res.path.size(); ++i)
            out << "  " << setw(2) << (i + 1) << ". " << formatStation(res.path[i]) << "\n";
        out << "  Total Time: " << minutes << " minutes";
        return out.str();
    }

    // ─────────────────────────────────────────────────────────────────────────
    // Metro map data
    // ─────────────────────────────────────────────────────────────────────────
    static void Create_Metro_Map(Graph_M& g)
    {
        // ── Red Line: Miyapur ↔ LB Nagar ─────────────────────────────────────
        g.addVertex("Miyapur~R");
        g.addVertex("JNTU College~R");
        g.addVertex("KPHB Colony~R");
        g.addVertex("Kukatpally~R");
        g.addVertex("Balanagar~R");
        g.addVertex("Moosapet~R");
        g.addVertex("Bharat Nagar~R");
        g.addVertex("Erragadda~R");
        g.addVertex("ESI Hospital~R");
        g.addVertex("SR Nagar~R");
        g.addVertex("Ameerpet~R");
        g.addVertex("Punjagutta~R");
        g.addVertex("Irrum Manzil~R");
        g.addVertex("Khairatabad~R");
        g.addVertex("Lakdi Ka Pul~R");
        g.addVertex("Assembly~R");
        g.addVertex("Nampally~R");
        g.addVertex("Gandhi Bhavan~R");
        g.addVertex("Osmania Medical College~R");
        g.addVertex("MG Bus Station~R");
        g.addVertex("Malakpet~R");
        g.addVertex("New Market~R");
        g.addVertex("Musarambagh~R");
        g.addVertex("Dilsukhnagar~R");
        g.addVertex("Chaitanyapuri~R");
        g.addVertex("Victoria Memorial~R");
        g.addVertex("LB Nagar~R");

        // ── Blue Line: Nagole ↔ Raidurg ───────────────────────────────────────
        g.addVertex("Nagole~B");
        g.addVertex("Uppal~B");
        g.addVertex("Stadium~B");
        g.addVertex("NGRI~B");
        g.addVertex("Habsiguda~B");
        g.addVertex("Tarnaka~B");
        g.addVertex("Mettuguda~B");
        g.addVertex("Secunderabad East~B");
        g.addVertex("Paradise~B");           // ← fixed: separate Blue node
        g.addVertex("Rasoolpura~B");
        g.addVertex("Prakash Nagar~B");
        g.addVertex("Begumpet~B");
        g.addVertex("Ameerpet~B");
        g.addVertex("Madhura Nagar~B");
        g.addVertex("Yousufguda~B");
        g.addVertex("Road No 5 Jubilee Hills~B");
        g.addVertex("Jubilee Hills Check Post~B");
        g.addVertex("Peddamma Temple~B");
        g.addVertex("Madhapur~B");
        g.addVertex("Durgam Cheruvu~B");
        g.addVertex("Hi-Tech City~B");
        g.addVertex("Raidurg~B");

        // ── Green Line: JBS ↔ MGBS ────────────────────────────────────────────
        g.addVertex("Jubilee Bus Station~G");
        g.addVertex("Paradise~G");           // ← fixed: separate Green node
        g.addVertex("Secunderabad West~G");
        g.addVertex("Gandhi Hospital~G");
        g.addVertex("Musheerabad~G");
        g.addVertex("RTC X Roads~G");
        g.addVertex("Chikkadpally~G");
        g.addVertex("Narayanguda~G");
        g.addVertex("Sultan Bazar~G");
        g.addVertex("MG Bus Station~G");

        // ── Red Line edges ─────────────────────────────────────────────────────
        g.addEdge("Miyapur~R",               "JNTU College~R",          2);
        g.addEdge("JNTU College~R",          "KPHB Colony~R",           1);
        g.addEdge("KPHB Colony~R",           "Kukatpally~R",            1);
        g.addEdge("Kukatpally~R",            "Balanagar~R",             2);
        g.addEdge("Balanagar~R",             "Moosapet~R",              2);
        g.addEdge("Moosapet~R",              "Bharat Nagar~R",          1);
        g.addEdge("Bharat Nagar~R",          "Erragadda~R",             1);
        g.addEdge("Erragadda~R",             "ESI Hospital~R",          1);
        g.addEdge("ESI Hospital~R",          "SR Nagar~R",              1);
        g.addEdge("SR Nagar~R",              "Ameerpet~R",              2);
        g.addEdge("Ameerpet~R",              "Punjagutta~R",            1);
        g.addEdge("Punjagutta~R",            "Irrum Manzil~R",          1);
        g.addEdge("Irrum Manzil~R",          "Khairatabad~R",           1);
        g.addEdge("Khairatabad~R",           "Lakdi Ka Pul~R",          1);
        g.addEdge("Lakdi Ka Pul~R",          "Assembly~R",              1);
        g.addEdge("Assembly~R",              "Nampally~R",              1);
        g.addEdge("Nampally~R",              "Gandhi Bhavan~R",         1);
        g.addEdge("Gandhi Bhavan~R",         "Osmania Medical College~R", 1);
        g.addEdge("Osmania Medical College~R","MG Bus Station~R",       1);
        g.addEdge("MG Bus Station~R",        "Malakpet~R",              2);
        g.addEdge("Malakpet~R",              "New Market~R",            1);
        g.addEdge("New Market~R",            "Musarambagh~R",           1);
        g.addEdge("Musarambagh~R",           "Dilsukhnagar~R",          2);
        g.addEdge("Dilsukhnagar~R",          "Chaitanyapuri~R",         2);
        g.addEdge("Chaitanyapuri~R",         "Victoria Memorial~R",     1);
        g.addEdge("Victoria Memorial~R",     "LB Nagar~R",              2);

        // ── Blue Line edges ────────────────────────────────────────────────────
        g.addEdge("Nagole~B",                "Uppal~B",                 3);
        g.addEdge("Uppal~B",                 "Stadium~B",               2);
        g.addEdge("Stadium~B",               "NGRI~B",                  2);
        g.addEdge("NGRI~B",                  "Habsiguda~B",             1);
        g.addEdge("Habsiguda~B",             "Tarnaka~B",               2);
        g.addEdge("Tarnaka~B",               "Mettuguda~B",             2);
        g.addEdge("Mettuguda~B",             "Secunderabad East~B",     2);
        g.addEdge("Secunderabad East~B",     "Paradise~B",              2);  // ← fixed
        g.addEdge("Paradise~B",              "Rasoolpura~B",            1);
        g.addEdge("Rasoolpura~B",            "Prakash Nagar~B",         1);
        g.addEdge("Prakash Nagar~B",         "Begumpet~B",              2);
        g.addEdge("Begumpet~B",              "Ameerpet~B",              2);
        g.addEdge("Ameerpet~B",              "Madhura Nagar~B",         2);
        g.addEdge("Madhura Nagar~B",         "Yousufguda~B",            1);
        g.addEdge("Yousufguda~B",            "Road No 5 Jubilee Hills~B", 2);
        g.addEdge("Road No 5 Jubilee Hills~B","Jubilee Hills Check Post~B", 1);
        g.addEdge("Jubilee Hills Check Post~B","Peddamma Temple~B",     2);
        g.addEdge("Peddamma Temple~B",       "Madhapur~B",              2);
        g.addEdge("Madhapur~B",              "Durgam Cheruvu~B",        2);
        g.addEdge("Durgam Cheruvu~B",        "Hi-Tech City~B",          2);
        g.addEdge("Hi-Tech City~B",          "Raidurg~B",               2);

        // ── Green Line edges ───────────────────────────────────────────────────
        g.addEdge("Jubilee Bus Station~G",   "Paradise~G",              1);  // ← fixed
        g.addEdge("Paradise~G",              "Secunderabad West~G",     1);
        g.addEdge("Secunderabad West~G",     "Gandhi Hospital~G",       2);
        g.addEdge("Gandhi Hospital~G",       "Musheerabad~G",           1);
        g.addEdge("Musheerabad~G",           "RTC X Roads~G",           1);
        g.addEdge("RTC X Roads~G",           "Chikkadpally~G",          1);
        g.addEdge("Chikkadpally~G",          "Narayanguda~G",           2);
        g.addEdge("Narayanguda~G",           "Sultan Bazar~G",          2);
        g.addEdge("Sultan Bazar~G",          "MG Bus Station~G",        1);

        // ── Interchange edges ──────────────────────────────────────────────────
        // Ameerpet: Red ↔ Blue  (walking distance ~ 1 km)
        g.addEdge("Ameerpet~R",              "Ameerpet~B",              1);

        // MG Bus Station: Red ↔ Green
        g.addEdge("MG Bus Station~R",        "MG Bus Station~G",        1);

        // Paradise: Blue ↔ Green  (← new, replaces merged Paradise~BG)
        g.addEdge("Paradise~B",              "Paradise~G",              1);
    }
};

unordered_map<string, Graph_M::Vertex> Graph_M::vtces;

// ─────────────────────────────────────────────────────────────────────────────
// main
// ─────────────────────────────────────────────────────────────────────────────
int main() {
    clearScreen();

    Graph_M g;
    Graph_M::Create_Metro_Map(g);

    cout << "\n";
    cout << "======================================================================\n";
    cout << "                 WELCOME TO HYDERABAD METRO CONNECT\n";
    cout << "======================================================================\n";
    cout << "                    • Fast • Safe • Connected\n";
    cout << "----------------------------------------------------------------------\n";

    while (true) {
        cout << "\n\tLIST OF ACTIONS\n\n";
        cout << "  1. LIST ALL THE STATIONS IN THE MAP\n";
        cout << "  2. SHOW THE METRO MAP\n";
        cout << "  3. GET SHORTEST DISTANCE FROM SOURCE TO DESTINATION\n";
        cout << "  4. GET SHORTEST TIME FROM SOURCE TO DESTINATION\n";
        cout << "  5. GET SHORTEST PATH (DISTANCE WISE)\n";
        cout << "  6. GET SHORTEST PATH (TIME WISE)\n";
        cout << "  7. EXIT\n";
        cout << "\nENTER YOUR CHOICE (1-7) : ";

        int choice = -1;
        cin >> choice;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        if (choice == 7) break;

        switch (choice) {
        case 1:
            g.display_Stations();
            break;

        case 2:
            g.display_Map();
            break;

        case 3: {
            string src = g.pickStation("Enter source station      : ");
            string dst = g.pickStation("Enter destination station : ");

            int distance = g.dijkstra(src, dst, false);
            if (distance < 0)
                cout << "  Path not found.\n";
            else
                cout << "  Shortest Distance: " << formatStation(src)
                     << " → " << formatStation(dst)
                     << " = " << distance << " KM\n";
            break;
        }

        case 4: {
            string src = g.pickStation("Enter source station      : ");
            string dst = g.pickStation("Enter destination station : ");

            int time = g.dijkstra(src, dst, true);
            if (time < 0)
                cout << "  Path not found.\n";
            else {
                double minutes = ceil(static_cast<double>(time) / 60.0);
                cout << "  Shortest Time: " << formatStation(src)
                     << " → " << formatStation(dst)
                     << " ≈ " << minutes << " minutes\n";
            }
            break;
        }

        case 5: {
            string src = g.pickStation("Enter source station      : ");
            string dst = g.pickStation("Enter destination station : ");
            cout << "\n  Route (Distance Wise): " << formatStation(src)
                 << " → " << formatStation(dst) << "\n";
            cout << g.Get_Minimum_Distance(src, dst) << "\n";
            break;
        }

        case 6: {
            string src = g.pickStation("Enter source station      : ");
            string dst = g.pickStation("Enter destination station : ");
            cout << "\n  Route (Time Wise): " << formatStation(src)
                 << " → " << formatStation(dst) << "\n";
            cout << g.Get_Minimum_Time(src, dst) << "\n";
            break;
        }

        default:
            cout << "  Invalid choice. Please enter a number from 1 to 7.\n";
        }
    }

    cout << "\nThank you for using Hyderabad Metro Connect. Have a safe journey!\n\n";
    return 0;
}
