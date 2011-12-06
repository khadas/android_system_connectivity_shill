// Copyright (c) 2011 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shill/metrics.h"

#include <base/lazy_instance.h>
#include <base/logging.h>
#include <base/string_util.h>
#include <base/stringprintf.h>

#include "shill/wifi_service.h"

using std::string;
using std::tr1::shared_ptr;

namespace shill {

static base::LazyInstance<Metrics> g_metrics(base::LINKER_INITIALIZED);

// static
const char Metrics::kMetricNetworkChannel[] = "Network.Shill.%s.Channel";
const int Metrics::kMetricNetworkChannelMax = Metrics::kWiFiChannelMax;
const char Metrics::kMetricNetworkServiceErrors[] =
    "Network.Shill.ServiceErrors";
const int Metrics::kMetricNetworkServiceErrorsMax = Service::kFailureMax;
const char Metrics::kMetricTimeToConfigMilliseconds[] =
    "Network.Shill.%s.TimeToConfig";
const char Metrics::kMetricTimeToJoinMilliseconds[] =
    "Network.Shill.%s.TimeToJoin";
const char Metrics::kMetricTimeToOnlineMilliseconds[] =
    "Network.Shill.%s.TimeToOnline";
const char Metrics::kMetricTimeToPortalMilliseconds[] =
    "Network.Shill.%s.TimeToPortal";
const int Metrics::kTimerHistogramMaxMilliseconds = 45 * 1000;
const int Metrics::kTimerHistogramMinMilliseconds = 1;
const int Metrics::kTimerHistogramNumBuckets = 50;

// static
const uint16 Metrics::kWiFiBandwidth5MHz = 5;
const uint16 Metrics::kWiFiBandwidth20MHz = 20;
const uint16 Metrics::kWiFiFrequency2412 = 2412;
const uint16 Metrics::kWiFiFrequency2472 = 2472;
const uint16 Metrics::kWiFiFrequency2484 = 2484;
const uint16 Metrics::kWiFiFrequency5170 = 5170;
const uint16 Metrics::kWiFiFrequency5180 = 5180;
const uint16 Metrics::kWiFiFrequency5230 = 5230;
const uint16 Metrics::kWiFiFrequency5240 = 5240;
const uint16 Metrics::kWiFiFrequency5320 = 5320;
const uint16 Metrics::kWiFiFrequency5500 = 5500;
const uint16 Metrics::kWiFiFrequency5700 = 5700;
const uint16 Metrics::kWiFiFrequency5745 = 5745;
const uint16 Metrics::kWiFiFrequency5825 = 5825;

Metrics::Metrics() : library_(&metrics_library_) {
  metrics_library_.Init();
  chromeos_metrics::TimerReporter::set_metrics_lib(library_);
}

Metrics::~Metrics() {}

// static
Metrics *Metrics::GetInstance() {
  return g_metrics.Pointer();
}

// static
Metrics::WiFiChannel Metrics::WiFiFrequencyToChannel(uint16 frequency) {
  WiFiChannel channel = kWiFiChannelUndef;
  if (kWiFiFrequency2412 <= frequency && frequency <= kWiFiFrequency2472) {
    if (((frequency - kWiFiFrequency2412) % kWiFiBandwidth5MHz) == 0)
      channel = static_cast<WiFiChannel>(
                    kWiFiChannel2412 +
                    (frequency - kWiFiFrequency2412) / kWiFiBandwidth5MHz);
  } else if (frequency == kWiFiFrequency2484) {
    channel = kWiFiChannel2484;
  } else if (kWiFiFrequency5170 <= frequency &&
             frequency <= kWiFiFrequency5230) {
    if ((frequency % kWiFiBandwidth20MHz) == 0)
      channel = static_cast<WiFiChannel>(
                    kWiFiChannel5180 +
                    (frequency - kWiFiFrequency5180) / kWiFiBandwidth20MHz);
    if ((frequency % kWiFiBandwidth20MHz) == 10)
      channel = static_cast<WiFiChannel>(
                    kWiFiChannel5170 +
                    (frequency - kWiFiFrequency5170) / kWiFiBandwidth20MHz);
  } else if (kWiFiFrequency5240 <= frequency &&
             frequency <= kWiFiFrequency5320) {
    if (((frequency - kWiFiFrequency5180) % kWiFiBandwidth20MHz) == 0)
      channel = static_cast<WiFiChannel>(
                    kWiFiChannel5180 +
                    (frequency - kWiFiFrequency5180) / kWiFiBandwidth20MHz);
  } else if (kWiFiFrequency5500 <= frequency &&
             frequency <= kWiFiFrequency5700) {
    if (((frequency - kWiFiFrequency5500) % kWiFiBandwidth20MHz) == 0)
      channel = static_cast<WiFiChannel>(
                    kWiFiChannel5500 +
                    (frequency - kWiFiFrequency5500) / kWiFiBandwidth20MHz);
  } else if (kWiFiFrequency5745 <= frequency &&
             frequency <= kWiFiFrequency5825) {
    if (((frequency - kWiFiFrequency5745) % kWiFiBandwidth20MHz) == 0)
      channel = static_cast<WiFiChannel>(
                    kWiFiChannel5745 +
                    (frequency - kWiFiFrequency5745) / kWiFiBandwidth20MHz);
  }
  CHECK(kWiFiChannelUndef <= channel && channel < kWiFiChannelMax);

  if (channel == kWiFiChannelUndef)
    LOG(WARNING) << "no mapping for frequency " << frequency;
  else
    VLOG(3) << "map " << frequency << " to " << channel;

  return channel;
}

void Metrics::RegisterService(const Service *service) {
  shared_ptr<ServiceMetrics> service_metrics(new ServiceMetrics);
  services_metrics_[service] = service_metrics;
  service_metrics->service = service;
  InitializeCommonServiceMetrics(service);
  service->InitializeCustomMetrics();
}

void Metrics::DeregisterService(const Service *service) {
  services_metrics_.erase(service);
}

void Metrics::AddServiceStateTransitionTimer(
    const Service *service,
    const string &histogram_name,
    Service::ConnectState start_state,
    Service::ConnectState stop_state) {
  ServiceMetricsLookupMap::iterator it = services_metrics_.find(service);
  if (it == services_metrics_.end()) {
    VLOG(1) << "service not found";
    DCHECK(false);
    return;
  }
  ServiceMetrics *service_metrics = it->second.get();
  CHECK(start_state < stop_state);
  chromeos_metrics::TimerReporter *timer =
      new chromeos_metrics::TimerReporter(histogram_name,
                                          kTimerHistogramMinMilliseconds,
                                          kTimerHistogramMaxMilliseconds,
                                          kTimerHistogramNumBuckets);
  service_metrics->timers.push_back(timer);  // passes ownership.
  service_metrics->start_on_state[start_state].push_back(timer);
  service_metrics->stop_on_state[stop_state].push_back(timer);
}

void Metrics::NotifyDefaultServiceChanged(const Service */*service*/) {
  // TODO(thieule): Handle the case when the default service has changed.
  // crosbug.com/24438
}

void Metrics::NotifyServiceStateChanged(const Service *service,
                                        Service::ConnectState new_state) {
  ServiceMetricsLookupMap::iterator it = services_metrics_.find(service);
  if (it == services_metrics_.end()) {
    VLOG(1) << "service not found";
    DCHECK(false);
    return;
  }
  ServiceMetrics *service_metrics = it->second.get();
  UpdateServiceStateTransitionMetrics(service_metrics, new_state);

  if (new_state == Service::kStateFailure)
    SendServiceFailure(service);

  if (new_state != Service::kStateReady)
    return;

  service->SendPostReadyStateMetrics();
}

string Metrics::GetFullMetricName(const char *metric_name,
                                  Technology::Identifier technology_id) {
  string technology = Technology::NameFromIdentifier(technology_id);
  technology[0] = base::ToUpperASCII(technology[0]);
  return base::StringPrintf(metric_name, technology.c_str());
}

void Metrics::NotifyServiceDisconnect(const Service */*service*/,
                                      bool /*manual_disconnect*/) {
  // TODO(thieule): Handle service disconnects.
  // crosbug.com/23253
}

void Metrics::NotifyPower() {
  // TODO(thieule): Handle suspend and resume.
  // crosbug.com/24440
}

bool Metrics::SendEnumToUMA(const string &name, int sample, int max) {
  return library_->SendEnumToUMA(name, sample, max);
}

void Metrics::InitializeCommonServiceMetrics(const Service *service) {
  Technology::Identifier technology = service->technology();
  string histogram = GetFullMetricName(kMetricTimeToConfigMilliseconds,
                                       technology);
  AddServiceStateTransitionTimer(
      service,
      histogram,
      Service::kStateConfiguring,
      Service::kStateReady);
  histogram = GetFullMetricName(kMetricTimeToPortalMilliseconds, technology);
  AddServiceStateTransitionTimer(
      service,
      histogram,
      Service::kStateReady,
      Service::kStatePortal);
  histogram = GetFullMetricName(kMetricTimeToOnlineMilliseconds, technology);
  AddServiceStateTransitionTimer(
      service,
      histogram,
      Service::kStateReady,
      Service::kStateOnline);
}

void Metrics::UpdateServiceStateTransitionMetrics(
    ServiceMetrics *service_metrics,
    Service::ConnectState new_state) {
  TimerReportersList::iterator it;
  TimerReportersList &start_timers = service_metrics->start_on_state[new_state];
  for (it = start_timers.begin(); it != start_timers.end(); ++it)
    (*it)->Start();

  TimerReportersList &stop_timers = service_metrics->stop_on_state[new_state];
  for (it = stop_timers.begin(); it != stop_timers.end(); ++it) {
    (*it)->Stop();
    (*it)->ReportMilliseconds();
  }
}

void Metrics::SendServiceFailure(const Service *service) {
  library_->SendEnumToUMA(kMetricNetworkServiceErrors,
                          service->failure(),
                          kMetricNetworkServiceErrorsMax);
}

void Metrics::set_library(MetricsLibraryInterface *library) {
  chromeos_metrics::TimerReporter::set_metrics_lib(library);
  library_ = library;
}

}  // namespace shill
