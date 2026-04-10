#include "benchmark_tests.hpp"
#include <chrono>

using namespace std;
using namespace filesystem;
using Clock = chrono::high_resolution_clock;

// Take the average of 100 runs
void BenchmarkTests::finalize_aggregate_stats(AggregateStats& agg) {
    int R = agg.runs;
    if (R <= 0) return;

    // ---- Times ----
    agg.avg_d_time  /= R;
    agg.avg_s_time  /= R;
    agg.avg_nv_time /= R;

    // ---- Pops ----
    agg.avg_d_pops  /= R;
    agg.avg_s_pops  /= R;
    agg.avg_nv_pops /= R;
    agg.avg_nvq_pops /= R;

    // ---- Pertinence counts ----
    agg.avg_in      /= R;
    agg.avg_in_trans/= R;
    agg.avg_out     /= R;
    agg.avg_pert    /= R;

    // ---- Ratios ----
    agg.avg_ratio_pert_m    /= R;
    agg.avg_ratio_pert_n    /= R;
    agg.avg_ratio_in_trans  /= R;
    agg.avg_ratio_spt_out   /= R;
    agg.avg_ratio_spt_in    /= R;
    agg.avg_ratio_pop_pert  /= R;
}

// Append results to average.csv
void BenchmarkTests::append_average_summary(const AggregateStats& A, const string& output_csv_path) {
    namespace fs = filesystem;

    // Extract directory: e.g. ".../spt_benchmark/"
    fs::path out_path(output_csv_path);
    fs::path dir = out_path.parent_path(); 

    // Build the final path: ".../spt_benchmark/average.csv"
    fs::path avg_path = dir / "average.csv";
    bool exists = fs::exists(avg_path);


    ofstream out(avg_path, ios::app);


    if (!exists) {
        out << "filename,n,m,runs,"
            << "avg_d_time,avg_s_time,avg_nv_time,"
            << "avg_d_pops,avg_s_pops,avg_nv_pops,avg_nvq_pops,"
            << "avg_in,avg_in_trans,avg_out,avg_pert,"
            << "ratio_pert_m,ratio_pert_n,ratio_in_trans,ratio_spt_out,ratio_spt_in,ratio_pop_pert,"
            << "runs_with_conflict,runs_with_misclassified_spt,mismatches\n";
    }

    // Extract only filename without extension
    string filename = out_path.stem().string();  // "dj38"

    out << filename << ","
        << A.n << "," << A.m << "," << A.runs << ","
        << A.avg_d_time << "," << A.avg_s_time << "," << A.avg_nv_time << ","
        << A.avg_d_pops << "," << A.avg_s_pops << "," << A.avg_nv_pops << "," << A.avg_nvq_pops << ","
        << A.avg_in << "," << A.avg_in_trans << "," << A.avg_out << "," << A.avg_pert << ","
        << A.avg_ratio_pert_m << "," << A.avg_ratio_pert_n << "," << A.avg_ratio_in_trans << ","
        << A.avg_ratio_spt_out << "," << A.avg_ratio_spt_in << "," << A.avg_ratio_pop_pert << ","
        << A.runs_with_conflict << "," << A.runs_with_misclassified_spt << "," << A.mismatches
        << "\n";
}

