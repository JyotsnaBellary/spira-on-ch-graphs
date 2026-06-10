#include<pre-processing/customizable-contraction-hierarchy.hpp>
#include <file-handler.hpp>
#include <kahip-runner.hpp>
#include <random>

using namespace chrono;

CCH::CCH(Graph &graph) : graph(graph) {}

static inline Dist from_unit_double(double u)
{
    // If Dist is integer (e.g., long long), store fixed-point micro-weights.
    const long long SCALE = 1'000'000;
    long long w = llround(u * SCALE);
    if (w <= 0)
        w = 1;
    return static_cast<Dist>(w);
}

void CCH::reset_customization_update_weights(bool assign_random_weights,
                                             uint64_t seed)
{
    mt19937_64 gen(seed);
    // Approximate open interval (0,1): avoid exact 0; 1 is already excluded by default.
    uniform_real_distribution<double> U(nextafter(0.0, 1.0), 1.0);
    // Iterate all upward edges (your shortcutsCache is the upward edge set)
    for (size_t i = 0; i < shortcutsCache.size(); ++i)
    {
        const Edge &edge_pos = shortcutsCache[i];

        Edge edge = graph.get_edge(edge_pos.src, edge_pos.trg);
        Edge rev_edge = graph.get_edge(edge.rev_id);
        if (edge.shortcut)
        {
            edge.cost = rev_edge.cost = INF;
            edge.sc = rev_edge.sc = {INVALID_NODE, INVALID_EDGE, INVALID_EDGE};
            if (edge.replaced)
            {
                edge.cost = rev_edge.cost = edge.original_cost;
                edge.replaced = rev_edge.cost = false;
                // edge.shortcut = rev_edge.shortcut = false;
            }
        }
        else
        {
            // base/original upward edge
            if (assign_random_weights)
            {
                double r = U(gen);            // (0,1)
                Dist w = from_unit_double(r); // or static_cast<Dist>(r) if Dist is double
                edge.cost = rev_edge.cost = w;
            }
        }
    }
}

void CCH::customization(bool default_setting, bool assign_random_weights)
{
    if (!default_setting)
    {
        reset_customization_update_weights(assign_random_weights);
    }

    // coming to customization

    // iterate over shortcuts in shortcut cache
    for (int i = 0; i < shortcut_of_rank_by_trg.size(); i++)
    {
        size_t edge_pos = shortcut_of_rank_by_trg[i];

        Edge e = shortcutsCache[edge_pos];
        Weight best_cost = e.cost;
        NodeId best_node = -1;
        ShortcutInfo best_node_shortcut_info;
        const vector<ShortcutInfo> &lowers = lower_triangle_nodes[edge_pos];
        for (int i = 0; i < lowers.size(); i++)
        {
            NodeId nodeId = lowers[i].middle;
            Edge e_uv = graph.get_edge(lowers[i].e_uv);

            Edge e_vw = graph.get_edge(lowers[i].e_vw);
            // tie-break if you want: prefer smaller x on equal cost
            const Weight via = e_uv.cost + e_vw.cost;

            if (i == 0)
            {
                best_cost = via;
                best_node = nodeId;
                best_node_shortcut_info = lowers[i];
                continue;
            }

            // maybe the second check is not important
            if (via < best_cost || (via == best_cost && rank_of_node[nodeId] < rank_of_node[best_node]))
            {
                best_cost = via;
                best_node = nodeId;
                best_node_shortcut_info = lowers[i];
            }
        }

        if (best_cost > e.cost)
        {
            continue;
        }
        graph.update_shortcut_info(e.id, e.rev_id, best_node_shortcut_info, best_cost, true);
        // cout << "check cost nowwwww" << graph.get_edge(e.id).cost << endl;
    }
    return;
}

CCH_Result CCH::preprocess()
{

    // cout << "number of nodes:" << graph.num_nodes() << endl;
    // cout << "number of edges:" << graph.number_of_edges() << endl;
    auto start_ch = high_resolution_clock::now();
    // vector<int> contraction_order = compute_contraction_order();

    compute_lower_triangles(contraction_order);
    // cout << "Number of triangles to customize: " << shortcutsCache.size() << "\n";

    set_shortcut_rank();
    // ---- compute stats ----
    const size_t M = shortcutsCache.size();
    long long total_triangles = 0;
    int max_triangles = 0;

    for (size_t i = 0; i < M; ++i)
    {
        int t = static_cast<int>(lower_triangle_nodes[i].size());
        total_triangles += t;
        if (t > max_triangles)
            max_triangles = t;
    }

    // avg as integer (rounded to nearest). If you prefer exact, change the field type to double.
    int avg_triangles = M ? static_cast<int>((total_triangles + static_cast<long long>(M) / 2) /
                                             static_cast<long long>(M))
                          : 0;

    CCH_Result cch_result = {shortcuts, avg_triangles, max_triangles};
    return cch_result;
    // return customization();
}

