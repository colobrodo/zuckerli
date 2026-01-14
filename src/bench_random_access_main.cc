#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <random>
#include <vector>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "compressed_graph.h"
#include "uncompressed_graph.h"

ABSL_FLAG(std::string, input_path, "", "Input file path.");
ABSL_FLAG(uint32_t, random, 1000,
          "Number of random access to randomly selected node");
ABSL_FLAG(uint32_t, repeats, 10, "The number of repeats");
ABSL_FLAG(uint32_t, seed, 0, "The seed of the experiment");
ABSL_FLAG(bool, tsv, false,
          "Porcellain-like option: If true the output is provided as tsv, with "
          "three field:"
          " the first for the iteration number, followed by the total time of "
          "this run and lastly the run's average time (in ns/arc)");

ABSL_FLAG(bool, first, false,
          "If true measures only the time to decode and access the first "
          "element of the list");

void TimedRandomAccess(zuckerli::CompressedGraph graph, uint32_t random,
                       uint32_t repeats, bool first, uint32_t seed,
                       bool print_as_tsv) {
  std::mt19937 mt(seed);
  std::uniform_int_distribution<uint32_t> distrib(0, graph.size() - 1);
  std::vector<uint32_t> samples(random, 0);
  for (uint32_t i = 0; i < random; i++) {
    auto a = distrib(mt);
    samples[i] = a;
  }

  if (!print_as_tsv)
    std::cout << "Random access to adjacency lists..." << std::endl;
  for (uint32_t repeat = 0; repeat < repeats; repeat++) {
    auto c = 0;
    auto t_start = std::chrono::high_resolution_clock::now();
    if (first) {
      for (uint32_t sample : samples) {
        auto first_element = graph.Neighbours(sample)[0];
        (void)first_element;
      }
      c = random;
    } else {
      for (uint32_t sample : samples) {
        auto neighbours = graph.Neighbours(sample);
        c += neighbours.size();
      }
    }
    auto t_stop = std::chrono::high_resolution_clock::now();
    auto elapsed =
        std::chrono::duration<double, std::nano>(t_stop - t_start).count();
    auto time_per_arc = elapsed / c;
    if (print_as_tsv) {
      std::cout << repeat << "\t" << elapsed << "\t" << time_per_arc
                << std::endl;
    } else {
      std::cout << "Wall time elapsed: " << elapsed << " ns" << std::endl
                << "Average random access: " << time_per_arc << " ns/arc"
                << std::endl;
    }
  }
}

int main(int argc, char *argv[]) {
  absl::ParseCommandLine(argc, argv);
  zuckerli::CompressedGraph graph(absl::GetFlag(FLAGS_input_path));
  auto print_as_tsv = absl::GetFlag(FLAGS_tsv);
  if (!print_as_tsv) {
    std::cout << "This graph has " << graph.size() << " nodes." << std::endl;
  }
  TimedRandomAccess(graph, absl::GetFlag(FLAGS_random),
                    absl::GetFlag(FLAGS_repeats), absl::GetFlag(FLAGS_first),
                    absl::GetFlag(FLAGS_seed), print_as_tsv);
  return 0;
}