// Analyse and gather pertinence statistics for SPT
PertinenceStats BenchmarkTests::analyze_spt_pertinence(
    const SsspResult &res,
    const Graph &graph)
{
    PertinenceStats stats;

    for (EdgeId eid = 0; eid < (EdgeId)graph.number_of_edges(); ++eid)
    {
        Edge edge = graph.get_edge(eid);

        // Check if an edge is marked as in and out pertinent
        if (res.in_pertinent_edges[eid] and res.out_pertinent_edges[eid])
        {
            cerr << "Warning: edge " << eid
                 << " is marked both in-pertinent and out-pertinent.\n";
            stats.conflict_both += 1;
        }
        else if (res.in_pertinent_edges[eid])
        {
            // Check if edge is in-pertinent
            if (edge.cost < 2*(res.distance[edge.trg] - res.median)) {
                stats.total_in_pertinent_edges += 1;

                // Check if the in-pertinent edge was later added to priority queue P
                if (res.in_pertinent_edges_extracted_in_forward_phase[eid])
                {
                    stats.total_in_pertinent_extracted_in_forward += 1;
                }

            }
            else {
                cout << "Edge incorrectly marked as in-pertinent" << endl;
            }            
        }
        // Check if an edge is out-pertinent
        else if (res.out_pertinent_edges[eid])
        {
            if (edge.cost <= 2*(res.median - res.distance[edge.src])) {
                stats.total_out_pertinent_edges += 1;
            }
            else {
                cout << "Edge incorrectly marked as out-pertinent" << endl;
            }

        }
    }

    // Track total number of pertinent edges
    stats.total_pertinent_edges = stats.total_in_pertinent_edges + stats.total_out_pertinent_edges;
    // cout << "Analyzing SPT edges for pertinence...\n";
    for (NodeId u = 0; u < (NodeId)res.via_edge.size(); ++u)
    {
        EdgeId eid = res.via_edge[u];

        // skip root or unreachable
        if (eid == INVALID_EDGE || eid == -1)
            continue;

        stats.total_spt_edges++;

        bool is_out = res.out_pertinent_edges[eid];
        bool is_in = res.in_pertinent_edges[eid];

        if (is_out)
            stats.out_spt++;
        if (is_in)
            stats.in_spt++;

        // SPT edge is neither in nor out
        if (!is_out && !is_in)
        {
            cout << "WARNING: SPT edge " << eid
                 << " (" << graph.get_edge(eid).src << " -> "
                 << graph.get_edge(eid).trg
                 << ") is NOT pertinent.\n";

            stats.non_pertinent_edge_in_spt++;
        }
    }

    // Mark if non-pertinent edges are included in SPT
    stats.spt_edges_incorrectly_classified = (stats.non_pertinent_edge_in_spt > 0);

    // ----- Ratios -----
    if (graph.number_of_edges() > 0)
    {
        // ratio of pertinent edges to total edges in a graph instance
        stats.ratio_pert_m = (double)stats.total_pertinent_edges / (double)graph.number_of_edges();
    }
    if (graph.number_of_nodes() > 0)
    {
        // ratio of pertinent edges to total edges in a graph instance
        stats.ratio_pert_n = (double)stats.total_pertinent_edges / (double)graph.number_of_nodes();
    }
    if (stats.total_in_pertinent_edges > 0)
    {
        // ratio of in pertinent edges transferred to priority Queue P
        stats.ratio_in_transferred =
            (double)stats.total_in_pertinent_extracted_in_forward /
            (double)stats.total_in_pertinent_edges;
    }
    if (stats.total_spt_edges > 0)
    {
        // ratio of out-pertinent edges to all pertinent edges
        stats.ratio_spt_out = (double)stats.out_spt / (double)stats.total_spt_edges;

        // ratio of in-pertinent edges to all pertinent edges
        stats.ratio_spt_in = (double)stats.in_spt / (double)stats.total_spt_edges;
    }

    return stats;
}

// Analyse and gather pertinence statistics for a path
PertinenceStats BenchmarkTests::analyze_path_pertinence(const SsspResult &res, const Graph &graph)
{
    PertinenceStats stats;

    for (EdgeId eid = 0; eid < (EdgeId)graph.number_of_edges(); ++eid)
    {

        // Check if an edge is marked as in and out pertinent
        if (res.in_pertinent_edges[eid] and res.out_pertinent_edges[eid])
        {
            cerr << "Warning: edge " << eid
                 << " is marked both in-pertinent and out-pertinent.\n";
            stats.conflict_both += 1;
        }
        else if (res.in_pertinent_edges[eid])
        {
            // count as in-pertinent
            stats.total_in_pertinent_edges += 1;
            if (res.in_pertinent_edges_extracted_in_forward_phase[eid])
            {
                stats.total_in_pertinent_extracted_in_forward += 1;
            }
        }
        else if (res.out_pertinent_edges[eid])
        {
            // count as out-pertinent
            stats.total_out_pertinent_edges += 1;
        }
    }

    // Track total number of pertinent edges
    stats.total_pertinent_edges = stats.total_in_pertinent_edges + stats.total_out_pertinent_edges;

    for (EdgeId eid : res.edge_ids)
    {
        bool is_out = res.out_pertinent_edges[eid];
        bool is_in = res.in_pertinent_edges[eid];

        // skip root or unreachable
        if (eid == INVALID_EDGE)
            continue;

        stats.total_spt_edges++;
        if (is_out)
            stats.out_spt++;
        if (is_in)
            stats.in_spt++;

        // SPT edge is neither in nor out
        if (!is_out && !is_in)
        {
            const Edge &e = graph.get_edge(eid);
            cout << "WARNING: Path edge " << eid
                 << " (" << e.src << " -> " << e.trg
                 << ") is NOT pertinent.\n";
            stats.non_pertinent_edge_in_spt++;
        }
    }

    // Mark if non-pertinent edges are included in SPT
    stats.spt_edges_incorrectly_classified = (stats.non_pertinent_edge_in_spt > 0);

    // ----- Ratios -----
    if (graph.number_of_edges() > 0)
    {
        // ratio of pertinent edges to total edges in a graph instance
        stats.ratio_pert_m = (double)stats.total_pertinent_edges / (double)graph.number_of_edges();
    }
    if (graph.number_of_nodes() > 0)
    {
        // ratio of pertinent edges to total edges in a graph instance
        stats.ratio_pert_n = (double)stats.total_pertinent_edges / (double)graph.number_of_nodes();
    }
    if (stats.total_in_pertinent_edges > 0)
    {
        stats.ratio_in_transferred =
            (double)stats.total_in_pertinent_extracted_in_forward /
            (double)stats.total_in_pertinent_edges;
    }
    if (stats.total_spt_edges > 0)
    {
        stats.ratio_spt_out = (double)stats.out_spt / (double)stats.total_spt_edges;
        stats.ratio_spt_in = (double)stats.in_spt / (double)stats.total_spt_edges;
    }

    return stats;
}

