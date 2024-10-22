#include "compressed_graph.h"

#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <random>
#include <vector>

#include "common.h"
#include "context_model.h"
#include "decode.h"
#include "integer_coder.h"

ABSL_FLAG(std::string, input_path, "", "Input file path.");
ABSL_FLAG(uint32_t, repeats, 10, "The number of repeats");
ABSL_FLAG(uint32_t, random, 1000, "Number of random access to randomly selected node");

void TimedRandomAccess(const zuckerli::UncompressedGraph& graph, uint32_t random, uint32_t repeats) {
  std::mt19937 mt();
  mt.seed(0);
  std::uniform_int_distribution<> distrib(1, random);
  std::vector<bool> samples(random, 0);
  for(uint32_t i = 0; i < random; i++) {
    samples[i] = distrib(mt);
  }
  int num_visited = 0;
  for(uint32_t i = 0; i < random; i++) {
    std::cout << samples[i] << std::endl;
  }

  return;

  std::cout << "Random access to adjacency lists..." << std::endl;
  for (uint32_t repeat = 0; repeat < repeats; repeat++) {
    auto t_start = std::chrono::high_resolution_clock::now();
    for (uint32_t sample: samples) {
      auto neighbour = graph.Neighbours(sample);
    }
    auto t_stop = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration<double, std::milli>(t_stop - t_start).count();
    auto time_per_node = elapsed / random;
    std::cout
        << "Wall time elapsed: "
        << elapsed
        << " ms" << std::endl
        << "Average random access: "
        << time_per_node
        << " ms/node" << std::endl;
  }
}

int main(int argc, char* argv[]) {
  absl::ParseCommandLine(argc, argv);
  zuckerli::CompressedGraph graph(absl::GetFlag(FLAGS_input_path));
  std::cout << "This graph has " << graph.size() << " nodes." << std::endl;
  TimedRandomAccess(graph, absl::GetFlag(FLAGS_random), absl::GetFlag(FLAGS_repeats));
  return 0;
}