void CCH::print_shortcuts_by_trg_order() const
{

    for (int r = 0; r < shortcut_of_rank_by_trg.size(); r++)
    {
        size_t idx = shortcut_of_rank_by_trg[r];
        const Edge &e = shortcutsCache[idx];
        cout << "rank " << r
                  << " : shortcut (" << e.src << " -> " << e.trg << ")  v=" << e.trg << " r(v) = " << rank_of_node[e.trg]
                  << " | middles: [";

        // find another way
        const auto &mids = lower_triangle_nodes[idx]; // print in stored order
        for (size_t i = 0; i < mids.size(); ++i)
        {
            if (i)
                cout << ", ";
            NodeId w = mids[i].middle;
            cout << w << "(r=" << rank_of_node[w] << ")";
        }

        cout << "]\n";
    }
}
void CCH::set_shortcut_rank()
{

    const size_t M = shortcutsCache.size();
    // 1) build cached signatures of middle-node ranks (DESC)
    mid_rank_sig_desc.assign(M, {});
    for (size_t i = 0; i < M; ++i)
    {
        const auto &mids = lower_triangle_nodes[i];
        auto &sig = mid_rank_sig_desc[i];
        sig.reserve(mids.size());
        for (ShortcutInfo w : mids)
            sig.push_back(rank_of_node[w.middle]);
        sort(sig.begin(), sig.end(), greater<int>());
    }

    // 2) sort an index by (v asc), then lexicographic sig desc
    shortcut_of_rank_by_trg.resize(M);
    iota(shortcut_of_rank_by_trg.begin(), shortcut_of_rank_by_trg.end(), 0);

    auto lex_desc = [&](const vector<int> &A, const vector<int> &B)
    {
        const size_t L = min(A.size(), B.size());
        for (size_t i = 0; i < L; ++i)
        {
            if (A[i] != B[i])
                return A[i] < B[i]; // lower rank wins
        }

        // identical prefix: longer list wins (has a next comparison term)
        if (A.size() != B.size())
            return A.size() > B.size();
        return false; // equal
    };

    stable_sort(shortcut_of_rank_by_trg.begin(), shortcut_of_rank_by_trg.end(),
                     [&](size_t ia, size_t ib)
                     {
                         const Edge &ea = shortcutsCache[ia];
                         const Edge &eb = shortcutsCache[ib];
                    #ifndef NDEBUG
                         assert_upward(ea);
                         assert_upward(eb);
                    #endif
                         if (ea.trg != eb.trg)
                             return rank_of_node[ea.trg] < rank_of_node[eb.trg]; // primary: v ascending
                         if (lex_desc(mid_rank_sig_desc[ia], mid_rank_sig_desc[ib]))
                             return true;
                         if (lex_desc(mid_rank_sig_desc[ib], mid_rank_sig_desc[ia]))
                             return false;
                         //  final tiebreakers for deterministic order:
                         if (ea.src != eb.src)
                             return rank_of_node[ea.src] < rank_of_node[eb.src]; // which have lower src
                         return ia < ib;
                     });

    // 3) build inverse mapping
    rank_of_shortcut_by_trg.assign(M, -1);
    for (size_t r = 0; r < M; ++r)
        rank_of_shortcut_by_trg[shortcut_of_rank_by_trg[r]] = static_cast<int>(r);
}

int CCH::add_shortcuts(const vector<NodeId> &neighbors, NodeId middle_node)
{
    const int k = neighbors.size();
    if (k < 2)
    {
        return 0;
    }
    // Fewer reallocs when degrees are large
    shortcutsCache.reserve(shortcutsCache.size() + k * (k - 1) / 2);
    lower_triangle_nodes.reserve(lower_triangle_nodes.size() + k * (k - 1) / 2);

    int new_edges = 0;
    for (auto i = 0; i < k; i++)
    {
        NodeId node1 = neighbors[i];
        for (auto j = i + 1; j < k; j++)
        {
            NodeId node2 = neighbors[j];

            if (node1 == node2)
                continue;

            uint64_t key = pair_key(node1, node2);
            auto it = shortcutPos.find(key);

            // add noth middle_node to node1 and middle node to node 2
            // ensure orientation is correct else get rever edge.
            EdgeId edge_uv = graph.get_edge(node1, middle_node).id;
            EdgeId edge_vw = graph.get_edge(middle_node, node2).id;
            ShortcutInfo shortcutInfo = {middle_node, edge_uv, edge_vw};
            // auto it1 = shortcutPositionLookup.find(node1);
            if (it != shortcutPos.end())
            {

                lower_triangle_nodes[it->second].push_back(shortcutInfo);
                continue;
            }

            if (!graph.edge_exists(node1, node2))
            {

                Edge edge = {graph.number_of_edges(), graph.number_of_edges() + 1, node1, node2, INF_WEIGHT, true, true};
                Edge rev_edge = {edge.rev_id, edge.id, node2, node1, INF_WEIGHT, true, true};
                graph.set_edge(edge, rev_edge);
                new_edges += 1;
            }

            // create new shortcut record
            // should not be doing this. should not be considering your new neighbors. find another wa to store your shortcuts
            const Edge &e = graph.get_edge(node1, node2); // O(1)
            shortcutsCache.emplace_back(e);
            size_t pos = shortcutsCache.size() - 1;
            shortcutPos.emplace(key, pos);
            // start triangle list with one element efficiently
            lower_triangle_nodes.emplace_back(1, shortcutInfo);
        }
    }

    return new_edges;
}

void CCH::compute_lower_triangles(const vector<NodeId> &contraction_order)
{
    for (int i = 0; i < contraction_order.size(); i++)
    {
        NodeId nodeId = contraction_order[i];
        vector<NodeId> neighbors = graph.get_sorted_higher_neighbors(nodeId, rank_of_node);
        shortcuts += add_shortcuts(neighbors, nodeId);
        graph.deactivate(nodeId);
    }
    // cout << "Shortcuts Added: " << shortcuts << endl;
    return;
}

void CCH::compute_contraction_order()
{

    vector<int> reverse_contracted_nodes;

    FileHandler fh;

    fh.write_to_metis_format(graph, "./Input_Data/test.graph");

    string output_file_path = "";
    try
    {
        output_file_path = run_kahip_on_test_graph("level2");
    }
    catch (const exception &e)
    {
        cerr << "Error: " << e.what() << "\n";
        return;
    }

    Ordering ordering = fh.read_kahip_output(output_file_path, graph.number_of_nodes());
    contraction_order = ordering.node_of_rank;

    rank_of_node = ordering.rank_of_node;
    // return contraction_order;
}