// ---- Helper: Cost if cost mismatches between two src-dst queries ----
bool BenchmarkTests::same_cost(Cost a, Cost b)
{
    // If one of the costs is "unreachable"
    if (a < 0 || b < 0)
        return a == b;

    return fabs(a - b) <= 1e-9;
}

vector<pair<int, int>> BenchmarkTests::generate_query_pairs(int n)
{

    // ----- Generate 100 unique (src, dst) pairs -----
    mt19937 rng(62); // fixed seed for reproducibility
    uniform_int_distribution<int> dist(0, n - 1);

    // encode (src,dst) as 64-bit to ensure uniqueness
    vector<pair<int, int>> query_pairs;
    query_pairs.reserve(100);
    set<long long> used; 

    auto encode = [](int s, int t) -> long long
    {
        return (static_cast<long long>(static_cast<unsigned int>(s)) << 32) | static_cast<unsigned int>(t);
    };

    // add 100 pairs to query pairs
    while (static_cast<int>(query_pairs.size()) < 100)
    {
        int s = dist(rng);
        int t = dist(rng);
        if (s == t)
            continue;
        long long key = encode(s, t);
        if (used.insert(key).second)
        {
            query_pairs.emplace_back(s, t);
        }
    }

    return query_pairs;
}

// ---- Main Benchmark ----
void BenchmarkTests::run_src_dst_benchmark_on_graph(Graph &graph, const string &output_csv_path)
{
    // generate a 100 unique query pairs
    const int n = graph.number_of_nodes();
    vector<pair<int, int>> query_pairs = generate_query_pairs(n);

    // Initialize algorithms
    Dijkstra dijkstra(graph);
    Spira spira(graph);
    NewVariant new_variant(graph);

    // CSV Header
    ofstream out(output_csv_path);
    out << boolalpha;
    out << "src,dst,d_time_us,d_cost,s_time_us,s_cost,nv_time_us,nv_cost,"
        << "d_pops,s_pops,nv_pops,nv_q_pops,"
        << "d_avg_pops_per_node,s_avg_pops_per_node,nv_avg_pops_per_node,"
        << "in_pert,in_pert_trans,out_pert,pert,pert_conflicts,"
        << "out_spt,in_spt,non_pert_spt,total_spt,classified_incorrectly,"
        << "ratio_pert_m,ratio_pert_n,ratio_in_transferred,ratio_spt_out,ratio_spt_in,ratio_pop_pert,"
        << "matched\n";

    AggregateStats agg;
    agg.filepath = output_csv_path;
    agg.n = graph.number_of_nodes();
    agg.m = graph.number_of_edges();
    agg.runs = query_pairs.size();

    // Run 100 random SPT queries
    cout << "Running 100 src-dst queries..." << endl;
    int mismatch = 0;
    for (auto [src, dst] : query_pairs)
    {
        // === Dijkstra ===
        auto dijkstra_start = Clock::now();
        SsspResult result_dijkstra = dijkstra.compute_shortest_path(src, dst);
        auto dijkstra_end = Clock::now();
        long long dijkstra_time_us = chrono::duration_cast<chrono::microseconds>(dijkstra_end - dijkstra_start).count();
        Cost dijkstra_cost = result_dijkstra.total_cost;

        // === Spira ===
        auto spira_start = Clock::now();
        SsspResult result_spira = spira.compute_shortest_path(src, dst);
        auto spira_end = Clock::now();
        long long spira_time_us = chrono::duration_cast<chrono::microseconds>(spira_end - spira_start).count();
        Cost spira_cost = result_spira.total_cost;

        // === New Variant ===
        auto new_variant_start = Clock::now();
        SsspResult result_new_variant = new_variant.compute_shortest_path(src, dst);
        auto new_variant_end = Clock::now();
        long long new_variant_time_us = chrono::duration_cast<chrono::microseconds>(new_variant_end - new_variant_start).count();
        Cost new_variant_cost = result_new_variant.total_cost;

        // Check if the results of all three algorithms match
        bool matched = same_cost(dijkstra_cost, spira_cost) && same_cost(dijkstra_cost, new_variant_cost);

        // Count as mismatch if results are wrong 
        if (!matched)
        {
            cout << "Warning: cost mismatch for src=" << src << ", dst=" << dst << "\n";
            for (auto i = 0; i < result_new_variant.path.size(); i++)
            {
                cout << result_new_variant.path[i] << " ";
            }
            mismatch++;
        }

        // Analyze pertinence edges for src dst query
        PertinenceStats stats = analyze_path_pertinence(result_new_variant, graph);
 
        stats.ratio_pop_pert = (stats.total_pertinent_edges > 0) ? (double)(result_new_variant.number_of_pops) / (double)(stats.total_pertinent_edges) : 0.0;
        
        // Log any suspecious pertinence statistics
        if (stats.conflict_both != 0)
        {
            cout << "Conflicts in pertinence classification.\n";
        }

        if (stats.spt_edges_incorrectly_classified)
        {
            cout << "SPT edges classified not classified correctly.\n";
        }

        // aggregate stats
        agg.avg_d_time += dijkstra_time_us;
        agg.avg_s_time += spira_time_us;
        agg.avg_nv_time += new_variant_time_us;

        agg.avg_d_pops += result_dijkstra.number_of_pops;
        agg.avg_s_pops += result_spira.number_of_pops;
        agg.avg_nv_pops += result_new_variant.number_of_pops;
        agg.avg_nvq_pops += result_new_variant.number_of_Q_pops;

        agg.avg_in += stats.total_in_pertinent_edges;
        agg.avg_in_trans += stats.total_in_pertinent_extracted_in_forward;
        agg.avg_out += stats.total_out_pertinent_edges;
        agg.avg_pert += stats.total_pertinent_edges;

        agg.avg_ratio_pert_m += stats.ratio_pert_m;
        agg.avg_ratio_pert_n += stats.ratio_pert_n;
        agg.avg_ratio_in_trans += stats.ratio_in_transferred;
        agg.avg_ratio_spt_out += stats.ratio_spt_out;
        agg.avg_ratio_spt_in += stats.ratio_spt_in;
        agg.avg_ratio_pop_pert += stats.ratio_pop_pert;

        // suspicious runs
        if (stats.conflict_both > 0)
            agg.runs_with_conflict++;
        if (stats.spt_edges_incorrectly_classified)
            agg.runs_with_misclassified_spt++;

        // mismatches
        if (!matched)
            agg.mismatches++;

        // === Write one CSV row ===
        out << src << ',' << dst << ','
            << dijkstra_time_us << ',' << dijkstra_cost << ',' << spira_time_us << ',' << spira_cost << ',' << new_variant_time_us << ',' << new_variant_cost << ','
            << result_dijkstra.number_of_pops << ',' << result_spira.number_of_pops << ',' << result_new_variant.number_of_pops << ',' << result_new_variant.number_of_Q_pops << ','
            << result_dijkstra.avg_pops_per_node << ',' << result_spira.avg_pops_per_node << ',' << result_new_variant.avg_pops_per_node << ','
            << stats.total_in_pertinent_edges << ',' << stats.total_in_pertinent_extracted_in_forward << ',' << stats.total_out_pertinent_edges << ',' << stats.total_pertinent_edges << ',' << stats.conflict_both << ','
            << stats.out_spt << ',' << stats.in_spt << ',' << stats.non_pertinent_edge_in_spt << ',' << stats.total_spt_edges << ',' << stats.spt_edges_incorrectly_classified << ','
            << stats.ratio_pert_m << ',' << stats.ratio_pert_n << ',' << stats.ratio_in_transferred << ',' << stats.ratio_spt_out << ',' << stats.ratio_spt_in << ',' << stats.ratio_pop_pert << ','
            << matched << '\n';
    }
    cout << "Number of Mismatched Queries: " << mismatch << endl;
    out.close();

    // Calculate Average and write to average.csv
    finalize_aggregate_stats(agg);
    append_average_summary(agg, output_csv_path);
}

