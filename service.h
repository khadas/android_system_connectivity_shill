// Copyright (c) 2011 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SHILL_SERVICE_
#define SHILL_SERVICE_

#include <string>

#include <base/memory/ref_counted.h>
#include <base/memory/scoped_ptr.h>

namespace shill {

class Connection;
class Configuration;
class ControlInterface;
class Device;
class Endpoint;
class EventDispatcher;
class ServiceAdaptorInterface;

class Service : public base::RefCounted<Service> {
 public:
  enum ConnectFailure {
    kServiceFailureUnknown,
    kServiceFailureActivationFailure,
    kServiceFailureOutOfRange,
    kServiceFailurePinMissing,
    kServiceFailureConfigurationFailed,
    kServiceFailureBadCredentials,
    kServiceFailureNeedEVDO,
    kServiceFailureNeedHomeNetwork,
    kServiceFailureOTASPFailure,
    kServiceFailureAAAFailure
  };
  enum ConnectState {
    kServiceStateUnknown,
    kServiceStateIdle,
    kServiceStateAssociating,
    kServiceStateConfiguring,
    kServiceStateConnected,
    kServiceStateDisconnected,
    kServiceStateFailure
  };

  // A constructor for the Service object
  Service(ControlInterface *control_interface,
          EventDispatcher *dispatcher,
          Device *device,
          const std::string& name);
  virtual ~Service();
  virtual void Connect() = 0;
  virtual void Disconnect() = 0;

  // Returns a string that is guaranteed to uniquely identify this
  // Service instance.
  virtual const std::string& UniqueName() { return name_; }

 private:
  const std::string name_;
  bool available_;
  bool configured_;
  bool auto_connect_;
  Configuration *configuration_;
  Connection *connection_;
  Device *device_;
  Endpoint *endpoint_;
  scoped_ptr<ServiceAdaptorInterface> adaptor_;
  friend class ServiceAdaptorInterface;
};

}  // namespace shill

#endif  // SHILL_SERVICE_
