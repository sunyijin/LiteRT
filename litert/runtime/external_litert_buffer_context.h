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

#ifndef TENSORFLOW_LITE_EXPERIMENTAL_LITERT_RUNTIME_EXTERNAL_LITERT_BUFFER_CONTEXT_H_
#define TENSORFLOW_LITE_EXPERIMENTAL_LITERT_RUNTIME_EXTERNAL_LITERT_BUFFER_CONTEXT_H_

#include <memory>
#include <utility>

#include "absl/container/flat_hash_map.h"  // from @com_google_absl
#include "litert/c/litert_common.h"
#include "litert/c/litert_tensor_buffer_requirements.h"
#include "litert/cc/litert_expected.h"
#include "litert/cc/litert_tensor_buffer.h"
#include "litert/cc/litert_tensor_buffer_requirements.h"
#include "tensorflow/lite/c/c_api_opaque.h"  // from @org_tensorflow
#include "tensorflow/lite/c/c_api_types.h"  // from @org_tensorflow
#include "tensorflow/lite/c/common.h"  // from @org_tensorflow

namespace litert {
namespace internal {

class ExternalLiteRtBufferContext : public TfLiteExternalContext {
 public:
  ExternalLiteRtBufferContext() = default;
  ~ExternalLiteRtBufferContext() = default;

  // Registers a tensor buffer requirements for the given tensor.
  // The registered TensorBufferRequirements object is owned by
  // ExternalLiteRtBufferContext.
  // Note: Currently, the system pre-registers tensor buffer requirements before
  // they're actually used. A more efficient approach would be to query
  // DelegateKernel only when these requirements are needed.
  LiteRtStatus RegisterBufferRequirement(
      const TfLiteOpaqueTensor* tensor,
      TensorBufferRequirements&& buffer_requirements);

  inline LiteRtStatus RegisterBufferRequirement(
      const TfLiteTensor* tensor,
      TensorBufferRequirements&& buffer_requirements) {
    return RegisterBufferRequirement(
        reinterpret_cast<const TfLiteOpaqueTensor*>(tensor),
        std::move(buffer_requirements));
  }

  inline LiteRtStatus RegisterLiteRtBufferRequirement(
      const TfLiteTensor* tensor,
      LiteRtTensorBufferRequirements& litert_buffer_requirements) {
    return RegisterBufferRequirement(
        reinterpret_cast<const TfLiteOpaqueTensor*>(tensor),
        TensorBufferRequirements(litert_buffer_requirements,
                                 /*owned=*/true));
  }

  // Gets a registered tensor buffer requirements for the given tensor.
  // The returned TensorBufferRequirements object is still owned by
  // ExternalLiteRtBufferContext.
  litert::Expected<TensorBufferRequirements*> GetBufferRequirement(
      const TfLiteOpaqueTensor* tensor);

  inline litert::Expected<TensorBufferRequirements*> GetBufferRequirement(
      const TfLiteTensor* tensor) {
    return GetBufferRequirement(
        reinterpret_cast<const TfLiteOpaqueTensor*>(tensor));
  }

  // Registers a tensor buffer for the given tensor.
  // The registered TensorBuffer object is owned by ExternalLiteRtBufferContext.
  LiteRtStatus RegisterTensorBuffer(const TfLiteOpaqueTensor* tensor,
                                    TensorBuffer&& tensor_buffer);

  inline LiteRtStatus RegisterTensorBuffer(const TfLiteTensor* tensor,
                                           TensorBuffer&& tensor_buffer) {
    return RegisterTensorBuffer(
        reinterpret_cast<const TfLiteOpaqueTensor*>(tensor),
        std::move(tensor_buffer));
  }

  // Gets a registered tensor buffer for the given tensor.
  // The returned TensorBuffer object is duplication (reference counted)
  // of registered TensorBuffer.
  litert::Expected<TensorBuffer> GetTensorBuffer(
      const TfLiteOpaqueTensor* tensor);

  inline litert::Expected<TensorBuffer> GetTensorBuffer(
      const TfLiteTensor* tensor) {
    return GetTensorBuffer(reinterpret_cast<const TfLiteOpaqueTensor*>(tensor));
  }

  // Creates a tensor buffer for the given tensor.
  // The callers takes ownership of the returned TensorBuffer object.
  litert::Expected<TensorBuffer> CreateBufferForTensor(
      const TfLiteOpaqueTensor* tensor);

  inline litert::Expected<TensorBuffer> CreateBufferForTensor(
      const TfLiteTensor* tensor) {
    return CreateBufferForTensor(
        reinterpret_cast<const TfLiteOpaqueTensor*>(tensor));
  }

  // Sets the async execution mode. It's set by CompiledModel and used by
  // DelegateKernel to decide whether to use async execution mode.
  inline void SetAsyncExecutionMode(bool async_execution_mode) {
    async_execution_mode_ = async_execution_mode;
  }

  // Returns true if the async execution mode is set.
  inline bool IsAsyncExecutionMode() const { return async_execution_mode_; }

 private:
  absl::flat_hash_map<const TfLiteOpaqueTensor*, TensorBufferRequirements>
      buffer_requirements_;
  absl::flat_hash_map<const TfLiteOpaqueTensor*, TensorBuffer> tensor_buffers_;

  ExternalLiteRtBufferContext(const ExternalLiteRtBufferContext&) = delete;
  ExternalLiteRtBufferContext& operator=(const ExternalLiteRtBufferContext&) =
      delete;

  bool async_execution_mode_ = false;
};

}  // namespace internal
}  // namespace litert

#endif  // TENSORFLOW_LITE_EXPERIMENTAL_LITERT_RUNTIME_EXTERNAL_LITERT_BUFFER_CONTEXT_H_