// ---- Helper: Count node-wise mismatches between two SPTs ----
int BenchmarkTests::count_distance_mismatches(const vector<Cost> &a,
                                              const vector<Cost> &b)
{
    int mismatches = 0;
    for (size_t i = 0; i < a.size(); ++i)
    {
        // Count as mismatch if the distance from source don't match between vector and vector b
        if (fabs(a[i] - b[i]) > 1e-9) 
            mismatches++;
    }
    return mismatches;
}

// ---- Helper: Compare SPT equality ----
bool BenchmarkTests::compare_spt_results(const SsspResult &result_a,
                                         const SsspResult &result_b,
                                         const string &nameA,
                                         const string &nameB)
{
    // Must have distance values for the same number of vertices
    if (result_a.distance.size() != result_b.distance.size())
        return false;

    // Compare vertex by vertex
    for (size_t i = 0; i < result_a.distance.size(); ++i)
    {
        // Store results of algorithm a and algorithm b
        Cost dist_a = result_a.distance[i];
        Cost dist_b = result_a.distance[i];

        // Check 3 cases
        // 1. Both distances are finite, but differ by more than tolerance
        if ((dist_a < INF_COST && dist_b < INF_COST && fabs(dist_a - dist_b) > 1e-9) ||
            // A has node as unreachable and B has node as reachable
            (dist_a == INF_COST && dist_b != INF_COST) ||
            //  B has node as unreachable and A has node as reachable
            (dist_b == INF_COST && dist_a != INF_COST))
        {
            // Log algprithm names where results don't match
            cout << "Distance mismatch at node " << i << ": "
                 << nameA << " has " << dist_a << ", "
                 << nameB << " has " << dist_b << "\n";
            return false;
        }
    }

    // All distances matched
    return true;
}

