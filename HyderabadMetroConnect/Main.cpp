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

string formatStation(const string& station)
{
    size_t pos = station.find('~');
    if (pos != string::npos) 
    {
        return station.substr(0, pos) + " ^" + station.substr(pos + 1);
    }
    return station;
}

class Graph_M {
public:
    class Vertex {
    public:
        unordered_map<string, int> nbrs;
    };

    static unordered_map<string, Vertex> vtces;

    Graph_M() {
        vtces.clear();
    }

    int numVertex() {
        return static_cast<int>(vtces.size());
    }

    bool containsVertex(string vname) {
        return vtces.count(vname) > 0;
    }

    void addVertex(string vname) {
        if (containsVertex(vname)) return;
        Vertex vtx;
        vtces[vname] = vtx;
    }

    void removeVertex(string vname) {
        if (!containsVertex(vname)) return;

        vector<string> keys;
        for (auto& entry : vtces[vname].nbrs) {
            keys.push_back(entry.first);
        }

        for (const string& key : keys) {
            vtces[key].nbrs.erase(vname);
        }

        vtces.erase(vname);
    }

    int numEdges() {
        int count = 0;
        for (const auto& entry : vtces) {
            count += static_cast<int>(entry.second.nbrs.size());
        }
        return count / 2;
    }

    bool containsEdge(string vname1, string vname2) {
        return vtces.count(vname1) > 0 && vtces.count(vname2) > 0 && vtces[vname1].nbrs.count(vname2) > 0;
    }

    void addEdge(string vname1, string vname2, int value) {
        if (vtces.count(vname1) == 0 || vtces.count(vname2) == 0 || vtces[vname1].nbrs.count(vname2) > 0) {
            return;
        }
        vtces[vname1].nbrs[vname2] = value;
        vtces[vname2].nbrs[vname1] = value;
    }

    void removeEdge(string vname1, string vname2) {
        if (!containsEdge(vname1, vname2)) {
            return;
        }
        vtces[vname1].nbrs.erase(vname2);
        vtces[vname2].nbrs.erase(vname1);
    }

    void display_Map()
    {
        cout << "\n";
        cout << "----------------------------------------------------------------------\n";
        cout << "                   HYDERABAD METRO CONNECT\n";
        cout << "----------------------------------------------------------------------\n\n";

        vector<string> keys;
        for (const auto& entry : vtces)
        {
            keys.push_back(entry.first);
        }
        sort(keys.begin(), keys.end());

        for (const string& key : keys)
        {
            cout << formatStation(key) << endl;
            cout << "--------------------------------------------------" << endl;

            const Vertex& vtx = vtces[key];

            vector<string> vtxnbrs;
            for (const auto& nbr : vtx.nbrs)
            {
                vtxnbrs.push_back(nbr.first);
            }

            sort(vtxnbrs.begin(), vtxnbrs.end());

            for (const string& nbr : vtxnbrs)
            {
                cout << "    "
                     << left << setw(35)
                     << formatStation(nbr)
                     << right << setw(5)
                     << vtx.nbrs.at(nbr)
                     << " km"
                     << endl;
            }

            cout << endl;
        }

        cout << "======================================================================\n";
    }

    void display_Stations() {
        vector<string> keys;
        for (const auto& entry : vtces) {
            keys.push_back(entry.first);
        }
        sort(keys.begin(), keys.end());

        int i = 1;
        for (const string& key : keys) {
            cout << i << ". " << formatStation(key) << endl;
            i++;
        }
    }

    bool hasPath(string vname1, string vname2, unordered_map<string, bool>& processed) {
        if (containsEdge(vname1, vname2)) {
            return true;
        }

        processed[vname1] = true;

        const Vertex& vtx = vtces[vname1];
        for (const auto& entry : vtx.nbrs) {
            const string& nbr = entry.first;
            if (!processed.count(nbr) || !processed[nbr]) {
                if (hasPath(nbr, vname2, processed)) {
                    return true;
                }
            }
        }

        return false;
    }

    class DijkstraPair {
    public:
        string vname;
        int cost;

        bool operator<(const DijkstraPair& other) const {
            return cost > other.cost; // min-heap behavior
        }
    };

