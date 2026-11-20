#include <vector>
#include <fuzzer/FuzzedDataProvider.h>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
  FuzzedDataProvider provider(Data, Size);

  // Dynamically consume vectors of values safely
  std::vector<uint32_t> vec = provider.ConsumeBytes<uint32_t>(provider.ConsumeIntegralInRange<size_t>(0, 100));

  return 0;
}
