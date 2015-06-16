// Copyright (c) 2011 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SHILL_SUPPLICANT_MOCK_SUPPLICANT_PROCESS_PROXY_H_
#define SHILL_SUPPLICANT_MOCK_SUPPLICANT_PROCESS_PROXY_H_

#include <map>
#include <string>

#include <base/macros.h>
#include <gmock/gmock.h>

#include "shill/dbus_variant_gmock_printer.h"
#include "shill/supplicant/supplicant_process_proxy_interface.h"

namespace shill {

class MockSupplicantProcessProxy : public SupplicantProcessProxyInterface {
 public:
  MockSupplicantProcessProxy();
  ~MockSupplicantProcessProxy() override;

  MOCK_METHOD1(CreateInterface,
               ::DBus::Path(
                   const std::map<std::string, ::DBus::Variant>& args));
  MOCK_METHOD1(GetInterface, ::DBus::Path(const std::string& ifname));
  MOCK_METHOD1(RemoveInterface, void(const ::DBus::Path& path));
  MOCK_METHOD0(GetDebugLevel, std::string());
  MOCK_METHOD1(SetDebugLevel, void(const std::string& level));

 private:
  DISALLOW_COPY_AND_ASSIGN(MockSupplicantProcessProxy);
};

}  // namespace shill

#endif  // SHILL_SUPPLICANT_MOCK_SUPPLICANT_PROCESS_PROXY_H_
