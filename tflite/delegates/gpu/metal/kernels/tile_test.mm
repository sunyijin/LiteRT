/* Copyright 2021 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#import <XCTest/XCTest.h>

#include "tflite/delegates/gpu/common/status.h"
#include "tflite/delegates/gpu/common/tasks/tile_test_util.h"
#include "tflite/delegates/gpu/metal/kernels/test_util.h"

@interface TileTest : XCTestCase
@end

@implementation TileTest {
  tflite::gpu::metal::MetalExecutionEnvironment exec_env_;
}

- (void)testTileChannels {
  auto status = TileChannelsTest(&exec_env_);
  XCTAssertTrue(status.ok(), @"%s", std::string(status.message()).c_str());
}

- (void)testTileChannelsX4 {
  auto status = TileChannelsX4Test(&exec_env_);
  XCTAssertTrue(status.ok(), @"%s", std::string(status.message()).c_str());
}

- (void)testTileWidth {
  auto status = TileWidthTest(&exec_env_);
  XCTAssertTrue(status.ok(), @"%s", std::string(status.message()).c_str());
}

- (void)testTileHeight {
  auto status = TileHeightTest(&exec_env_);
  XCTAssertTrue(status.ok(), @"%s", std::string(status.message()).c_str());
}

- (void)testTileHWC {
  auto status = TileHWCTest(&exec_env_);
  XCTAssertTrue(status.ok(), @"%s", std::string(status.message()).c_str());
}

@end
