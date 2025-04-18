#include <iostream>
#include <vector>
#include <climits>
#include <unordered_map>
#include <map>
#include <stack>
#include <algorithm>
#include <limits>
#include <fstream>
#include "json.hpp"

using namespace std;
using json = nlohmann::json;

enum class TimeOfDay { Morning, Afternoon, Evening };

struct Traffic {
    unordered_map<TimeOfDay, int> weights;

    int getWeight(TimeOfDay t) const {
        return weights.at(t);
    }
};

struct Edge {
    int u, v, baseWeight;
    Traffic traffic;
};

class Graph {
private:
    int V;
    unordered_map<int, vector<Edge>> adjList;
    unordered_map<int, string> cityNames;
    unordered_map<string, int> nameToId;

public:
    Graph(int vertices) : V(vertices) {}

    void addCity(int id, const string& name) {
        cityNames[id] = name;
        nameToId[name] = id;
    }

    void addEdge(int u, int v, int baseWeight, Traffic traffic) {
        adjList[u].push_back({u, v, baseWeight, traffic});
        adjList[v].push_back({v, u, baseWeight, traffic});
    }

    void loadFromJson(const string& filename) {
        ifstream file(filename);
        json data;
        file >> data;

        for (auto& city : data["cities"]) {
            addCity(city["id"], city["name"]);
        }

        for (auto& edge : data["edges"]) {
            Traffic t{{
                {TimeOfDay::Morning, edge["traffic"]["morning"]},
                {TimeOfDay::Afternoon, edge["traffic"]["afternoon"]},
                {TimeOfDay::Evening, edge["traffic"]["evening"]},
            }};
            addEdge(edge["u"], edge["v"], edge["base"], t);
        }
    }

    void displayMap() {
        cout << "\n📌 Graph Layout:\n";
        for (const auto& pair : adjList) {
            for (const auto& edge : pair.second) {
                cout << cityNames[edge.u] << " -> " << cityNames[edge.v]
                     << " (Base: " << edge.baseWeight
                     << ", Traffic (Morning): " << edge.traffic.getWeight(TimeOfDay::Morning)
                     << ", Afternoon: " << edge.traffic.getWeight(TimeOfDay::Afternoon)
                     << ", Evening: " << edge.traffic.getWeight(TimeOfDay::Evening) << ")\n";
            }
        }
    }

    vector<int> bellmanFord(int start, vector<int>& parent, TimeOfDay timeOfDay) {
        vector<int> dist(V, numeric_limits<int>::max());
        parent.assign(V, -1);
        dist[start] = 0;

        for (int i = 1; i <= V - 1; ++i) {
            for (const auto& pair : adjList) {
                for (const auto& edge : pair.second) {
                    int totalWeight = edge.baseWeight + edge.traffic.getWeight(timeOfDay);
                    if (dist[edge.u] != numeric_limits<int>::max() && dist[edge.u] + totalWeight < dist[edge.v]) {
                        dist[edge.v] = dist[edge.u] + totalWeight;
                        parent[edge.v] = edge.u;
                    }
                }
            }
        }

        return dist;
    }

    void printPath(int start, int end, const vector<int>& parent) {
        vector<int> path;
        for (int at = end; at != -1; at = parent[at]) path.push_back(at);
        reverse(path.begin(), path.end());

        if (path.front() != start) {
            cout << "No path found.\n";
            return;
        }

        cout << "🛣️  Path: ";
        for (size_t i = 0; i < path.size(); ++i) {
            cout << cityNames[path[i]];
            if (i + 1 != path.size()) cout << " -> ";
        }
        cout << endl;
    }

    TimeOfDay parseTime(const string& timeStr) {
        if (timeStr == "morning") return TimeOfDay::Morning;
        if (timeStr == "afternoon") return TimeOfDay::Afternoon;
        if (timeStr == "evening") return TimeOfDay::Evening;
        throw invalid_argument("Invalid time of day.");
    }

    void userInterface() {
        while (true) {
            string startName, endName, timeStr;
            cout << "\nEnter start city name (or 'exit' to quit): ";
            cin >> startName;
            if (startName == "exit") break;

            cout << "Enter destination city name: ";
            cin >> endName;

            cout << "Enter time of day (morning/afternoon/evening): ";
            cin >> timeStr;

            if (nameToId.find(startName) == nameToId.end() || nameToId.find(endName) == nameToId.end()) {
                cout << "❌ Invalid city name(s). Try again.\n";
                continue;
            }

            int start = nameToId[startName];
            int end = nameToId[endName];
            TimeOfDay timeOfDay;
            try {
                timeOfDay = parseTime(timeStr);
            } catch (...) {
                cout << "❌ Invalid time of day. Try again.\n";
                continue;
            }

            vector<int> parent;
            vector<int> dist = bellmanFord(start, parent, timeOfDay);

            if (dist[end] == numeric_limits<int>::max()) {
                cout << "No route found from " << startName << " to " << endName << endl;
            } else {
                cout << "🚗 Shortest distance (with traffic for " << timeStr << "): " << dist[end] << endl;
                printPath(start, end, parent);
            }
        }
    }
};

int main() {
    Graph g(10);
    g.loadFromJson("/Users/deepinderdeep/Documents/DSA Project - Route and Recomm System/DSA Project - Route and Recomm System/cities_and_edges.json");
    g.displayMap();
    g.userInterface();
    return 0;
}
