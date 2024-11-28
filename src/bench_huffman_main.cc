
#include "huffman.h"

#include <random>
#include <algorithm>

#include "bit_reader.h"
#include "integer_coder.h"
#include "absl/random/zipf_distribution.h"

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"

ABSL_FLAG(uint32_t, random, 1000, "Number of reads to bench for repeat");
ABSL_FLAG(uint32_t, repeats, 10, "The number of repeats");
ABSL_FLAG(uint32_t, seed, 0, "The seed of the experiment");

// Bench the average read time over a set of encoded integer sampled from a zipf distribution
void TimedHuffmanRead(uint32_t random, uint32_t repeats, uint32_t seed) {
  constexpr size_t kNumContexts = 1;
  constexpr size_t kDefaultContext = 0;
  
  zuckerli::IntegerData data;

  std::mt19937 mt(seed);
  absl::zipf_distribution<uint32_t> zipf_dist(1000000000, 1.5);

  for (size_t i = 0; i < random; i++) {
    size_t integer = zipf_dist(mt);
    data.Add(kDefaultContext, integer);
  }

  zuckerli::BitWriter writer;
  std::vector<double> unused_bits_per_ctx;
  HuffmanEncode(data, kNumContexts, &writer, {}, &unused_bits_per_ctx);

  std::vector<double> times_per_repeats;
  std::vector<uint8_t> encoded = std::move(writer).GetData();
  std::cout 
    << "Start decoding " << random << " integers"
    << std::endl;
  for (uint32_t repeat = 0; repeat < repeats; repeat++) {
    zuckerli::BitReader reader(encoded.data(), encoded.size());
    zuckerli::HuffmanReader symbol_reader;
    symbol_reader.Init(kNumContexts, &reader);

    auto t_start = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < random; i++) {
      zuckerli::IntegerCoder::Read(kDefaultContext, &reader, &symbol_reader);
    }
    auto t_stop = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration<double, std::nano>(t_stop - t_start).count();
    auto time_per_read = elapsed / random;
    times_per_repeats.push_back(time_per_read);
    std::cout
      << "Wall time elapsed: "
      << elapsed
      << " ns" << std::endl
      << "Average read time: "
      << time_per_read
      << " ns/read" << std::endl;
  }

  std::sort(times_per_repeats.begin(), times_per_repeats.end());
  double median_time;
  if (times_per_repeats.size() % 2 == 0) {
    median_time = (times_per_repeats[times_per_repeats.size() / 2 - 1] + times_per_repeats[times_per_repeats.size() / 2]) / 2;
  } else {
    median_time = times_per_repeats[times_per_repeats.size() / 2];
  }
  std::cout << "Median time: " << median_time << " ns/read" << std::endl;
}

int main(int argc, char* argv[]) {
  absl::ParseCommandLine(argc, argv);
  TimedHuffmanRead(absl::GetFlag(FLAGS_random), absl::GetFlag(FLAGS_repeats), absl::GetFlag(FLAGS_seed));
  return 0;
}