// ---- Main Benchmark ----
void BenchmarkTests::run_spt_benchmark_on_graph(Graph &graph, const string &output_csv_path)
{
    const int n = graph.number_of_nodes();
    const int num_sources = min(n, 100);

    // --- Select 100 unique random sources ---
    vector<int> sources;
    sources.reserve(num_sources);

    mt19937 rng(42); // fixed seed for reproducibility
    uniform_int_distribution<int> dist(0, n - 1);

    // Select 100 random source vertices
    unordered_set<int> used;
    while (sources.size() < num_sources)
    {
        int src = dist(rng);
        if (used.insert(src).second)
            sources.push_back(src);
    }

    // Initialize teh SSSP algorithms
    Dijkstra dijkstra(graph);
    Spira spira(graph);
    NewVariant new_variant(graph);

    // CSV Header
    ofstream out(output_csv_path);
    out << "src,"
        << "d_time_us,s_time_us,nv_time_us,"
        << "d_pops,s_pops,nv_pops,nv_q_pops,"
        << "d_avg_pops_per_node,s_avg_pops_per_node,nv_avg_pops_per_node,"
        << "in_pert,in_pert_trans,out_pert,pert,pert_conflicts,"
        << "out_spt,in_spt,non_pert_spt,total_spt,classified_incorrectly,"
        << "ratio_pert_m,ratio_pert_n,ratio_in_transferred,ratio_spt_out,ratio_spt_in,ratio_pop_pert,"
        << "mismatch_count_djk_spi,mismatch_count_djk_new,"
        << "mismatches\n";

    AggregateStats agg;
    agg.filepath = output_csv_path;
    agg.n = graph.number_of_nodes();
    agg.m = graph.number_of_edges();
    agg.runs = sources.size();

    // Run 100 random SPT queries
    cout << "Running " << num_sources << " SPT queries..." << endl;
    int mismatch_runs = 0;

    for (int src : sources)
    {
        // === Dijkstra ===
        auto dijkstra_start = Clock::now();
        SsspResult result_dijkstra = dijkstra.compute_shortest_path(src, -1);
        auto dijkstra_end = Clock::now();
        long long dijkstra_time_us =
            chrono::duration_cast<chrono::microseconds>(dijkstra_end - dijkstra_start).count();

        // === Spira ===
        auto spira_start = Clock::now();
        SsspResult result_spira = spira.compute_shortest_path(src, -1);
        auto spira_end = Clock::now();
        long long spira_time_us =
            chrono::duration_cast<chrono::microseconds>(spira_end - spira_start).count();

        // === New Variant ===
        auto new_variant_start = Clock::now();
        SsspResult result_new_variant = new_variant.compute_shortest_path(src, -1);
        auto new_variant_end = Clock::now();
        long long new_variant_time_us =
            chrono::duration_cast<chrono::microseconds>(new_variant_end - new_variant_start).count();


        // === Compare distances ===
        bool match_dijkstra_spira = compare_spt_results(result_dijkstra, result_spira, "Dijkstra", "Spira");
        bool match_dijkstra_new_variant = compare_spt_results(result_dijkstra, result_new_variant, "Dijkstra", "NewVariant");

        // Track number of distance mismatches from source to every other node 
        int mismatch_count_ds = count_distance_mismatches(result_dijkstra.distance, result_spira.distance);
        int mismatch_count_dn = count_distance_mismatches(result_dijkstra.distance, result_new_variant.distance);

        // If there are mismatches, results are wrong and therefore the implementation is wrong
        bool mismatches = !(match_dijkstra_spira && match_dijkstra_new_variant);
        if (mismatches)
            mismatch_runs++;

        // cout  << "finished run for src: " << src << endl;
        // cout << "analysis" << endl;

        // Analyze pertinence edges for SPT query
        PertinenceStats stats = analyze_spt_pertinence(result_new_variant, graph);

        // Log any suspecious pertinence statistics
        stats.ratio_pop_pert = (stats.total_pertinent_edges > 0) ? (double)(result_new_variant.number_of_pops) / (double)(stats.total_pertinent_edges) : 0.0;
        if (stats.conflict_both != 0)
        {
            cout << "conflicts in pertinence classification.\n";
        }
        if (stats.spt_edges_incorrectly_classified)
        {
            cout << "SPT edges not classified correctly.\n";
        }

        // aggregate stats
        agg.avg_d_time += dijkstra_time_us;
        agg.avg_s_time += spira_time_us;
        agg.avg_nv_time += new_variant_time_us;

        agg.avg_d_pops += result_dijkstra.number_of_pops;
        agg.avg_s_pops += result_spira.number_of_pops;
        agg.avg_nv_pops += result_new_variant.number_of_pops;
        agg.avg_nvq_pops += result_new_variant.number_of_Q_pops;

        agg.avg_in += stats.total_in_pertinent_edges;
        agg.avg_in_trans += stats.total_in_pertinent_extracted_in_forward;
        agg.avg_out += stats.total_out_pertinent_edges;
        agg.avg_pert += stats.total_pertinent_edges;

        agg.avg_ratio_pert_m += stats.ratio_pert_m;
        agg.avg_ratio_pert_n += stats.ratio_pert_n;
        agg.avg_ratio_in_trans += stats.ratio_in_transferred;
        agg.avg_ratio_spt_out += stats.ratio_spt_out;
        agg.avg_ratio_spt_in += stats.ratio_spt_in;
        agg.avg_ratio_pop_pert += stats.ratio_pop_pert;

        // suspicious runs
        if (stats.conflict_both > 0)
            agg.runs_with_conflict++;
        if (stats.spt_edges_incorrectly_classified)
            agg.runs_with_misclassified_spt++;

        // mismatches
        if (mismatches)
            agg.mismatches++;

        // === Write one CSV row ===
        out << src << ','
            << dijkstra_time_us << ',' << spira_time_us << ',' << new_variant_time_us << ','
            << result_dijkstra.number_of_pops << ',' << result_spira.number_of_pops << ',' << result_new_variant.number_of_pops << ',' << result_new_variant.number_of_Q_pops << ','
            << result_dijkstra.avg_pops_per_node << ',' << result_spira.avg_pops_per_node << ',' << result_new_variant.avg_pops_per_node << ','
            << stats.total_in_pertinent_edges << ',' << stats.total_in_pertinent_extracted_in_forward << ',' << stats.total_out_pertinent_edges << ',' << stats.total_pertinent_edges << ',' << stats.conflict_both << ','
            << stats.out_spt << ',' << stats.in_spt << ',' << stats.non_pertinent_edge_in_spt << ',' << stats.total_spt_edges << ',' << stats.spt_edges_incorrectly_classified << ','
            << stats.ratio_pert_m << ',' << stats.ratio_pert_n << ',' << stats.ratio_in_transferred << ',' << stats.ratio_spt_out << ',' << stats.ratio_spt_in << ',' << stats.ratio_pop_pert << ','
            << mismatch_count_ds << ',' << mismatch_count_dn << ','
            << boolalpha << mismatches << '\n';
    }

    out.close();

    // cout << "Benchmark complete.\n";
    cout << "Mismatches in " << mismatch_runs << " / " << num_sources << " runs.\n";
    // cout << "Results saved to " << output_csv_path << endl;
    cout << endl;

    // Calculate and save average over a 100 queries
    finalize_aggregate_stats(agg);
    append_average_summary(agg, output_csv_path);
}