    int dijkstra(string src, string des, bool nan) {
        if (!containsVertex(src) || !containsVertex(des)) {
            return -1;
        }

        unordered_map<string, int> dist;
        priority_queue<DijkstraPair> heap;

        for (const auto& entry : vtces) {
            dist[entry.first] = INT_MAX;
        }
        dist[src] = 0;
        heap.push({src, 0});

        while (!heap.empty()) {
            DijkstraPair rp = heap.top();
            heap.pop();

            if (rp.cost != dist[rp.vname]) {
                continue;
            }

            if (rp.vname == des) {
                return rp.cost;
            }

            const Vertex& v = vtces[rp.vname];
            for (const auto& nbr : v.nbrs) {
                int w = nan ? (120 + 40 * nbr.second) : nbr.second;
                if (dist[rp.vname] != INT_MAX && dist[rp.vname] + w < dist[nbr.first]) {
                    dist[nbr.first] = dist[rp.vname] + w;
                    heap.push({nbr.first, dist[nbr.first]});
                }
            }
        }

        return -1;
    }

    struct PathResult {
        vector<string> path;
        int totalCost = -1;
    };

    PathResult shortestPath(string src, string dst, bool timeWise) {
        PathResult result;

        if (!containsVertex(src) || !containsVertex(dst)) {
            return result;
        }

        unordered_map<string, int> dist;
        unordered_map<string, string> parent;
        priority_queue<DijkstraPair> heap;

        for (const auto& entry : vtces) {
            dist[entry.first] = INT_MAX;
        }

        dist[src] = 0;
        parent[src] = "";
        heap.push({src, 0});

        while (!heap.empty()) {
            DijkstraPair rp = heap.top();
            heap.pop();

            if (rp.cost != dist[rp.vname]) {
                continue;
            }

            if (rp.vname == dst) {
                break;
            }

            const Vertex& v = vtces[rp.vname];
            for (const auto& nbr : v.nbrs) {
                int w = timeWise ? (120 + 40 * nbr.second) : nbr.second;
                if (dist[rp.vname] != INT_MAX && dist[rp.vname] + w < dist[nbr.first]) {
                    dist[nbr.first] = dist[rp.vname] + w;
                    parent[nbr.first] = rp.vname;
                    heap.push({nbr.first, dist[nbr.first]});
                }
            }
        }

        if (dist[dst] == INT_MAX) {
            return result;
        }

        vector<string> rev;
        string cur = dst;
        while (cur != "") {
            rev.push_back(cur);
            if (cur == src) break;
            cur = parent.count(cur) ? parent[cur] : "";
        }

        if (rev.empty() || rev.back() != src) {
            return result;
        }

        reverse(rev.begin(), rev.end());
        result.path = rev;
        result.totalCost = dist[dst];
        return result;
    }

    string Get_Minimum_Distance(string src, string dst) {
        PathResult res = shortestPath(src, dst, false);
        if (res.totalCost < 0) {
            return "Path not found";
        }

        ostringstream out;
        for (size_t i = 0; i < res.path.size(); ++i) {
            out << i + 1 << ". " << formatStation(res.path[i]) << '\n';
        }
        out << "Total Distance: " << res.totalCost << " KM";
        return out.str();
    }

    string Get_Minimum_Time(string src, string dst) {
        PathResult res = shortestPath(src, dst, true);
        if (res.totalCost < 0) {
            return "Path not found";
        }

        double minutes = ceil(static_cast<double>(res.totalCost) / 60.0);

        ostringstream out;
        for (size_t i = 0; i < res.path.size(); ++i) {
            out << i + 1 << ". " << formatStation(res.path[i]) << '\n';
        }
        out << "Total Time: " << minutes << " minutes";
        return out.str();
    }

