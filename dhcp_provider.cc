// Copyright (c) 2011 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shill/dhcp_provider.h"

#include <base/logging.h>

#include "shill/dhcpcd_proxy.h"

namespace shill {

DHCPProvider::DHCPProvider() {
  VLOG(2) << __func__;
}

DHCPProvider::~DHCPProvider() {
  VLOG(2) << __func__;
}

DHCPProvider* DHCPProvider::GetInstance() {
  return Singleton<DHCPProvider>::get();
}

void DHCPProvider::Init(DBus::Connection *connection) {
  VLOG(2) << __func__;
  listener_.reset(new DHCPCDListener(this, connection));
}

DHCPConfigRefPtr DHCPProvider::CreateConfig(const Device &device) {
  VLOG(2) << __func__;
  return DHCPConfigRefPtr(new DHCPConfig(this, device));
}

DHCPConfigRefPtr DHCPProvider::GetConfig(unsigned int pid) {
  VLOG(2) << __func__;
  PIDConfigMap::iterator it = configs_.find(pid);
  if (it == configs_.end()) {
    return DHCPConfigRefPtr(NULL);
  }
  return it->second;
}

void DHCPProvider::BindPID(unsigned int pid, DHCPConfigRefPtr config) {
  VLOG(2) << __func__ << " pid: " << pid;
  configs_[pid] = config;
}

void DHCPProvider::UnbindPID(unsigned int pid) {
  VLOG(2) << __func__ << " pid: " << pid;
  configs_.erase(pid);
}

}  // namespace shill
