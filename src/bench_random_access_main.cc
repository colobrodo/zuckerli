
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <random>
#include <vector>

#include "compressed_graph.h"
#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "uncompressed_graph.h"

ABSL_FLAG(std::string, input_path, "", "Input file path.");
ABSL_FLAG(uint32_t, random, 1000, "Number of random access to randomly selected node");
ABSL_FLAG(uint32_t, repeats, 10, "The number of repeats");
ABSL_FLAG(uint32_t, seed, 0, "The seed of the experiment");

void TimedRandomAccess(zuckerli::CompressedGraph graph, uint32_t random, uint32_t repeats, uint32_t seed) {
  std::mt19937 mt(seed);
  std::uniform_int_distribution<int> distrib(0, graph.size() - 1);
  std::vector<bool> samples(random, 0);
  for(uint32_t i = 0; i < random; i++) {
    auto a = distrib(mt);
    std::cout << "Gen: " << a << std::endl;
    samples[i] = a;
  }

  std::cout << "Random access to adjacency lists..." << std::endl;
  for (uint32_t repeat = 0; repeat < repeats; repeat++) {
    auto arcs = 0;
    auto t_start = std::chrono::high_resolution_clock::now();
    for (uint32_t sample: samples) {
      auto neighbours = graph.Neighbours(sample);
      arcs += neighbours.size();
    }
    auto t_stop = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration<double, std::nano>(t_stop - t_start).count();
    std::cout << "arcs: " << arcs << std::endl;
    auto time_per_arc = elapsed / arcs;
    std::cout
        << "Wall time elapsed: "
        << elapsed
        << " ns" << std::endl
        << "Average random access: "
        << time_per_arc
        << " ns/arc" << std::endl;
  }
}

int main(int argc, char* argv[]) {
  absl::ParseCommandLine(argc, argv);
  zuckerli::CompressedGraph graph(absl::GetFlag(FLAGS_input_path));
  std::cout << "This graph has " << graph.size() << " nodes." << std::endl;
  TimedRandomAccess(graph, absl::GetFlag(FLAGS_random), absl::GetFlag(FLAGS_repeats), absl::GetFlag(FLAGS_seed));
  return 0;
}