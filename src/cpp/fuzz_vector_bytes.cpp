#include <vector>
#include <stdint.h>
#include <stddef.h>

// Your target function
void ProcessByteVector(const std::vector<uint8_t>& vec);

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
  // Construct the vector directly from the fuzzer's memory
  std::vector<uint8_t> vec(Data, Data + Size);

  // Pass it to your code
  ProcessByteVector(vec);

  return 0;
}
