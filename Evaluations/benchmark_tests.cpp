#include "benchmark_tests.hpp"
#include <chrono>

using namespace std;
using namespace filesystem;

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


void BenchmarkTests::append_average_summary(const AggregateStats& A, const string& output_csv_path) {
    namespace fs = std::filesystem;

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
    std::string filename = out_path.stem().string();  // "dj38"

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

PertinenceStats BenchmarkTests::analyze_spt_pertinence(
    const SsspResult &res,
    const Graph &graph)
{
    PertinenceStats stats;

    for (EdgeId eid = 0; eid < (EdgeId)graph.number_of_edges(); ++eid)
    {
        Edge edge = graph.get_edge(eid);

        if (res.in_pertinent_edges[eid] and res.out_pertinent_edges[eid])
        {
            
            cerr << "Warning: edge " << eid
                 << " is marked both in-pertinent and out-pertinent.\n";
            stats.conflict_both += 1;
        }
        else if (res.in_pertinent_edges[eid])
        {
            if (edge.cost < 2*(res.distance[edge.trg] - res.median)) {
                stats.total_in_pertinent_edges += 1;
                if (res.in_pertinent_edges_extracted_in_forward_phase[eid])
                {
                    stats.total_in_pertinent_extracted_in_forward += 1;
                }

            }
            else {
                cout << "Edge incorrectly marked as in-pertinent" << endl;
            }            
        }
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

        // ---------- WARNING 1: SPT edge is neither in nor out ----------
        if (!is_out && !is_in)
        {
            cout << "WARNING: SPT edge " << eid
                 << " (" << graph.get_edge(eid).src << " -> "
                 << graph.get_edge(eid).trg
                 << ") is NOT pertinent.\n";

            stats.non_pertinent_edge_in_spt++;
        }
    }

    stats.spt_edges_incorrectly_classified = (stats.non_pertinent_edge_in_spt > 0);

    // ----- Ratios -----
    if (graph.number_of_edges() > 0)
    {
        stats.ratio_pert_m = (double)stats.total_pertinent_edges / (double)graph.number_of_edges();
    }
    if (graph.number_of_nodes() > 0)
    {
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

PertinenceStats BenchmarkTests::analyze_path_pertinence(const SsspResult &res, const Graph &graph)
{
    PertinenceStats stats;

    for (EdgeId eid = 0; eid < (EdgeId)graph.number_of_edges(); ++eid)
    {

        if (res.in_pertinent_edges[eid] and res.out_pertinent_edges[eid])
        {
            cerr << "Warning: edge " << eid
                 << " is marked both in-pertinent and out-pertinent.\n";
            stats.conflict_both += 1;
        }
        else if (res.in_pertinent_edges[eid])
        {
            stats.total_in_pertinent_edges += 1;
            if (res.in_pertinent_edges_extracted_in_forward_phase[eid])
            {
                stats.total_in_pertinent_extracted_in_forward += 1;
            }
        }
        else if (res.out_pertinent_edges[eid])
        {
            stats.total_out_pertinent_edges += 1;
        }
    }

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

        // ---------- WARNING 1: Path edge is neither in nor out ----------
        if (!is_out && !is_in)
        {
            const Edge &e = graph.get_edge(eid);
            cout << "WARNING: Path edge " << eid
                 << " (" << e.src << " -> " << e.trg
                 << ") is NOT pertinent.\n";
            stats.non_pertinent_edge_in_spt++;
        }
    }

    stats.spt_edges_incorrectly_classified = (stats.non_pertinent_edge_in_spt > 0);

    // ----- Ratios -----
    if (graph.number_of_edges() > 0)
    {
        stats.ratio_pert_m = (double)stats.total_pertinent_edges / (double)graph.number_of_edges();
    }
    if (graph.number_of_nodes() > 0)
    {
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

vector<pair<int, int>> BenchmarkTests::generate_query_pairs(int n)
{

    // ----- Generate 100 unique (src, dst) pairs -----
    mt19937 rng(62); // fixed seed for reproducibility
    uniform_int_distribution<int> dist(0, n - 1);

    vector<pair<int, int>> query_pairs;
    query_pairs.reserve(100);
    set<long long> used; // encode (src,dst) as 64-bit to ensure uniqueness

    auto encode = [](int s, int t) -> long long
    {
        return (static_cast<long long>(static_cast<unsigned int>(s)) << 32) | static_cast<unsigned int>(t);
    };

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

// ---- Benchmark helper ----
void BenchmarkTests::run_src_dst_benchmark_on_graph(Graph &graph, const string &output_csv_path)
{
    const int n = graph.number_of_nodes();
    vector<pair<int, int>> query_pairs = generate_query_pairs(n);

    Dijkstra dijkstra(graph);
    Spira spira(graph);
    NewVariant new_variant(graph);

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

    cout << "Running 100 src-dst queries..." << endl;
    int mismatch = 0;
    for (auto [src, dst] : query_pairs)
    {
        auto d_start = chrono::high_resolution_clock::now();
        SsspResult rd = dijkstra.compute_shortest_path(src, dst);
        auto d_end = chrono::high_resolution_clock::now();
        long long d_time_us = chrono::duration_cast<chrono::microseconds>(d_end - d_start).count();
        Cost dc = rd.total_cost;

        auto s_start = chrono::high_resolution_clock::now();
        SsspResult rs = spira.compute_shortest_path(src, dst);
        auto s_end = chrono::high_resolution_clock::now();
        long long s_time_us = chrono::duration_cast<chrono::microseconds>(s_end - s_start).count();
        Cost sc = rs.total_cost;

        auto n_start = chrono::high_resolution_clock::now();
        SsspResult rn = new_variant.compute_shortest_path(src, dst);
        auto n_end = chrono::high_resolution_clock::now();
        long long n_time_us = chrono::duration_cast<chrono::microseconds>(n_end - n_start).count();
        Cost nc = rn.total_cost;

        auto same_cost = [](Cost a, Cost b)
        {
            if (a < 0 || b < 0)
                return a == b;
            return fabs(a - b) <= 1e-9;
        };

        bool matched = same_cost(dc, sc) && same_cost(dc, nc);

        if (!matched)
        {
            cout << "Warning: cost mismatch for src=" << src << ", dst=" << dst << "\n";
            for (auto i = 0; i < rn.path.size(); i++)
            {
                cout << rn.path[i] << " ";
            }
            mismatch++;
        }

        PertinenceStats stats = analyze_path_pertinence(rn, graph);
 
        stats.ratio_pop_pert = (stats.total_pertinent_edges > 0) ? (double)(rn.number_of_pops) / (double)(stats.total_pertinent_edges) : 0.0;
        if (stats.conflict_both != 0)
        {
            cout << "Conflicts in pertinence classification.\n";
        }

        if (stats.spt_edges_incorrectly_classified)
        {
            cout << "SPT edges classified not classified correctly.\n";
        }

        // aggregate stats
        agg.avg_d_time += d_time_us;
        agg.avg_s_time += s_time_us;
        agg.avg_nv_time += n_time_us;

        agg.avg_d_pops += rd.number_of_pops;
        agg.avg_s_pops += rs.number_of_pops;
        agg.avg_nv_pops += rn.number_of_pops;
        agg.avg_nvq_pops += rn.number_of_Q_pops;

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

        // cout << "No pertinent edges found.\n";
        out << src << ',' << dst << ','
            << d_time_us << ',' << dc << ',' << s_time_us << ',' << sc << ',' << n_time_us << ',' << nc << ','
            << rd.number_of_pops << ',' << rs.number_of_pops << ',' << rn.number_of_pops << ',' << rn.number_of_Q_pops << ','
            << rd.avg_pops_per_node << ',' << rs.avg_pops_per_node << ',' << rn.avg_pops_per_node << ','
            << stats.total_in_pertinent_edges << ',' << stats.total_in_pertinent_extracted_in_forward << ',' << stats.total_out_pertinent_edges << ',' << stats.total_pertinent_edges << ',' << stats.conflict_both << ','
            << stats.out_spt << ',' << stats.in_spt << ',' << stats.non_pertinent_edge_in_spt << ',' << stats.total_spt_edges << ',' << stats.spt_edges_incorrectly_classified << ','
            << stats.ratio_pert_m << ',' << stats.ratio_pert_n << ',' << stats.ratio_in_transferred << ',' << stats.ratio_spt_out << ',' << stats.ratio_spt_in << ',' << stats.ratio_pop_pert << ','
            << matched << '\n';
    }
    cout << "Number of Mismatched Queries: " << mismatch << endl;
    out.close();

    finalize_aggregate_stats(agg);
    append_average_summary(agg, output_csv_path);
}

// ---- Helper: Count node-wise mismatches between two SPTs ----
int BenchmarkTests::count_distance_mismatches(const std::vector<Cost> &a,
                                              const std::vector<Cost> &b)
{
    int mismatches = 0;
    for (size_t i = 0; i < a.size(); ++i)
    {
        if (std::fabs(a[i] - b[i]) > 1e-9) // tolerance for floating-point comparison
            mismatches++;
    }
    return mismatches;
}

// ---- Helper: Compare SPT equality ----
bool BenchmarkTests::compare_spt_results(const SsspResult &a,
                                         const SsspResult &b,
                                         const std::string &nameA,
                                         const std::string &nameB)
{
    if (a.distance.size() != b.distance.size())
        return false;

    for (size_t i = 0; i < a.distance.size(); ++i)
    {
        Cost da = a.distance[i];
        Cost db = b.distance[i];
        if ((da < INF_COST && db < INF_COST && std::fabs(da - db) > 1e-9) ||
            (da == INF_COST && db != INF_COST) ||
            (db == INF_COST && da != INF_COST))
        {
            cout << "Distance mismatch at node " << i << ": "
                 << nameA << " has " << da << ", "
                 << nameB << " has " << db << "\n";
            return false;
        }
    }
    return true;
}

// ---- Main Benchmark ----
void BenchmarkTests::run_spt_benchmark_on_graph(Graph &graph, const std::string &output_csv_path)
{
    const int n = graph.number_of_nodes();
    const int num_sources = std::min(n, 100);

    // --- Select 100 unique random sources ---
    std::vector<int> sources;
    sources.reserve(num_sources);

    std::mt19937 rng(42); // fixed seed for reproducibility
    std::uniform_int_distribution<int> dist(0, n - 1);

    std::unordered_set<int> used;
    while (sources.size() < num_sources)
    {
        int src = dist(rng);
        if (used.insert(src).second)
            sources.push_back(src);
    }

    Dijkstra dijkstra(graph);
    Spira spira(graph);
    NewVariant new_variant(graph);

    std::ofstream out(output_csv_path);
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


    std::cout << "Running " << num_sources << " SPT queries..." << std::endl;
    int mismatch_runs = 0;

    for (int src : sources)
    {
        // === Dijkstra ===
        auto d_start = std::chrono::high_resolution_clock::now();
        SsspResult rd = dijkstra.compute_shortest_path(src, -1);
        auto d_end = std::chrono::high_resolution_clock::now();
        long long d_time_us =
            std::chrono::duration_cast<std::chrono::microseconds>(d_end - d_start).count();

        // === Spira ===
        auto s_start = std::chrono::high_resolution_clock::now();
        SsspResult rs = spira.compute_shortest_path(src, -1);
        auto s_end = std::chrono::high_resolution_clock::now();
        long long s_time_us =
            std::chrono::duration_cast<std::chrono::microseconds>(s_end - s_start).count();

        // === New Variant ===
        auto n_start = std::chrono::high_resolution_clock::now();
        SsspResult rn = new_variant.compute_shortest_path(src, -1);
        auto n_end = std::chrono::high_resolution_clock::now();
        long long n_time_us =
            std::chrono::duration_cast<std::chrono::microseconds>(n_end - n_start).count();

        // int in_pertinent_edges = 0;
        // int out_pertinent_edges = 0;
        // int in_pertinent_edges_transferred = 0;

        // cout << "in_pertinent_edges: " << rn.in_pertinent_edges.size() << endl;
        //     for (EdgeId eid = 0; eid < (EdgeId)graph.number_of_edges(); ++eid) {
        //     if (rn.in_pertinent_edges[eid]) {
        //         in_pertinent_edges += 1;
        //         if(rn.in_pertinent_edges_extracted_in_forward_phase[eid]){
        //             in_pertinent_edges_transferred += 1;
        //         }
        //     }
        //     else if (rn.out_pertinent_edges[eid]) {
        //         out_pertinent_edges += 1;
        //     }
        // }
        // int total_pertinent_edges = in_pertinent_edges + out_pertinent_edges;

        // === Compare distances ===
        bool match_ds = compare_spt_results(rd, rs, "Dijkstra", "Spira");
        bool match_dn = compare_spt_results(rd, rn, "Dijkstra", "NewVariant");

        int mismatch_count_ds = count_distance_mismatches(rd.distance, rs.distance);
        int mismatch_count_dn = count_distance_mismatches(rd.distance, rn.distance);

        bool mismatches = !(match_ds && match_dn);
        if (mismatches)
            mismatch_runs++;

        // cout  << "finished run for src: " << src << endl;
        // cout << "analysis" << endl;
        PertinenceStats stats = analyze_spt_pertinence(rn, graph);

        stats.ratio_pop_pert = (stats.total_pertinent_edges > 0) ? (double)(rn.number_of_pops) / (double)(stats.total_pertinent_edges) : 0.0;
        if (stats.conflict_both != 0)
        {
            cout << "conflicts in pertinence classification.\n";
        }
        if (stats.spt_edges_incorrectly_classified)
        {
            cout << "SPT edges not classified correctly.\n";
        }

        // aggregate stats
        agg.avg_d_time += d_time_us;
        agg.avg_s_time += s_time_us;
        agg.avg_nv_time += n_time_us;

        agg.avg_d_pops += rd.number_of_pops;
        agg.avg_s_pops += rs.number_of_pops;
        agg.avg_nv_pops += rn.number_of_pops;
        agg.avg_nvq_pops += rn.number_of_Q_pops;

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
            << d_time_us << ',' << s_time_us << ',' << n_time_us << ','
            << rd.number_of_pops << ',' << rs.number_of_pops << ',' << rn.number_of_pops << ',' << rn.number_of_Q_pops << ','
            << rd.avg_pops_per_node << ',' << rs.avg_pops_per_node << ',' << rn.avg_pops_per_node << ','
            << stats.total_in_pertinent_edges << ',' << stats.total_in_pertinent_extracted_in_forward << ',' << stats.total_out_pertinent_edges << ',' << stats.total_pertinent_edges << ',' << stats.conflict_both << ','
            << stats.out_spt << ',' << stats.in_spt << ',' << stats.non_pertinent_edge_in_spt << ',' << stats.total_spt_edges << ',' << stats.spt_edges_incorrectly_classified << ','
            << stats.ratio_pert_m << ',' << stats.ratio_pert_n << ',' << stats.ratio_in_transferred << ',' << stats.ratio_spt_out << ',' << stats.ratio_spt_in << ',' << stats.ratio_pop_pert << ','
            << mismatch_count_ds << ',' << mismatch_count_dn << ','
            << std::boolalpha << mismatches << '\n';
    }

    out.close();

    // std::cout << "Benchmark complete.\n";
    std::cout << "Mismatches in " << mismatch_runs << " / " << num_sources << " runs.\n";
    // std::cout << "Results saved to " << output_csv_path << std::endl;
    std::cout << std::endl;

    finalize_aggregate_stats(agg);
    append_average_summary(agg, output_csv_path);
}

// ---- Wrapper to read and run ----
void BenchmarkTests::process_sparse_graph_file(const string &filepath, WeightMode weight_mode, string &output_dir)
{
    FileHandler fh;
    Graph graph = fh.read_sparse_graph_file(filepath, weight_mode);
    graph.sort_all_neighbors();

    string output_dir1 = output_dir + "/src_dst_benchmark";
    create_directories(output_dir1); // make sure directory exists

    string filename = path(filepath).stem().string(); // "osm5" from "osm5.txt"
    string output_csv = output_dir1 + "/" + filename + ".csv";
    // cout << "Running on " << filepath << " -> " << output_csv << endl;
    cout << "Running src-dst benchmark -> " << output_csv << endl;
    run_src_dst_benchmark_on_graph(graph, output_csv);

    string output_dir2 = output_dir + "/spt_benchmark";
    create_directories(output_dir2); // make sure directory exists
    cout << endl;
    filename = path(filepath).stem().string(); // "osm5" from "osm5.txt"
    output_csv = output_dir2 + "/" + filename + ".csv";
    cout << "Running spt benchmark -> " << output_csv << endl;
    run_spt_benchmark_on_graph(graph, output_csv);

    // cout << endl;
}

// ---- Wrapper to read and run ----
void BenchmarkTests::process_dense_graph_file(const string &filepath, WeightMode weight_mode, string &output_dir)
{
    FileHandler fh;
    Graph graph = fh.read_dense_graph_file(filepath, weight_mode);
    graph.sort_all_neighbors();

    string output_dir1 = output_dir + "/src_dst_benchmark";
    create_directories(output_dir1); // make sure directory exists

    string filename = path(filepath).stem().string(); // "osm5" from "osm5.txt"
    string output_csv = output_dir1 + "/" + filename + ".csv";

    // cout << "Running on " << filepath << " -> " << output_csv << endl;
    cout << "Running src-dst benchmark -> " << output_csv << endl;
    run_src_dst_benchmark_on_graph(graph, output_csv);

    string output_dir2 = output_dir + "/spt_benchmark";
    create_directories(output_dir2); // make sure directory exists

    filename = path(filepath).stem().string(); // "osm5" from "osm5.txt"
    output_csv = output_dir2 + "/" + filename + ".csv";
    cout << endl;
    cout << "Running spt benchmark -> " << output_csv << endl;
    run_spt_benchmark_on_graph(graph, output_csv);
}

int BenchmarkTests::run_benchmark_on_sparse_graphs()
{
    string input_dir = "./Input_Data/SparseRoadNetworks";
    string output_dir_random = "output/sparse_networks/random_weights";
    string output_dir_exponential = "output/sparse_networks/exponential_weights";
    string output_dir_original = "output/sparse_networks/original_weights";

    cout << "starting benchmark on sparse graphs...\n";
    cout << endl;
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

int BenchmarkTests::run_benchmark_on_dense_graphs()
{
    string input_dir = "./Input_Data/DenseNetworks";
    string output_dir_random = "output/DenseNetworks/random_weights";
    string output_dir_original = "output/DenseNetworks/original_weights";
    string output_dir_exponential = "output/DenseNetworks/exponential_weights";
    string output_dir_uniform = "output/DenseNetworks/uniform_random_weights";

    cout << "starting benchmark on dense graphs...\n";
    cout << endl;
    cout << endl;

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
            // // Case 2: random weights (true),
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

int BenchmarkTests::run_benchmark_on_exponential_size_sweep(int min_n,
                                                            int max_n,
                                                            int num_sizes,
                                                            double lambda,
                                                            uint64_t base_seed,
                                                            string base_output_dir,
                                                            bool symmetric_bidirectional)
{
    using namespace std;

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

    int step = (max_n - min_n) / (num_sizes - 1);
    if (step <= 0)
        step = 1;

    string base_output_dir1 = base_output_dir + "/src_dst_benchmark";
    create_directories(base_output_dir1);

    string base_output_dir2 = base_output_dir + "/spt_benchmark";
    create_directories(base_output_dir2);

    FileHandler fh;

    for (int k = 0; k < num_sizes; ++k)
    {
        int n = min_n + k * step;
        if (n > max_n)
            n = max_n;

        uint64_t seed = base_seed + static_cast<uint64_t>(n) * 1000003ULL;

        cout << "\n=== Generating exponential graph: n=" << n
             << " (" << (k + 1) << "/" << num_sizes << ") ===\n";

        cout << endl;
        Graph graph = fh.generate_complete_exponential_graph(n, lambda, seed, symmetric_bidirectional);
        graph.sort_all_neighbors();

        // Save CSV directly in base_output_dir
        ostringstream fname;
        fname << base_output_dir1 << "/n" << n << ".csv";

        // for spt
        ostringstream fname2;
        fname2 << base_output_dir2 << "/n" << n << ".csv";

        // cout << "Running benchmark -> " << fname.str() << endl;
        cout << "Running src-dst benchmark -> " << fname.str() << endl;
        run_src_dst_benchmark_on_graph(graph, fname.str());
        cout << endl;
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



// Reads the header of an OSM-style file where the first line is node count
// and the second line is edge count. Useful for sizing a complete graph.
GraphCounts BenchmarkTests::read_osm_header_counts(const std::string& osm_path)
{
    std::ifstream in(osm_path);
    if (!in)
    {
        throw std::runtime_error("Cannot open OSM file: " + osm_path);
    }

    GraphCounts counts;
    if (!(in >> counts.n))
    {
        throw std::runtime_error("Missing node count in OSM file: " + osm_path);
    }
    if (!(in >> counts.m))
    {
        throw std::runtime_error("Missing edge count in OSM file: " + osm_path);
    }
    return counts;
}

// Given a target number of undirected edges, return the node count needed
// for a complete graph to have at least that many edges.
long long BenchmarkTests::complete_graph_nodes_for_edges(long long target_edges)
{
    // Solve n(n-1)/2 >= target_edges for n.
    double n = (1.0 + std::sqrt(1.0 + 8.0 * static_cast<double>(target_edges))) / 2.0;
    cout << n << endl;
    return static_cast<long long>(std::ceil(n));

}

void BenchmarkTests::run_benchmark_dense_graph_osm_edges() {
    namespace fs = std::filesystem;

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

    // auto generate_uniform_complete = [](int n, uint64_t seed) {
    //     std::mt19937_64 gen(seed);
    //     std::uniform_real_distribution<Cost> dist(0.0, 1.0);

    //     Graph g(n);
    //     for (int i = 0; i < n; ++i) {
    //         Node node{i, 0.0, 0.0};
    //         // node.id = i;
    //         // node.latitude = 0.0;
    //         // node.longitude = 0.0;
    //         g.set_node(node);
    //     // }
    //     // for (int i = 0; i < n; ++i) {
    //         for (int j = i + 1; j < n; ++j) {
    //             Edge e, r;
    //             e.src = i;
    //             e.trg = j;
    //             r.src = j;
    //             r.trg = i;
    //             Cost w = dist(gen);
    //             e.cost = w;
    //             r.cost = w;
    //             g.set_edge(e, r);
    //         }
    //     }
    //     return g;
    // };

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
