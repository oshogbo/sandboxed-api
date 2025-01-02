// Copyright 2022 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <woff2/decode.h>
#include <woff2/encode.h>
#include <woff2/output.h>

#include <cinttypes>
#include <cstddef>
#include <memory>

extern "C" bool ConvertWOFF2ToTTF(uint8_t *result, size_t result_length,
                                  const uint8_t* data, size_t length) {
  // Although the sandbox API uses a header of a function marked as deprecated,
  // internally we use the recommended API. The reason for keep using deprecated
  // function API is to avoid the need for an output class (WOFF2Out),
  // which would require a wrapper structure that we want to avoid.
  woff2::WOFF2MemoryOut out(result, result_length);
  return ::woff2::ConvertWOFF2ToTTF(data, length, &out);
}

extern "C" bool ConvertTTFToWOFF2(const uint8_t *data, size_t length,
                                  uint8_t *result, size_t *result_length) {
  return ::woff2::ConvertTTFToWOFF2(data, length, result, result_length);
}

extern "C" size_t MaxWOFF2CompressedSize(const uint8_t* data, size_t length) {
  return ::woff2::MaxWOFF2CompressedSize(data, length);
}

extern "C" size_t ComputeWOFF2FinalSize(const uint8_t* data, size_t length) {
  return ::woff2::ComputeWOFF2FinalSize(data, length);
}
