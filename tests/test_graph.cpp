#include <gtest/gtest.h>
#include "data_structures/graph.hpp"

// TEST(GraphTest, AddSetAndUpdateEdge) {
//     // 10 nodes => valid IDs: 0..9
//     Graph g(10);

//     // set some node coordinates
//     g.set_node(0, {0, 48.667887, 9.244100});
//     g.set_node(1, {1, 48.667867, 9.244120});
//     g.set_node(2, {2, 48.647887, 9.242100});

//     // prepare space for edges so set_edge() can write by id
//     g.edges.resize(3);

//     // set edges by id; set_edge() also updates adjacency
//     // Edge ctor: Edge(eid, src, trg, cost, is_shortcut=false)
//     Edge edge0(0, 0, 1, 5, false);
//     Edge edge1(1, 1, 2, 7, false);
//     Edge edge2(2, 2, 0, 2, false);
//     g.set_edge(edge0);
//     g.set_edge(edge1);
//     g.set_edge(edge2);
    
//     // verify initial costs
//     EXPECT_EQ(g.get_edge(0).cost, 5);
//     EXPECT_EQ(g.get_edge(1).cost, 7);
//     EXPECT_EQ(g.get_edge(2).cost, 2);

//     // adjacency should have been populated
//     {
//         const auto& nbrs0 = g.neighbors(0);
//         auto it01 = nbrs0.find(1);
//         ASSERT_NE(it01, nbrs0.end());
//         EXPECT_EQ(it01->second.first, 0); // edge id
//         EXPECT_EQ(it01->second.second, 5); // weight
//     }

//     // update an edge weight via adjacency map (keeps min structure)
//     g.update_edge(0, 1, 3);
//     EXPECT_EQ(g.get_edge(0).cost, 3);

//     // adjacency reflects updated weight
//     {
//         const auto& nbrs0 = g.neighbors(0);
//         auto it01 = nbrs0.find(1);
//         ASSERT_NE(it01, nbrs0.end());
//         EXPECT_EQ(it01->second.second, 3);
//     }
// }

// TEST(GraphTest, BuildAdjacencyFromInternalEdges) {
//     Graph g(5);
//     // create 2 edges: 0->1 (4), 1->3 (9)
//     g.edges.resize(2);
//     Edge edge0(0, 0, 1, 4, false);
//     Edge edge1(1, 1, 3, 9, false);
//     g.set_edge(edge0);
//     g.set_edge(edge1);

//     // clear and rebuild adj from g.edges
//     g.build_adjacency_list(nullptr);

//     const auto& n0 = g.neighbors(0);
//     ASSERT_NE(n0.find(1), n0.end());
//     EXPECT_EQ(n0.at(1).first, 0);
//     EXPECT_EQ(n0.at(1).second, 4);

//     const auto& n1 = g.neighbors(1);
//     ASSERT_NE(n1.find(3), n1.end());
//     EXPECT_EQ(n1.at(3).first, 1);
//     EXPECT_EQ(n1.at(3).second, 9);
// }
