// Copyright 2024 Google LLC.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "tflite/experimental/litert/core/dynamic_loading.h"

#include <dlfcn.h>

#ifndef __ANDROID__
#if __has_include(<link.h>)
#include <link.h>
#endif
#endif

#include <filesystem>  // NOLINT
#include <string>
#include <vector>

#include "absl/strings/str_format.h"
#include "absl/strings/string_view.h"
#include "tflite/experimental/litert/c/litert_common.h"
#include "tflite/experimental/litert/c/litert_logging.h"
#include "tflite/experimental/litert/core/filesystem.h"

namespace litert::internal {

LiteRtStatus OpenLib(const std::vector<std::string>& so_paths,
                     void** lib_handle, bool log_failure) {
  for (const auto& so_path : so_paths) {
    if (OpenLib(so_path, lib_handle, log_failure) == kLiteRtStatusOk) {
      return kLiteRtStatusOk;
    }
  }
  return kLiteRtStatusErrorDynamicLoading;
}

LiteRtStatus OpenLib(absl::string_view so_path, void** lib_handle,
                     bool log_failure) {
  LITERT_LOG(LITERT_VERBOSE, "Loading shared library: %s\n", so_path.data());
#ifdef RTLD_DEEPBIND
  void* res = ::dlopen(so_path.data(), RTLD_NOW | RTLD_LOCAL | RTLD_DEEPBIND);
#else
  void* res = ::dlopen(so_path.data(), RTLD_NOW | RTLD_LOCAL);
#endif

  if (res == nullptr) {
    if (log_failure) {
      LITERT_LOG(LITERT_WARNING, "Failed to load .so at path: %s\n",
                 so_path.data());
      LogDlError();
    }
    return kLiteRtStatusErrorDynamicLoading;
  }
  *lib_handle = res;
  LITERT_LOG(LITERT_INFO, "Successfully loaded shared library: %s\n",
             so_path.data());
  return kLiteRtStatusOk;
}

LiteRtStatus CloseLib(void* lib_handle) {
  if (0 != ::dlclose(lib_handle)) {
    LITERT_LOG(LITERT_ERROR, "Failed to close .so with error: %s", ::dlerror());
    return kLiteRtStatusErrorDynamicLoading;
  }
  return kLiteRtStatusOk;
}

namespace {

static constexpr absl::string_view kCompilerPluginLibPatternFmt =
    "%sCompilerPlugin";

static constexpr absl::string_view kSo = ".so";

LiteRtStatus FindLiteRtSharedLibsHelper(const std::string& search_path,
                                        std::vector<std::string>& results) {
  if (!Exists(search_path)) {
    return kLiteRtStatusErrorInvalidArgument;
  }

  const std::string compiler_plugin_lib_pattern =
      absl::StrFormat(kCompilerPluginLibPatternFmt, kLiteRtSharedLibPrefix);
  // TODO implement path glob in core/filesystem.h and remove filesystem
  // incldue from this file.
  for (const auto& entry : std::filesystem::directory_iterator(search_path)) {
    const auto& path = entry.path();
    if (entry.is_regular_file()) {
      const auto stem = path.stem().string();
      const auto ext = path.extension().string();
      if (stem.find(compiler_plugin_lib_pattern) == 0 && kSo == ext) {
        LITERT_LOG(LITERT_VERBOSE, "Found shared library: %s\n", path.c_str());
        results.push_back(path);
      }
    } else if (entry.is_directory()) {
      FindLiteRtSharedLibsHelper(path, results);
    }
  }

  return kLiteRtStatusOk;
}

}  // namespace

LiteRtStatus FindLiteRtSharedLibs(absl::string_view search_path,
                                  std::vector<std::string>& results) {
  std::string root(search_path.data());
  return FindLiteRtSharedLibsHelper(root, results);
}

}  // namespace litert::internal
