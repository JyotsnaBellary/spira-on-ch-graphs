#include <iostream>
#include "file_handler.hpp"
#include <dijkstra.hpp>
#include <spira.hpp>

using namespace std;

int main() {
    FileHandler fh;
    Graph graph = fh.read_file("./RoadNetworks/osm1.txt");

    // graph.print_adj_simple();


    Dijkstra dijkstra(graph);
    DijkstraResult res = dijkstra.compute_shortest_path(239, 419);
    cout << "Total cost: " << res.total_cost << endl;
    cout << "Number of pops: " << res.number_of_pops << endl;

    Spira spira(graph);
    DijkstraResult res1 = spira.compute_shortest_path(239, 419);
    cout << "Spira Total cost: " << res1.total_cost << endl;
    cout << "Number of pops: " << res1.number_of_pops << endl;
    return 0;
}