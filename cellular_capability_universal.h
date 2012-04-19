// Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SHILL_CELLULAR_CAPABILITY_UNIVERSAL_
#define SHILL_CELLULAR_CAPABILITY_UNIVERSAL_

#include <deque>
#include <string>
#include <vector>

#include <base/memory/scoped_ptr.h>
#include <base/memory/weak_ptr.h>
#include <gtest/gtest_prod.h>  // for FRIEND_TEST
#include <mm/ModemManager-enums.h>

#include "shill/accessor_interface.h"
#include "shill/cellular.h"
#include "shill/cellular_capability.h"
#include "shill/mm1_modem_modem3gpp_proxy_interface.h"
#include "shill/mm1_modem_modemcdma_proxy_interface.h"
#include "shill/mm1_modem_proxy_interface.h"
#include "shill/mm1_modem_simple_proxy_interface.h"
#include "shill/mm1_sim_proxy_interface.h"


struct mobile_provider;

namespace shill {

// CellularCapabilityUniversal handles modems using the
// org.chromium.ModemManager1 DBUS interface.  This class is used for
// all types of modems, i.e. CDMA, GSM, and LTE modems.
class CellularCapabilityUniversal : public CellularCapability {
 public:
  typedef std::vector<DBusPropertiesMap> ScanResults;
  typedef DBusPropertiesMap ScanResult;

  CellularCapabilityUniversal(Cellular *cellular, ProxyFactory *proxy_factory);

  // Inherited from CellularCapability.
  virtual void StartModem(Error *error, const ResultCallback &callback);
  virtual void StopModem(Error *error, const ResultCallback &callback);
  virtual void Connect(const DBusPropertiesMap &properties, Error *error,
                       const ResultCallback &callback);
  virtual void Disconnect(Error *error, const ResultCallback &callback);
  virtual void Activate(const std::string &carrier,
                        Error *error, const ResultCallback &callback);

  virtual void OnServiceCreated();
  virtual void UpdateStatus(const DBusPropertiesMap &properties);
  virtual void SetupConnectProperties(DBusPropertiesMap *properties);
  virtual void GetRegistrationState();
  virtual void GetProperties();
  virtual void Register(const ResultCallback &callback);

  virtual void RegisterOnNetwork(const std::string &network_id,
                                 Error *error,
                                 const ResultCallback &callback);
  virtual bool IsRegistered();
  virtual std::string CreateFriendlyServiceName();
  virtual void RequirePIN(const std::string &pin, bool require,
                          Error *error, const ResultCallback &callback);
  virtual void EnterPIN(const std::string &pin,
                        Error *error, const ResultCallback &callback);
  virtual void UnblockPIN(const std::string &unblock_code,
                          const std::string &pin,
                          Error *error, const ResultCallback &callback);
  virtual void ChangePIN(const std::string &old_pin,
                         const std::string &new_pin,
                         Error *error, const ResultCallback &callback);

  virtual void Scan(Error *error, const ResultCallback &callback);
  virtual std::string GetNetworkTechnologyString() const;
  virtual std::string GetRoamingStateString() const;
  virtual void GetSignalQuality();
  virtual std::string GetTypeString() const { return "Universal"; }
  virtual void OnDBusPropertiesChanged(
      const std::string &interface,
      const DBusPropertiesMap &changed_properties,
      const std::vector<std::string> &invalidated_properties);
  virtual void OnModem3GPPPropertiesChanged(
      const DBusPropertiesMap &properties);

 protected:
  virtual void InitProxies();
  virtual void ReleaseProxies();

  virtual bool AllowRoaming();

 private:
  friend class CellularTest;
  friend class CellularCapabilityUniversalTest;
  friend class CellularCapabilityTest;
  FRIEND_TEST(CellularCapabilityUniversalTest, CreateDeviceFromProperties);
  FRIEND_TEST(CellularCapabilityUniversalTest, CreateFriendlyServiceName);
  FRIEND_TEST(CellularCapabilityUniversalTest, GetIMEI);
  FRIEND_TEST(CellularCapabilityUniversalTest, GetIMSI);
  FRIEND_TEST(CellularCapabilityUniversalTest, GetMSISDN);
  FRIEND_TEST(CellularCapabilityUniversalTest, GetSPN);
  FRIEND_TEST(CellularCapabilityUniversalTest, RequirePIN);
  FRIEND_TEST(CellularCapabilityUniversalTest, EnterPIN);
  FRIEND_TEST(CellularCapabilityUniversalTest, UnblockPIN);
  FRIEND_TEST(CellularCapabilityUniversalTest, ChangePIN);
  FRIEND_TEST(CellularCapabilityUniversalTest, InitAPNList);
  FRIEND_TEST(CellularCapabilityUniversalTest, ParseScanResult);
  FRIEND_TEST(CellularCapabilityUniversalTest, ParseScanResultProviderLookup);
  FRIEND_TEST(CellularCapabilityUniversalTest, RegisterOnNetwork);
  FRIEND_TEST(CellularCapabilityUniversalTest, Scan);
  FRIEND_TEST(CellularCapabilityUniversalTest, SetAccessTechnologies);
  FRIEND_TEST(CellularCapabilityUniversalTest, SetHomeProvider);
  FRIEND_TEST(CellularCapabilityUniversalTest, UpdateOperatorInfo);
  FRIEND_TEST(CellularCapabilityUniversalTest, GetRegistrationState);
  FRIEND_TEST(CellularCapabilityUniversalTest, OnDBusPropertiesChanged);
  FRIEND_TEST(CellularCapabilityUniversalTest, SetupApnTryList);
  FRIEND_TEST(CellularCapabilityTest, AllowRoaming);
  FRIEND_TEST(CellularCapabilityTest, TryApns);
  FRIEND_TEST(CellularTest, StartUniversalRegister);
  FRIEND_TEST(ModemTest, CreateDeviceFromProperties);