// ---- Wrapper to read and run ----
void BenchmarkTests::process_sparse_graph_file(const string &filepath, WeightMode weight_mode, string &output_dir)
{
    // Read Sparse graph instance
    FileHandler fh;
    Graph graph = fh.read_sparse_graph_file(filepath, weight_mode);

    // Sort neighbors
    graph.sort_all_neighbors();

    string output_src_dst = output_dir + "/src_dst_benchmark";
    create_directories(output_src_dst); // make sure directory exists

    // Get file name
    string filename = path(filepath).stem().string();
    string output_csv = output_src_dst + "/" + filename + ".csv";

    cout << "Running src-dst benchmark -> " << output_csv << endl;
    run_src_dst_benchmark_on_graph(graph, output_csv);

    string output_spt = output_dir + "/spt_benchmark";
    create_directories(output_spt); // make sure directory exists
    
    // Get file name
    filename = path(filepath).stem().string(); 
    output_csv = output_spt + "/" + filename + ".csv";
    
    cout << endl;
    cout << "Running spt benchmark -> " << output_csv << endl;
    run_spt_benchmark_on_graph(graph, output_csv);
}

// ---- Wrapper to read and run ----
void BenchmarkTests::process_dense_graph_file(const string &filepath, WeightMode weight_mode, string &output_dir)
{
    FileHandler fh;
    Graph graph = fh.read_dense_graph_file(filepath, weight_mode);
    graph.sort_all_neighbors();

    string output_dir1 = output_dir + "/src_dst_benchmark";
    create_directories(output_dir1); // make sure directory exists

    // Get file name
    string filename = path(filepath).stem().string(); 
    string output_csv = output_dir1 + "/" + filename + ".csv";

    cout << "Running src-dst benchmark -> " << output_csv << endl;
    run_src_dst_benchmark_on_graph(graph, output_csv);

    string output_spt = output_dir + "/spt_benchmark";
    create_directories(output_spt); // make sure directory exists

    // Get file name
    filename = path(filepath).stem().string(); 
    output_csv = output_spt + "/" + filename + ".csv";

    cout << endl;
    cout << "Running spt benchmark -> " << output_csv << endl;
    run_spt_benchmark_on_graph(graph, output_csv);
}

