// Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SHILL_MOCK_DEVICE_INFO_
#define SHILL_MOCK_DEVICE_INFO_

#include <vector>

#include <base/basictypes.h>
#include <gmock/gmock.h>

#include "shill/device_info.h"

namespace shill {

class ByteString;
class ControlInterface;
class EventDispatcher;
class IPAddress;
class Manager;
class Metrics;

class MockDeviceInfo : public DeviceInfo {
 public:
  MockDeviceInfo(ControlInterface *control_interface,
                 EventDispatcher *dispatcher,
                 Metrics *metrics,
                 Manager *manager);
  virtual ~MockDeviceInfo();

  MOCK_CONST_METHOD1(GetDevice, DeviceRefPtr(int interface_index));
  MOCK_CONST_METHOD1(GetIndex, int(const std::string &interface_name));
  MOCK_CONST_METHOD2(GetMACAddress, bool(int interface_index,
                                         ByteString* address));
  MOCK_CONST_METHOD1(GetMACAddressFromKernel, ByteString(int interface_index));
  MOCK_CONST_METHOD3(GetByteCounts, bool(int interface_index,
                                         uint64 *rx_bytes,
                                         uint64 *tx_bytes));
  MOCK_CONST_METHOD2(GetFlags, bool(int interface_index,
                                    unsigned int* flags));
  MOCK_CONST_METHOD2(GetAddresses, bool(int interface_index,
                                        std::vector<AddressData>* addresses));
  MOCK_CONST_METHOD1(FlushAddresses, void(int interface_index));
  MOCK_CONST_METHOD2(HasOtherAddress,
                     bool(int interface_index,
                          const IPAddress &excluded_address));
  MOCK_CONST_METHOD1(CreateTunnelInterface,  bool(std::string *interface_name));
  MOCK_CONST_METHOD1(DeleteInterface, bool(int interface_index));
  MOCK_METHOD1(RegisterDevice, void(const DeviceRefPtr &));
  MOCK_METHOD1(DeregisterDevice, void(const DeviceRefPtr &));

 private:
  DISALLOW_COPY_AND_ASSIGN(MockDeviceInfo);
};

}  // namespace shill

#endif  // SHILL_MOCK_DEVICE_INFO_
