// Copyright (c) 2011 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SHILL_CONTROL_INTERFACE_
#define SHILL_CONTROL_INTERFACE_

#include <algorithm>
#include <string>

#include <base/logging.h>

namespace shill {

class Device;
class DeviceAdaptorInterface;
class IPConfig;
class IPConfigAdaptorInterface;
class Manager;
class ManagerAdaptorInterface;
class Profile;
class ProfileAdaptorInterface;
class Service;
class ServiceAdaptorInterface;

// This is the Interface for an object factory that creates adaptor objects
class ControlInterface {
 public:
  virtual ~ControlInterface() {}
  virtual DeviceAdaptorInterface *CreateDeviceAdaptor(Device *device) = 0;
  virtual IPConfigAdaptorInterface *CreateIPConfigAdaptor(
      IPConfig *ipconfig) = 0;
  virtual ManagerAdaptorInterface *CreateManagerAdaptor(Manager *manager) = 0;
  virtual ProfileAdaptorInterface *CreateProfileAdaptor(Profile *profile) = 0;
  virtual ServiceAdaptorInterface *CreateServiceAdaptor(Service *service) = 0;

  static void RpcIdToStorageId(std::string *rpc_id) {
    CHECK(rpc_id);
    std::replace(rpc_id->begin(), rpc_id->end(), '/', '_');
  }
};

}  // namespace shill
#endif  // SHILL_CONTROL_INTERFACE_
