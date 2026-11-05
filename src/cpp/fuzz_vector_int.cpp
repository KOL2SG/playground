#include <vector>
#include <stdint.h>
#include <stddef.h>

void ProcessIntVector(const std::vector<int>& vec);

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
  // 1. Calculate how many integers we can extract from the fuzzed bytes
  size_t num_elements = Size / sizeof(int);
  if (num_elements == 0) {
    return 0;
  }

  // 2. Safely cast the raw pointer and populate the vector
  const int* int_data = reinterpret_cast<const int*>(Data);
  std::vector<int> vec(int_data, int_data + num_elements);

  // 3. Execute target function
  ProcessIntVector(vec);

  return 0;
}
