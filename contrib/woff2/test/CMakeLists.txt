# Copyright 2022 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     https://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

include(GoogleTest)

add_executable(woff2_sapi_test

        woff2_sapi_test.cc
)


target_link_libraries(woff2_sapi_test PRIVATE
        absl::flags
        absl::flags_parse
        sapi_contrib::woff2
        sapi::test_main

        # testing raw WOFF2
        PkgConfig::WOFF2_ENC
        PkgConfig::WOFF2_DEC
        PkgConfig::WOFF2_COMMON
)

gtest_discover_tests(woff2_sapi_test PROPERTIES
        ENVIRONMENT "TEST_DATA_DIR=${PROJECT_SOURCE_DIR}/testdata")
