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

#ifndef CONTRIB_WOFF2_WOFF2_WRAPPER_H_
#define CONTRIB_WOFF2_WOFF2_WRAPPER_H_

#include <cinttypes>
#include <cstddef>
#include <cstdlib>

extern "C" {

bool ConvertWOFF2ToTTF(uint8_t *result, size_t result_length,
                       const uint8_t* data, size_t length);
bool ConvertTTFToWOFF2(const uint8_t *data, size_t length,
                       uint8_t *result, size_t *result_length);
size_t MaxWOFF2CompressedSize(const uint8_t* data, size_t length);
size_t ComputeWOFF2FinalSize(const uint8_t* data, size_t length);
} // extern "C"

#endif  // CONTRIB_WOFF2_WOFF2_WRAPPER_H_
