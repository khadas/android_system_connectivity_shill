//
// Copyright (C) 2015 The Android Open Source Project
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#include "proxy_dbus_shill_wifi_client.h"

namespace {
const int kRescanIntervalMilliseconds = 200;
const int kServiceDisconnectTimeoutMilliseconds = 5000;
const char kDefaultBgscanMethod[] = "default";
} // namespace

ProxyDbusShillWifiClient::ProxyDbusShillWifiClient(
    scoped_refptr<dbus::Bus> dbus_bus) {
  dbus_client_.reset(new ProxyDbusClient(dbus_bus));
}

bool ProxyDbusShillWifiClient::SetLogging() {
  dbus_client_->SetLogging(ProxyDbusClient::TECHNOLOGY_WIFI);
  return true;
}

bool ProxyDbusShillWifiClient::RemoveAllWifiEntries() {
  for (auto& profile_proxy : dbus_client_->GetProfileProxies()) {
    brillo::Any property_value;
    CHECK(dbus_client_->GetPropertyValueFromProfileProxy(
          profile_proxy.get(), shill::kEntriesProperty, &property_value));
    auto entry_ids = property_value.Get<std::vector<std::string>>();
    for (const auto& entry_id : entry_ids) {
      brillo::VariantDictionary entry_props;
      if (profile_proxy->GetEntry(entry_id, &entry_props, nullptr)) {
        if (entry_props[shill::kTypeProperty].Get<std::string>() ==
            shill::kTypeWifi) {
          profile_proxy->DeleteEntry(entry_id, nullptr);
        }
      }
    }
  }
  return true;
}

bool ProxyDbusShillWifiClient::ConfigureServiceByGuid(
    const std::string& guid,
    AutoConnectType autoconnect,
    const std::string& passphrase) {
  brillo::VariantDictionary service_params;
  if (guid.empty()) {
    return false;
  }
  SetAutoConnectInServiceParams(autoconnect, &service_params);
  if (!passphrase.empty()) {
    service_params.insert(std::make_pair(
        shill::kPassphraseProperty, brillo::Any(passphrase)));
  }
  return dbus_client_->ConfigureServiceByGuid(guid, service_params);
}

bool ProxyDbusShillWifiClient::ConfigureWifiService(
    const std::string& ssid,
    const std::string& security,
    const brillo::VariantDictionary& security_params,
    bool save_credentials,
    StationType station_type,
    bool hidden_network,
    const std::string& guid,
    AutoConnectType autoconnect) {
  brillo::VariantDictionary service_params;
  // Create the configure params dictionary.
  service_params.insert(std::make_pair(
      shill::kTypeProperty, brillo::Any(std::string(shill::kTypeWifi))));
  service_params.insert(std::make_pair(
      shill::kWifiHiddenSsid, brillo::Any(hidden_network)));
  service_params.insert(std::make_pair(
      shill::kSSIDProperty, brillo::Any(ssid)));
  service_params.insert(std::make_pair(
      shill::kSecurityClassProperty, brillo::Any(security)));
  service_params.insert(std::make_pair(
      shill::kModeProperty, brillo::Any(GetModeFromStationType(station_type))));
  SetAutoConnectInServiceParams(autoconnect, &service_params);
  service_params.insert(security_params.begin(), security_params.end());
  if (!guid.empty()) {
    service_params.insert(std::make_pair(
        shill::kGuidProperty, brillo::Any(guid)));
  }
  for (const auto& param: service_params) {
    LOG(INFO) << __func__ << ". Param: " << param.first << "="
              << param.second.TryGet<bool>() << ","
              << param.second.TryGet<int>() << ","
              << param.second.TryGet<std::string>() << ".";
  }
  return dbus_client_->ConfigureService(service_params);
}

