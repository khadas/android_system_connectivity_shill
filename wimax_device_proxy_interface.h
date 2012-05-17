// Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SHILL_WIMAX_DEVICE_PROXY_INTERFACE_
#define SHILL_WIMAX_DEVICE_PROXY_INTERFACE_

#include <string>

#include <base/basictypes.h>

#include "shill/callbacks.h"

namespace shill {

class Error;

// These are the methods that a WiMaxManager.Device proxy must support. The
// interface is provided so that it can be mocked in tests.
class WiMaxDeviceProxyInterface {
 public:
  typedef base::Callback<void(const RpcIdentifiers &)> NetworksChangedCallback;

  virtual ~WiMaxDeviceProxyInterface() {}

  virtual void Enable(Error *error,
                      const ResultCallback &callback,
                      int timeout) = 0;
  virtual void Disable(Error *error,
                       const ResultCallback &callback,
                       int timeout) = 0;
  virtual void ScanNetworks(Error *error,
                            const ResultCallback &callback,
                            int timeout) = 0;
  virtual void Connect(Error *error,
                       const ResultCallback &callback,
                       int timeout) = 0;
  virtual void Disconnect(Error *error,
                          const ResultCallback &callback,
                          int timeout) = 0;

  virtual void set_networks_changed_callback(
      const NetworksChangedCallback &callback) = 0;

  // Properties.
  virtual uint8 Index(Error *error) = 0;
  virtual std::string Name(Error *error) = 0;
};

}  // namespace shill

#endif  // SHILL_WIMAX_DEVICE_PROXY_INTERFACE_H_
