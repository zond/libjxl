// Copyright (c) the JPEG XL Project Authors. All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#ifndef TOOLS_BENCHMARK_BENCHMARK_CODEC_H_
#define TOOLS_BENCHMARK_BENCHMARK_CODEC_H_

#include <jxl/memory_manager.h>

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "lib/extras/codec_in_out.h"
#include "lib/extras/packed_image.h"
#include "lib/jxl/base/span.h"
#include "lib/jxl/base/status.h"
#include "tools/benchmark/benchmark_args.h"
#include "tools/benchmark/benchmark_stats.h"
#include "tools/speed_stats.h"
#include "tools/thread_pool_internal.h"

namespace jpegxl {
namespace tools {

using ::jxl::CodecInOut;
using ::jxl::Span;
using ::jxl::Status;
using ::jxl::extras::PackedPixelFile;

// Thread-compatible.
class ImageCodec {
 public:
  explicit ImageCodec(const BenchmarkArgs& args)
      : args_(args),
        butteraugli_target_(1.0f),
        q_target_(100.0f),
        bitrate_target_(0.0f) {}

  virtual ~ImageCodec() = default;

  void set_description(const std::string& desc) { description_ = desc; }
  const std::string& description() const { return description_; }

  virtual Status ParseParameters(const std::string& parameters);

  virtual Status ParseParam(const std::string& param);

  virtual Status Compress(const std::string& filename,
                          const PackedPixelFile& ppf, ThreadPool* pool,
                          std::vector<uint8_t>* compressed,
                          jpegxl::tools::SpeedStats* speed_stats) = 0;

  virtual Status Decompress(const std::string& filename,
                            Span<const uint8_t> compressed, ThreadPool* pool,
                            PackedPixelFile* ppf,
                            jpegxl::tools::SpeedStats* speed_stats) = 0;

  virtual void GetMoreStats(BenchmarkStats* stats) {}

  virtual bool IgnoreAlpha() const { return false; }

  virtual Status CanRecompressJpeg() const { return false; }
  virtual Status RecompressJpeg(const std::string& filename,
                                const std::vector<uint8_t>& data,
                                std::vector<uint8_t>* compressed,
                                jpegxl::tools::SpeedStats* speed_stats) {
    return false;
  }

  virtual std::string GetErrorMessage() const { return error_message_; }

 protected:
  const BenchmarkArgs& args_;
  std::string params_;
  std::string description_;
  float butteraugli_target_;
  float q_target_;
  float bitrate_target_;
  std::string error_message_;
};

using ImageCodecPtr = std::unique_ptr<ImageCodec>;

// Creates an image codec by name, e.g. "jxl" to get a new instance of the
// jxl codec. Optionally, behind a colon, parameters can be specified,
// then ParseParameters of the codec gets called with the part behind the colon.
ImageCodecPtr CreateImageCodec(const std::string& description,
                               JxlMemoryManager* memory_manager);

}  // namespace tools
}  // namespace jpegxl

#endif  // TOOLS_BENCHMARK_BENCHMARK_CODEC_H_
