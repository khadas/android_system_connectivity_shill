// Copyright (c) 2011 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shill/manager_dbus_adaptor.h"

#include <map>
#include <string>

using std::map;
using std::string;

namespace shill {

// TODO(cmasone): Figure out if we should be trying to own sub-interfaces.
// static
const char ManagerDBusAdaptor::kInterfaceName[] = SHILL_INTERFACE;
// ".Manager";
// static
const char ManagerDBusAdaptor::kPath[] = SHILL_PATH "/Manager";

ManagerDBusAdaptor::ManagerDBusAdaptor(DBus::Connection& conn, Manager *manager)
    : DBusAdaptor(conn, kPath),
      manager_(manager) {
}
ManagerDBusAdaptor::~ManagerDBusAdaptor() {}

void ManagerDBusAdaptor::UpdateRunning() {}

map<string, ::DBus::Variant> ManagerDBusAdaptor::GetProperties(
    ::DBus::Error &error) {
  return map<string, ::DBus::Variant>();
}

void ManagerDBusAdaptor::SetProperty(const string& name,
                                     const ::DBus::Variant& value,
                                     ::DBus::Error &error) {
}

string ManagerDBusAdaptor::GetState(::DBus::Error &error) {
  return string();
}

::DBus::Path ManagerDBusAdaptor::CreateProfile(const string& name,
                                               ::DBus::Error &error) {
  return ::DBus::Path();
}

void ManagerDBusAdaptor::RemoveProfile(const ::DBus::Path& path,
                                       ::DBus::Error &error) {
}

void ManagerDBusAdaptor::RequestScan(const string& ,
                                     ::DBus::Error &error) {
}

void ManagerDBusAdaptor::EnableTechnology(const string& ,
                                          ::DBus::Error &error) {
}

void ManagerDBusAdaptor::DisableTechnology(const string& ,
                                           ::DBus::Error &error) {
}

::DBus::Path ManagerDBusAdaptor::GetService(
    const map<string, ::DBus::Variant>& ,
    ::DBus::Error &error) {
  return ::DBus::Path();
}

::DBus::Path ManagerDBusAdaptor::GetWifiService(
    const map<string, ::DBus::Variant>& ,
    ::DBus::Error &error) {
  return ::DBus::Path();
}

void ManagerDBusAdaptor::ConfigureWifiService(
    const map<string, ::DBus::Variant>& ,
    ::DBus::Error &error) {
}

void ManagerDBusAdaptor::RegisterAgent(const ::DBus::Path& ,
                                       ::DBus::Error &error) {
}

void ManagerDBusAdaptor::UnregisterAgent(const ::DBus::Path& ,
                                         ::DBus::Error &error) {
}

string ManagerDBusAdaptor::GetDebugTags(::DBus::Error &error) {
  return string();
}

void ManagerDBusAdaptor::SetDebugTags(const string& ,
                                      ::DBus::Error &error) {
}

string ManagerDBusAdaptor::ListDebugTags(::DBus::Error &error) {
  return string();
}

uint32_t ManagerDBusAdaptor::GetDebugMask(::DBus::Error &error) {
  return 0;
}

void ManagerDBusAdaptor::SetDebugMask(const uint32_t& ,
                                      ::DBus::Error &error) {
}

string ManagerDBusAdaptor::GetServiceOrder(::DBus::Error &error) {
  return string();
}

void ManagerDBusAdaptor::SetServiceOrder(const string& ,
                                         ::DBus::Error &error) {
}

}  // namespace shill