bool ProxyDbusShillWifiClient::ConnectToWifiNetwork(
    const std::string& ssid,
    const std::string& security,
    const brillo::VariantDictionary& security_params,
    bool save_credentials,
    StationType station_type,
    bool hidden_network,
    const std::string& guid,
    AutoConnectType autoconnect,
    long discovery_timeout_milliseconds,
    long association_timeout_milliseconds,
    long configuration_timeout_milliseconds,
    long* discovery_time_milliseconds,
    long* association_time_milliseconds,
    long* configuration_time_milliseconds,
    std::string* failure_reason) {
  *discovery_time_milliseconds = -1;
  *association_time_milliseconds = -1;
  *configuration_time_milliseconds = -1;
  if (station_type != kStationTypeManaged &&
      station_type != kStationTypeIBSS) {
    *failure_reason = "FAIL(Invalid station type specified.)";
    return false;
  }
  if (hidden_network && !ConfigureWifiService(
          ssid, security, security_params, save_credentials, station_type,
          hidden_network, guid,autoconnect)) {
    *failure_reason = "FAIL(Failed to configure hidden SSID)";
    return false;
  }
  brillo::VariantDictionary service_params;
  service_params.insert(std::make_pair(
      shill::kTypeProperty, brillo::Any(std::string(shill::kTypeWifi))));
  service_params.insert(std::make_pair(
      shill::kNameProperty, brillo::Any(ssid)));
  service_params.insert(std::make_pair(
      shill::kSecurityClassProperty, brillo::Any(security)));
  service_params.insert(std::make_pair(
      shill::kModeProperty, brillo::Any(GetModeFromStationType(station_type))));
  for (const auto& param: service_params) {
    LOG(INFO) << __func__ << ". Param: " << param.first << "="
              << param.second.TryGet<bool>() << ","
              << param.second.TryGet<int>() << ","
              << param.second.TryGet<std::string>() << ".";
  }
  brillo::Any signal_strength;
  auto service = dbus_client_->WaitForMatchingServiceProxy(
      service_params, shill::kTypeWifi, discovery_timeout_milliseconds,
      kRescanIntervalMilliseconds, discovery_time_milliseconds);
  if (!service ||
      !dbus_client_->GetPropertyValueFromServiceProxy(
          service.get(), shill::kSignalStrengthProperty, &signal_strength) ||
      (signal_strength.Get<uint8>() < 0)) {
    *failure_reason = "FAIL(Discovery timed out)";
    return false;
  }

  for (const auto& security_param : security_params) {
    CHECK(service->SetProperty(security_param.first, security_param.second, nullptr));
  }
  if (!guid.empty()) {
    CHECK(service->SetProperty(shill::kGuidProperty, brillo::Any(guid), nullptr));
  }
  if (autoconnect != kAutoConnectTypeUnspecified) {
    CHECK(service->SetProperty(
        shill::kAutoConnectProperty, brillo::Any(bool(autoconnect)), nullptr));
  }

  brillo::ErrorPtr error;
  if (!service->Connect(&error) &&
      error->GetCode() != shill::kErrorResultAlreadyConnected) {
    *failure_reason = "FAIL(Failed to call connect)";
    return false;
  }

  brillo::Any final_value;
  std::vector<brillo::Any> associated_states = {
    brillo::Any(std::string("configuration")),
    brillo::Any(std::string("ready")),
    brillo::Any(std::string("portal")),
    brillo::Any(std::string("online")) };
  if (!dbus_client_->WaitForServiceProxyPropertyValueIn(
          service->GetObjectPath(), shill::kStateProperty, associated_states,
          association_timeout_milliseconds, &final_value,
          association_time_milliseconds)) {
    *failure_reason = "FAIL(Association timed out)";
    LOG(ERROR) << "FAIL(Association timed out). Final State: " <<
      final_value.Get<std::string>();
    return false;
  }

  std::vector<brillo::Any> configured_states = {
    brillo::Any(std::string("ready")),
    brillo::Any(std::string("portal")),
    brillo::Any(std::string("online")) };
  if (!dbus_client_->WaitForServiceProxyPropertyValueIn(
          service->GetObjectPath(), shill::kStateProperty, configured_states,
          configuration_timeout_milliseconds, nullptr,
          configuration_time_milliseconds)) {
    *failure_reason = "FAIL(Configuration timed out)";
    LOG(ERROR) << "FAIL(Configuration timed out). Final State: " <<
      final_value.Get<std::string>();
    return false;
  }

  *failure_reason = "SUCCESS(Connection successful)";
  return true;
}

bool ProxyDbusShillWifiClient::DisconnectFromWifiNetwork(
    const std::string& ssid,
    long disconnect_timeout_milliseconds,
    long* disconnect_time_milliseconds,
    std::string* failure_reason) {
  *disconnect_time_milliseconds = -1;
  if (disconnect_timeout_milliseconds == 0) {
    disconnect_timeout_milliseconds = kServiceDisconnectTimeoutMilliseconds;
  }
  brillo::VariantDictionary service_params;
  service_params.insert(std::make_pair(
      shill::kTypeProperty, brillo::Any(std::string(shill::kTypeWifi))));
  service_params.insert(std::make_pair(
      shill::kNameProperty, brillo::Any(ssid)));
  std::unique_ptr<ServiceProxy> service =
      dbus_client_->GetMatchingServiceProxy(service_params);
  if (!service) {
    *failure_reason = "FAIL(Service not found)";
    return false;
  }
  if (!service->Disconnect(nullptr)) {
    *failure_reason = "FAIL(Failed to call disconnect)";
    return false;
  }
  brillo::Any final_value;
  std::vector<brillo::Any> disconnect_states = {
    brillo::Any(std::string("idle")) };
  if (!dbus_client_->WaitForServiceProxyPropertyValueIn(
          service->GetObjectPath(), shill::kStateProperty, disconnect_states,
          disconnect_timeout_milliseconds, &final_value,
          disconnect_time_milliseconds)) {
    *failure_reason = "FAIL(Disconnection timed out)";
    return false;
  }

  *failure_reason = "SUCCESS(Disconnection successful)";
  return true;
}