// Calls for Benchmarking on Sparse Graphs with different edge-weight distributions
int BenchmarkTests::run_benchmark_on_sparse_graphs()
{
    string input_dir = "./Input_Data/SparseRoadNetworks";
    string output_dir_random = "output/sparse_networks/random_weights";
    string output_dir_exponential = "output/sparse_networks/exponential_weights";
    string output_dir_original = "output/sparse_networks/original_weights";

    cout << "starting benchmark on sparse graphs...\n" << endl;
    cout << endl;

    for (const auto &entry : directory_iterator(input_dir))
    {
        if (entry.path().extension() == ".txt")
        {
            string filepath = entry.path().string();
            cout << "Processing file: " << filepath << endl;
            cout << endl;
            cout << "Processing with random weights: " << endl;
            // Case 1: random weights (true)
            process_sparse_graph_file(filepath, WeightMode::UniformRandomDistribution, output_dir_random);

            cout << "Processing with exponential weights: " << endl;
            // Case 2: exponential weights
            process_sparse_graph_file(filepath, WeightMode::Exponential, output_dir_exponential);

            cout << "Processing with original weights: " << endl;
            // Case 3: original weights
            process_sparse_graph_file(filepath, WeightMode::Original, output_dir_original);
        }
    }

    cout << "Benchmarking completed for all sparse graphs with original, exponential and uniformly random weights.\n";
    cout << endl;

    cout << "==============================================================================================================================" << endl;

    cout << endl;

    return 0;
}

// Calls for Benchmarking on Dense Graphs with different edge-weight distributions
int BenchmarkTests::run_benchmark_on_dense_graphs()
{
    string input_dir = "./Input_Data/DenseNetworks";
    string output_dir_random = "output/DenseNetworks/random_weights";
    string output_dir_original = "output/DenseNetworks/original_weights";
    string output_dir_exponential = "output/DenseNetworks/exponential_weights";
    string output_dir_uniform = "output/DenseNetworks/uniform_random_weights";

    cout << "starting benchmark on dense graphs...\n" << endl;
    cout << endl;

    // Iterate over all TSP instances
    for (const auto &entry : directory_iterator(input_dir))
    {
        if (entry.path().extension() == ".tsp")
        {
            string filepath = entry.path().string();
            cout << "Processing file: " << filepath << endl;
            cout << endl;

            cout << "Processing with original weights: " << endl;
            // Case 1: random weights (false), default is original weights
            process_dense_graph_file(filepath, WeightMode::Original, output_dir_original);

            cout << "Processing with random weights: " << endl;
            // Case 2: random weights (true),
            process_dense_graph_file(filepath, WeightMode::UniformRandomDistribution, output_dir_random);

            cout << "Processing with exponential weights: " << endl;
            // Case : Exponential weights (true),
            process_dense_graph_file(filepath, WeightMode::Exponential, output_dir_exponential);
        }
    }

    cout << "Benchmarking completed for all dense graphs with original, uniform, exponential and uniformly random weights.\n";
    cout << endl;

    cout << "==============================================================================================================================" << endl;
    cout << endl;

    return 0;
}

// Runs benchmarking experiments on a sequence of complete 
// graphs whose sizes grow between min_n and max_n
int BenchmarkTests::run_benchmark_on_exponential_size_sweep(int min_n,
                                                            int max_n,
                                                            int num_sizes,
                                                            double lambda,
                                                            uint64_t base_seed,
                                                            string base_output_dir,
                                                            bool symmetric_bidirectional)
{

    // Validate input
    if (num_sizes <= 1)
    {
        cerr << "num_sizes must be >= 2\n";
        return -1;
    }
    if (min_n <= 0 || max_n < min_n)
    {
        cerr << "invalid n range\n";
        return -1;
    }

    // Linear step size between graph sizes
    int step = (max_n - min_n) / (num_sizes - 1);
    if (step <= 0)
        step = 1;

    // Create output directories
    string src_dst_output_dir = base_output_dir + "/src_dst_benchmark";
    create_directories(src_dst_output_dir);

    string spt_output_dir = base_output_dir + "/spt_benchmark";
    create_directories(spt_output_dir);

    FileHandler fh;

    // Main benchmark loop for graph instances of each size
    for (int k = 0; k < num_sizes; ++k)
    {
        // Compute graph size for this iteration
        int n = min_n + k * step;
        if (n > max_n)
            n = max_n;

        // Each graph receives a unique seed to keep results reproducible
        uint64_t seed = base_seed + static_cast<uint64_t>(n) * 1000003ULL;

        cout << "\n=== Generating exponential graph: n=" << n
             << " (" << (k + 1) << "/" << num_sizes << ") ===\n" << endl;

         // Build a complete graph with exponential-distributed weights
        Graph graph = fh.generate_complete_exponential_graph(n, lambda, seed, symmetric_bidirectional);
        
        // Sorting adjacency lists 
        graph.sort_all_neighbors();

        // Save CSV for src dst queries
        ostringstream fname;
        fname << src_dst_output_dir << "/n" << n << ".csv";

        // Save CSV for spt queries
        ostringstream fname2;
        fname2 << spt_output_dir << "/n" << n << ".csv";

        // Run src dst benchmark
        cout << "Running src-dst benchmark -> " << fname.str() << endl;
        run_src_dst_benchmark_on_graph(graph, fname.str());

        cout << endl;

        // Run SPT benchmark
        cout << "Running spt benchmark -> " << fname2.str() << endl;
        run_spt_benchmark_on_graph(graph, fname2.str());
    }

    cout << "\nCompleted exponential size sweep: "
         << num_sizes << " graphs (" << min_n << "–" << max_n << " nodes)\n";
    cout << endl;
    cout << "==============================================================================================================================" << endl;
    cout << endl;

    return 0;
}