    static void Create_Metro_Map(Graph_M& g)
    {
        // Red Line: Miyapur <-> LB Nagar
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

        // Blue Line: Nagole <-> Raidurg
        g.addVertex("Nagole~B");
        g.addVertex("Uppal~B");
        g.addVertex("Stadium~B");
        g.addVertex("NGRI~B");
        g.addVertex("Habsiguda~B");
        g.addVertex("Tarnaka~B");
        g.addVertex("Mettuguda~B");
        g.addVertex("Secunderabad East~B");
        g.addVertex("Paradise~BG");
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

        // Green Line: JBS <-> MGBS
        g.addVertex("Jubilee Bus Station~G");
        g.addVertex("Secunderabad West~G");
        g.addVertex("Gandhi Hospital~G");
        g.addVertex("Musheerabad~G");
        g.addVertex("RTC X Roads~G");
        g.addVertex("Chikkadpally~G");
        g.addVertex("Narayanguda~G");
        g.addVertex("Sultan Bazar~G");
        g.addVertex("MG Bus Station~G");

        // Red Line edges (distances in km)
        g.addEdge("Miyapur~R", "JNTU College~R", 2);
        g.addEdge("JNTU College~R", "KPHB Colony~R", 1);
        g.addEdge("KPHB Colony~R", "Kukatpally~R", 1);
        g.addEdge("Kukatpally~R", "Balanagar~R", 2);
        g.addEdge("Balanagar~R", "Moosapet~R", 2);
        g.addEdge("Moosapet~R", "Bharat Nagar~R", 1);
        g.addEdge("Bharat Nagar~R", "Erragadda~R", 1);
        g.addEdge("Erragadda~R", "ESI Hospital~R", 1);
        g.addEdge("ESI Hospital~R", "SR Nagar~R", 1);
        g.addEdge("SR Nagar~R", "Ameerpet~R", 2);
        g.addEdge("Ameerpet~R", "Punjagutta~R", 1);
        g.addEdge("Punjagutta~R", "Irrum Manzil~R", 1);
        g.addEdge("Irrum Manzil~R", "Khairatabad~R", 1);
        g.addEdge("Khairatabad~R", "Lakdi Ka Pul~R", 1);
        g.addEdge("Lakdi Ka Pul~R", "Assembly~R", 1);
        g.addEdge("Assembly~R", "Nampally~R", 1);
        g.addEdge("Nampally~R", "Gandhi Bhavan~R", 1);
        g.addEdge("Gandhi Bhavan~R", "Osmania Medical College~R", 1);
        g.addEdge("Osmania Medical College~R", "MG Bus Station~R", 1);
        g.addEdge("MG Bus Station~R", "Malakpet~R", 2);
        g.addEdge("Malakpet~R", "New Market~R", 1);
        g.addEdge("New Market~R", "Musarambagh~R", 1);
        g.addEdge("Musarambagh~R", "Dilsukhnagar~R", 2);
        g.addEdge("Dilsukhnagar~R", "Chaitanyapuri~R", 2);
        g.addEdge("Chaitanyapuri~R", "Victoria Memorial~R", 1);
        g.addEdge("Victoria Memorial~R", "LB Nagar~R", 2);

        // Blue Line edges
        g.addEdge("Nagole~B", "Uppal~B", 3);
        g.addEdge("Uppal~B", "Stadium~B", 2);
        g.addEdge("Stadium~B", "NGRI~B", 2);
        g.addEdge("NGRI~B", "Habsiguda~B", 1);
        g.addEdge("Habsiguda~B", "Tarnaka~B", 2);
        g.addEdge("Tarnaka~B", "Mettuguda~B", 2);
        g.addEdge("Mettuguda~B", "Secunderabad East~B", 2);
        g.addEdge("Secunderabad East~B", "Paradise~BG", 2);
        g.addEdge("Paradise~BG", "Rasoolpura~B", 1);
        g.addEdge("Rasoolpura~B", "Prakash Nagar~B", 1);
        g.addEdge("Prakash Nagar~B", "Begumpet~B", 2);
        g.addEdge("Begumpet~B", "Ameerpet~B", 2);
        g.addEdge("Ameerpet~B", "Madhura Nagar~B", 2);
        g.addEdge("Madhura Nagar~B", "Yousufguda~B", 1);
        g.addEdge("Yousufguda~B", "Road No 5 Jubilee Hills~B", 2);
        g.addEdge("Road No 5 Jubilee Hills~B", "Jubilee Hills Check Post~B", 1);
        g.addEdge("Jubilee Hills Check Post~B", "Peddamma Temple~B", 2);
        g.addEdge("Peddamma Temple~B", "Madhapur~B", 2);
        g.addEdge("Madhapur~B", "Durgam Cheruvu~B", 2);
        g.addEdge("Durgam Cheruvu~B", "Hi-Tech City~B", 2);
        g.addEdge("Hi-Tech City~B", "Raidurg~B", 2);

        // Ameerpet interchange between Red and Blue
        g.addEdge("Ameerpet~R", "Ameerpet~B", 1);

        // Green Line edges
        g.addEdge("Jubilee Bus Station~G", "Secunderabad West~G", 2);
        g.addEdge("Secunderabad West~G", "Gandhi Hospital~G", 1);
        g.addEdge("Gandhi Hospital~G", "Musheerabad~G", 1);
        g.addEdge("Musheerabad~G", "RTC X Roads~G", 1);
        g.addEdge("RTC X Roads~G", "Chikkadpally~G", 1);
        g.addEdge("Chikkadpally~G", "Narayanguda~G", 2);
        g.addEdge("Narayanguda~G", "Sultan Bazar~G", 2);
        g.addEdge("Sultan Bazar~G", "MG Bus Station~G", 1);

        // MGBS interchange between Red and Green
        g.addEdge("MG Bus Station~R", "MG Bus Station~G", 1);

        // Kept as original model
        g.addEdge("Jubilee Bus Station~G", "Paradise~BG", 1);
    }
};