bool ProxyDbusShillWifiClient::ConfigureBgScan(
    const std::string& interface_name,
    const std::string& method_name,
    uint16 short_interval,
    uint16 long_interval,
    int signal_threshold) {
  brillo::VariantDictionary device_params;
  device_params.insert(std::make_pair(
      shill::kNameProperty, brillo::Any(interface_name)));
  std::unique_ptr<DeviceProxy> device =
      dbus_client_->GetMatchingDeviceProxy(device_params);
  if (!device) {
    return false;
  }
  bool is_success = true;
  if (method_name == kDefaultBgscanMethod) {
    is_success &= device->ClearProperty(shill::kBgscanMethodProperty, nullptr);
  } else {
    is_success &= device->SetProperty(
        shill::kBgscanMethodProperty,
        brillo::Any(method_name),
        nullptr);
  }
  is_success &= device->SetProperty(
      shill::kBgscanShortIntervalProperty,
      brillo::Any(short_interval),
      nullptr);
  is_success &= device->SetProperty(
      shill::kScanIntervalProperty,
      brillo::Any(long_interval),
      nullptr);
  is_success &= device->SetProperty(
      shill::kBgscanSignalThresholdProperty,
      brillo::Any(signal_threshold),
      nullptr);
  return is_success;
}
std::vector<std::string> ProxyDbusShillWifiClient::GetActiveWifiSSIDs() {

  return std::vector<std::string>();
}

bool ProxyDbusShillWifiClient::WaitForServiceStates(
    std::string ssid,
    const std::vector<std::string>& expected_states,
    const int timeout_seconds,
    std::string& final_state,
    int& time) {
  return true;
}

bool ProxyDbusShillWifiClient::CreateProfile(std::string profile_name) {
  return dbus_client_->CreateProfile(profile_name);
}

bool ProxyDbusShillWifiClient::PushProfile(std::string profile_name) {
  return dbus_client_->PushProfile(profile_name);
}

bool ProxyDbusShillWifiClient::PopProfile(std::string profile_name) {
  if (profile_name.empty()) {
    return dbus_client_->PopAnyProfile();
  } else {
    return dbus_client_->PopProfile(profile_name);
  }
}

bool ProxyDbusShillWifiClient::RemoveProfile(std::string profile_name) {
  return dbus_client_->RemoveProfile(profile_name);
}

bool ProxyDbusShillWifiClient::CleanProfiles() {
  return true;
}

bool ProxyDbusShillWifiClient::DeleteEntriesForSsid(std::string ssid) {
  return true;
}

std::vector<std::string> ProxyDbusShillWifiClient::ListControlledWifiInterfaces() {
  return std::vector<std::string>();
}

bool ProxyDbusShillWifiClient::Disconnect(std::string ssid) {
  return true;
}

std::string ProxyDbusShillWifiClient::GetServiceOrder() {
  return std::string();
}

bool ProxyDbusShillWifiClient::SetServiceOrder(std::string service_order) {
  return true;
}

bool ProxyDbusShillWifiClient::SetSchedScan(bool enable) {
  return true;

}
std::string ProxyDbusShillWifiClient::GetPropertyOnDevice(
    std::string interface_name,
    std::string property_name) {
  return std::string();
}

bool ProxyDbusShillWifiClient::SetPropertyOnDevice(
    std::string interface_name,
    std::string property_name,
    std::string property_value) {
  return true;
}

bool ProxyDbusShillWifiClient::RequestRoam(
    std::string bssid,
    std::string interface_name) {

  return true;
}

bool ProxyDbusShillWifiClient::SetDeviceEnabled(
    std::string interface_name,
    bool enable) {
  return true;
}

bool ProxyDbusShillWifiClient::DiscoverTDLSLink(
    std::string interface_name,
    std::string peer_mac_address) {
  return true;
}

bool ProxyDbusShillWifiClient::EstablishTDLSLink(
    std::string interface_name,
    std::string peer_mac_address) {
  return true;
}

bool ProxyDbusShillWifiClient::QueryTDLSLink(
    std::string interface_name,
    std::string peer_mac_address) {
  return true;
}

bool ProxyDbusShillWifiClient::AddWakePacketSource(
    std::string interface_name,
    std::string source_ip_address) {
  return true;
}

bool ProxyDbusShillWifiClient::RemoveWakePacketSource(
    std::string interface_name,
    std::string source_ip_address) {
  return true;
}

bool ProxyDbusShillWifiClient::RemoveAllWakePacketSources(
    std::string interface_name) {
  return true;
}

void ProxyDbusShillWifiClient::SetAutoConnectInServiceParams(
    AutoConnectType autoconnect,
    brillo::VariantDictionary* service_params) {
  if (autoconnect != kAutoConnectTypeUnspecified) {
    service_params->insert(std::make_pair(
        shill::kAutoConnectProperty,
        brillo::Any(static_cast<bool>(autoconnect))));
  }
}