// Reads node count and edge count. Useful for sizing a complete graph.
GraphCounts BenchmarkTests::read_osm_header_counts(const string& osm_path)
{
    ifstream in(osm_path);
    if (!in)
    {
        throw runtime_error("Cannot open OSM file: " + osm_path);
    }

    GraphCounts counts;
    if (!(in >> counts.n))
    {
        throw runtime_error("Missing node count in OSM file: " + osm_path);
    }
    if (!(in >> counts.m))
    {
        throw runtime_error("Missing edge count in OSM file: " + osm_path);
    }
    return counts;
}

// Given a target number of undirected edges, return the node count needed
// for a complete graph to have at least that many edges.
long long BenchmarkTests::complete_graph_nodes_for_edges(long long target_edges)
{
    // Solve n(n-1)/2 >= target_edges for n.
    double n = (1.0 + sqrt(1.0 + 8.0 * static_cast<double>(target_edges))) / 2.0;
    cout << n << endl;
    return static_cast<long long>(ceil(n));

}

void BenchmarkTests::run_benchmark_dense_graph_osm_edges() {
    namespace fs = filesystem;

    const string input_dir = "./Input_Data/SparseRoadNetworks";

    // Exponential outputs
    const string output_dir_exponential_base = "output/exp_complete_from_sparse/exponential_weights";
    const string exp_src_dir = output_dir_exponential_base + "/src_dst_benchmark";
    const string exp_spt_dir = output_dir_exponential_base + "/spt_benchmark";
    create_directories(exp_src_dir);
    create_directories(exp_spt_dir);

    // Uniform-random outputs
    const string output_dir_random_base = "output/exp_complete_from_sparse/random_weights";
    const string rand_src_dir = output_dir_random_base + "/src_dst_benchmark";
    const string rand_spt_dir = output_dir_random_base + "/spt_benchmark";
    create_directories(rand_src_dir);
    create_directories(rand_spt_dir);

    FileHandler fh;
    const double lambda = 1.0;
    const uint64_t base_seed = 4242;

    cout << "Starting complete-graph benchmarks sized from OSM edge counts...\n\n";

    for (const auto &entry : fs::directory_iterator(input_dir)) {
        if (entry.path().extension() != ".txt") continue;

        const string filepath = entry.path().string();
        const string stem = entry.path().stem().string();

        GraphCounts counts = read_osm_header_counts(filepath);
        // If your sparse graphs store each edge twice (both directions), counts.m already reflects that;
        // we use counts.m as the target undirected edge count here.
        long long target_edges = counts.m;
        int n = static_cast<int>(complete_graph_nodes_for_edges(target_edges));
        uint64_t seed = base_seed + static_cast<uint64_t>(n) * 1000003ULL;

        cout << "OSM file: " << filepath << " -> edges " << counts.m << ", n for complete: " << n << "\n";

        // Exponential weights
        {
            Graph graph = fh.generate_complete_exponential_graph(n, lambda, true, seed);
            graph.sort_all_neighbors();

            ostringstream fname1;
            fname1 << exp_src_dir << "/m" << counts.m << "_" << stem << ".csv";
            ostringstream fname2;
            fname2 << exp_spt_dir << "/m" << counts.m << "_" << stem << ".csv";

            cout << "Running src-dst benchmark (exp) -> " << fname1.str() << endl;
            run_src_dst_benchmark_on_graph(graph, fname1.str());
            cout << endl;
            cout << "Running spt benchmark (exp) -> " << fname2.str() << endl;
            run_spt_benchmark_on_graph(graph, fname2.str());
            cout << endl;
        }

        // Uniform random weights
        {
            Graph graph = fh.generate_complete_uniform_random_graph(n, lambda, true, seed + 1);
            graph.sort_all_neighbors();

            ostringstream fname1;
            fname1 << rand_src_dir << "/m" << counts.m << "_" << stem << ".csv";
            ostringstream fname2;
            fname2 << rand_spt_dir << "/m" << counts.m << "_" << stem << ".csv";

            cout << "Running src-dst benchmark (uniform) -> " << fname1.str() << endl;
            run_src_dst_benchmark_on_graph(graph, fname1.str());
            cout << endl;
            cout << "Running spt benchmark (uniform) -> " << fname2.str() << endl;
            run_spt_benchmark_on_graph(graph, fname2.str());
            cout << endl;
        }
    }

    cout << "Completed complete-graph benchmarks sized from OSM edges.\n";
    cout << "==============================================================================================================================" << endl;
}
