// Copyright (c) 2014 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shill/mock_mobile_operator_info.h"

#include <gmock/gmock.h>

namespace shill {

MockMobileOperatorInfo::MockMobileOperatorInfo(EventDispatcher *dispatcher)
    : MobileOperatorInfo(dispatcher) {}

MockMobileOperatorInfo::~MockMobileOperatorInfo() {}

void MockMobileOperatorInfo::SetEmptyDefaultsForProperties() {
  ON_CALL(*this, mccmnc()).WillByDefault(ReturnRef(empty_mccmnc_));
  ON_CALL(*this, operator_name())
      .WillByDefault(ReturnRef(empty_operator_name_));
}

}  // namespace shill
