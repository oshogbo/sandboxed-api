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

#include "contrib/woff2/woff2_sapi.h"

#include <woff2/decode.h>
#include <woff2/encode.h>
#include <woff2/output.h>

#include <cstddef>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <optional>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "contrib/woff2/woff2_wrapper.h"
#include "sandboxed_api/testing.h"
#include "sandboxed_api/util/fileops.h"
#include "sandboxed_api/util/path.h"
#include "sandboxed_api/util/status_matchers.h"
#include "woff2_sapi.sapi.h"  // NOLINT(build/include)

namespace {

using ::sapi::IsOk;
using ::testing::Eq;
using ::testing::IsNull;
using ::testing::Not;
using ::testing::StrEq;

class Woff2Test : public testing::Test {
 public:
  static void SetUpTestSuite() {
    test_data_dir_ = ::getenv("TEST_DATA_DIR");
    ASSERT_THAT(test_data_dir_, Not(IsNull()));
  }
  static absl::StatusOr<std::vector<uint8_t>> ReadFile(
      const char* in_file, size_t expected_size = SIZE_MAX);
  static const char* test_data_dir_;
};

class Woff2SapiSandboxTest : public Woff2Test {
 protected:
  static void SetUpTestSuite() {
    Woff2Test::SetUpTestSuite();
    sandbox_ = new ::sapi_woff2::Woff2SapiSandbox();
    ASSERT_THAT(sandbox_->Init(), IsOk());
    api_ = new ::sapi_woff2::WOFF2Api(sandbox_);
  }
  static void TearDownTestSuite() {
    delete api_;
    delete sandbox_;
  }
  static ::sapi_woff2::WOFF2Api* api_;

 private:
  static ::sapi_woff2::Woff2SapiSandbox* sandbox_;
};

const char* Woff2Test::test_data_dir_;
::sapi_woff2::Woff2SapiSandbox* Woff2SapiSandboxTest::sandbox_;
::sapi_woff2::WOFF2Api* Woff2SapiSandboxTest::api_;

std::streamsize GetStreamSize(std::ifstream& stream) {
  stream.seekg(0, std::ios_base::end);
  std::streamsize ssize = stream.tellg();
  stream.seekg(0, std::ios_base::beg);

  return ssize;
}

absl::StatusOr<std::vector<uint8_t>> Woff2Test::ReadFile(
    const char* in_file, size_t expected_size) {
  auto env = absl::StrCat(test_data_dir_, "/", in_file);
  std::ifstream f(env);
  if (!f.is_open()) {
    return absl::UnavailableError("File could not be opened");
  }
  std::streamsize ssize = GetStreamSize(f);
  if (expected_size != SIZE_MAX && ssize != expected_size) {
    return absl::UnavailableError("Incorrect size of file");
  }
  std::vector<uint8_t> inbuf((ssize));
  f.read(reinterpret_cast<char*>(inbuf.data()), ssize);
  if (ssize != f.gcount()) {
    return absl::UnavailableError("Premature end of file");
  }
  if (f.fail() || f.eof()) {
    return absl::UnavailableError("Error reading file");
  }
  return inbuf;
}

TEST_F(Woff2SapiSandboxTest, Compress) {
  auto source = ReadFile("Roboto-Regular.ttf");
  ASSERT_THAT(source, IsOk());
  sapi::v::Array array(source->data(), source->size());

  SAPI_ASSERT_OK_AND_ASSIGN(size_t size,
                            api_->MaxWOFF2CompressedSize(array.PtrBefore(),
                                                         source->size())
                            );
  sapi::v::IntBase<size_t> actual_size(size);
  sapi::v::Array<uint8_t> actual(size);
  SAPI_ASSERT_OK(
      api_->ConvertTTFToWOFF2(array.PtrBefore(), source->size(),
                              actual.PtrAfter(), actual_size.PtrBoth())
  );

  auto expected = ReadFile("Roboto-Regular.woff2");
  ASSERT_THAT(expected, IsOk());
  ASSERT_EQ(actual_size.GetValue(), expected->size());
  ASSERT_TRUE(std::equal(actual.GetData(), actual.GetData() + expected->size(),
                         expected->data()));
}

TEST_F(Woff2Test, CompressRaw) {
  auto source = ReadFile("Roboto-Regular.ttf");
  ASSERT_THAT(source, IsOk());

  size_t actual_size = woff2::MaxWOFF2CompressedSize(source->data(),
                                                     source->size());
  uint8_t* actual = new uint8_t[actual_size];
  woff2::ConvertTTFToWOFF2(source->data(), source->size(),
                           actual, &actual_size);

  auto expected = ReadFile("Roboto-Regular.woff2");
  ASSERT_THAT(expected, IsOk());
  ASSERT_EQ(actual_size, expected->size());
  ASSERT_TRUE(std::equal(actual, actual + expected->size(),
                         expected->data()));
}

TEST_F(Woff2SapiSandboxTest, Decompress) {
  auto source = ReadFile("Roboto-Regular.woff2");
  ASSERT_THAT(source, IsOk());
  sapi::v::Array array(source->data(), source->size());

  SAPI_ASSERT_OK_AND_ASSIGN(size_t size,
                            api_->ComputeWOFF2FinalSize(array.PtrBefore(),
                                                        source->size())
                            );
  sapi::v::IntBase<size_t> actual_size(size);
  sapi::v::Array<uint8_t> actual(size);
  SAPI_ASSERT_OK(
      api_->ConvertWOFF2ToTTF(actual.PtrAfter(), size,
                              array.PtrBefore(), source->size())
  );

  auto expected = ReadFile("Roboto-Regular.ttf");
  ASSERT_THAT(expected, IsOk());
  ASSERT_EQ(actual_size.GetValue(), expected->size());
  ASSERT_TRUE(std::equal(actual.GetData(), actual.GetData() + expected->size(),
                         expected->data()));
}

TEST_F(Woff2Test, DecompressRawDeprecated) {
  auto source = ReadFile("Roboto-Regular.woff2");
  ASSERT_THAT(source, IsOk());

  size_t actual_size = woff2::ComputeWOFF2FinalSize(source->data(),
                                                    source->size());
  uint8_t* actual = new uint8_t[actual_size];
  woff2::ConvertWOFF2ToTTF(actual, actual_size,
                           source->data(), source->size());

  auto expected = ReadFile("Roboto-Regular.ttf");
  ASSERT_THAT(expected, IsOk());
  ASSERT_EQ(actual_size, expected->size());
  ASSERT_TRUE(std::equal(actual, actual + expected->size(),
                         expected->data()));
}

TEST_F(Woff2Test, DecompressRaw) {
  auto source = ReadFile("Roboto-Regular.woff2");
  ASSERT_THAT(source, IsOk());

  size_t actual_size = woff2::ComputeWOFF2FinalSize(source->data(),
                                                    source->size());
  uint8_t* actual = new uint8_t[actual_size];
  woff2::WOFF2MemoryOut out(actual, actual_size);
  ::woff2::ConvertWOFF2ToTTF(source->data(), source->size(), &out);

  auto expected = ReadFile("Roboto-Regular.ttf");
  ASSERT_THAT(expected, IsOk());
  ASSERT_EQ(actual_size, expected->size());
  ASSERT_TRUE(std::equal(actual, actual + expected->size(),
                         expected->data()));
}

}  // namespace