  // Methods used in starting a modem
  void Start_EnableModemCompleted(const ResultCallback &callback,
                                  const Error &error);
  void Start_RegisterCompleted(const ResultCallback &callback,
                               const Error &error);

  // Methods used in stopping a modem
  void Stop_DisconnectCompleted(const ResultCallback &callback,
                               const Error &error);
  void Stop_Disable(const ResultCallback &callback);
  void Stop_DisableCompleted(const ResultCallback &callback,
                             const Error &error);

  // TOOD(jglasgow): document what this does!!!!!
  void SetAccessTechnologies(uint32 access_technologies);

  // Sets the upper level information about the home cellular provider from the
  // modem's IMSI and SPN.
  void SetHomeProvider();

  // Updates the Universal operator name and country based on a newly
  // obtained network id.
  void UpdateOperatorInfo();

  // Updates the serving operator on the active service.
  void UpdateServingOperator();

  // Initializes the |apn_list_| property based on the current |home_provider_|.
  void InitAPNList();

  Stringmap ParseScanResult(const ScanResult &result);

  KeyValueStore SimLockStatusToProperty(Error *error);

  void SetupApnTryList();
  void FillConnectPropertyMap(DBusPropertiesMap *properties);

  void HelpRegisterDerivedKeyValueStore(
      const std::string &name,
      KeyValueStore(CellularCapabilityUniversal::*get)(Error *error),
      void(CellularCapabilityUniversal::*set)(
          const KeyValueStore &value, Error *error));

  // Signal callbacks
  virtual void OnNetworkModeSignal(uint32 mode);

  // Property Change notification handlers
  virtual void OnModemPropertiesChanged(
      const DBusPropertiesMap &properties,
      const std::vector<std::string> &invalidated_properties);

  // TODO(jglasgow): install generic property change notification handler
  virtual void On3GPPRegistrationChanged(MMModem3gppRegistrationState state,
                                        const std::string &operator_code,
                                        const std::string &operator_name);
  virtual void OnSignalQualityChanged(uint32 quality);

  // Updates the sim_path_ variable and creates a new proxy to the
  // DBUS ModemManager1.Sim interface
  virtual void OnSimPathChanged(const std::string &sim_path);

  // Method callbacks
  virtual void OnRegisterReply(const ResultCallback &callback,
                               const Error &error);
  virtual void OnScanReply(const ResultCallback &callback,
                           const ScanResults &results,
                           const Error &error);
  // TODO(njw): None of the above callbacks need to be virtual.
  void OnConnectReply(const ResultCallback &callback,
                      const DBus::Path &bearer,
                      const Error &error);

  scoped_ptr<mm1::ModemModem3gppProxyInterface> modem_3gpp_proxy_;
  scoped_ptr<mm1::ModemModemCdmaProxyInterface> modem_cdma_proxy_;
  scoped_ptr<mm1::ModemProxyInterface> modem_proxy_;
  scoped_ptr<mm1::ModemSimpleProxyInterface> modem_simple_proxy_;
  scoped_ptr<mm1::SimProxyInterface> sim_proxy_;

  base::WeakPtrFactory<CellularCapabilityUniversal> weak_ptr_factory_;

  MMModem3gppRegistrationState registration_state_;
  MMModemCdmaRegistrationState cdma_registration_state_;
  uint32 access_technologies_;  // Bits based on MMModemAccessTechnology
  Cellular::Operator serving_operator_;
  std::string spn_;
  mobile_provider *home_provider_;
  std::string desired_network_;

  // Properties.
  std::string carrier_;
  std::string esn_;
  std::string firmware_revision_;
  std::string hardware_revision_;
  std::string imei_;
  std::string imsi_;
  std::string manufacturer_;
  std::string mdn_;
  std::string meid_;
  std::string min_;
  std::string model_id_;
  std::string selected_network_;
  Stringmaps found_networks_;
  std::deque<Stringmap> apn_try_list_;
  bool scanning_supported_;
  bool scanning_;
  uint16 scan_interval_;
  SimLockStatus sim_lock_status_;
  Stringmaps apn_list_;
  std::string sim_path_;
  DBus::Path bearer_path_;

  static unsigned int friendly_service_name_id_;

  DISALLOW_COPY_AND_ASSIGN(CellularCapabilityUniversal);
};

}  // namespace shill

#endif  // SHILL_CELLULAR_CAPABILITY_UNIVERSAL_
