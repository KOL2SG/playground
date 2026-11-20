#include <vector>
#include <string>
#include <sstream>
#include <stdint.h>
#include <stddef.h>

void ProcessStringVector(const std::vector<std::string>& vec);

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
  std::string full_input(reinterpret_cast<const char*>(Data), Size);
  std::vector<std::string> vec;

  // Split the raw string by newline delimiter
  std::stringstream ss(full_input);
  std::string item;
  while (std::getline(ss, item, '\n')) {
    vec.push_back(item);
  }

  ProcessStringVector(vec);
  return 0;
}