unordered_map<string, Graph_M::Vertex> Graph_M::vtces;

int main() {
    system("clear");
    Graph_M g;
    Graph_M::Create_Metro_Map(g);

    cout << "\n";
    cout << "======================================================================\n";
    cout << "                 WELCOME TO HYDERABAD METRO CONNECT\n";
    cout << "======================================================================\n";
    cout << "                    • Fast • Safe • Connected\n";
    cout << "----------------------------------------------------------------------\n";

    while (true) {
        cout << "\tLIST OF ACTIONS\n\n";
        cout << "1. LIST ALL THE STATIONS IN THE MAP\n";
        cout << "2. SHOW THE METRO MAP\n";
        cout << "3. GET SHORTEST DISTANCE FROM A 'SOURCE' STATION TO 'DESTINATION' STATION\n";
        cout << "4. GET SHORTEST TIME TO REACH FROM A 'SOURCE' STATION TO 'DESTINATION' STATION\n";
        cout << "5. GET SHORTEST PATH (DISTANCE WISE) TO REACH FROM A 'SOURCE' STATION TO 'DESTINATION' STATION\n";
        cout << "6. GET SHORTEST PATH (TIME WISE) TO REACH FROM A 'SOURCE' STATION TO 'DESTINATION' STATION\n";
        cout << "7. EXIT THE MENU\n";
        cout << "\nENTER YOUR CHOICE FROM THE ABOVE LIST (1 to 7) : ";

        int choice = -1;
        cin >> choice;

        if (choice == 7) {
            break;
        }

        switch (choice) {
        case 1:
            g.display_Stations();
            break;

        case 2:
            g.display_Map();
            break;

        case 3: {
            cout << "Enter the source station: ";
            string sourceStation;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            getline(cin, sourceStation);

            cout << "Enter the destination station: ";
            string destinationStation;
            getline(cin, destinationStation);

            int distance = g.dijkstra(sourceStation, destinationStation, false);
            if (distance < 0) {
                cout << "Path not found" << endl;
            } else {
                cout << "Shortest Distance from " << formatStation(sourceStation)
                     << " to " << formatStation(destinationStation)
                     << " is " << distance << " KM" << endl;
            }
            break;
        }

        case 4: {
            cout << "Enter the source station: ";
            string sourceStation;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            getline(cin, sourceStation);

            cout << "Enter the destination station: ";
            string destinationStation;
            getline(cin, destinationStation);

            int time = g.dijkstra(sourceStation, destinationStation, true);
            if (time < 0) {
                cout << "Path not found" << endl;
            } else {
                double minutes = ceil(static_cast<double>(time) / 60.0);
                cout << "Shortest Time from " << formatStation(sourceStation)
                     << " to " << formatStation(destinationStation)
                     << " is " << minutes << " minutes" << endl;
            }
            break;
        }

        case 5: {
            cout << "Enter the source station: ";
            string sourceStation;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            getline(cin, sourceStation);

            cout << "Enter the destination station: ";
            string destinationStation;
            getline(cin, destinationStation);

            string shortestPath = g.Get_Minimum_Distance(sourceStation, destinationStation);
            cout << "Shortest Path (Distance Wise) from " << formatStation(sourceStation)
                 << " to " << formatStation(destinationStation) << " is:\n"
                 << shortestPath << endl;
            break;
        }

        case 6: {
            cout << "Enter the source station: ";
            string sourceStation;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            getline(cin, sourceStation);

            cout << "Enter the destination station: ";
            string destinationStation;
            getline(cin, destinationStation);

            string shortestPath = g.Get_Minimum_Time(sourceStation, destinationStation);
            cout << "Shortest Path (Time Wise) from " << formatStation(sourceStation)
                 << " to " << formatStation(destinationStation) << " is:\n"
                 << shortestPath << endl;
            break;
        }

        default:
            cout << "Please enter a valid option! " << endl;
            cout << "The options you can choose are from 1 to 7. " << endl;
        }
    }

    return 0;
}