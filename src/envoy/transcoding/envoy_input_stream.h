/* Copyright 2017 Istio Authors. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include "precompiled/precompiled.h"

#include "common/buffer/buffer_impl.h"
#include "contrib/endpoints/src/grpc/transcoding/transcoder_input_stream.h"

namespace Grpc {

class EnvoyInputStream
    : public google::api_manager::transcoding::TranscoderInputStream {
 public:
  // Add a buffer to input stream, will consume all buffer from parameter
  // if the stream is not finished
  void Move(Buffer::Instance &instance);

  // Mark the buffer is finished
  void Finish() { finished_ = true; }

  // TranscoderInputStream
  virtual bool Next(const void **data, int *size) override;
  virtual void BackUp(int count) override;
  virtual bool Skip(int count) override { return false; }  // Not implemented
  virtual google::protobuf::int64 ByteCount() const override {
    return byte_count_;
  }
  virtual int64_t BytesAvailable() const override;

 private:
  Buffer::OwnedImpl buffer_;
  int position_{0};
  int64_t byte_count_{0};
  bool finished_{false};
};

}  // namespace Grpc