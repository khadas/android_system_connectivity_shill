// Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SHILL_ARP_PACKET_H_
#define SHILL_ARP_PACKET_H_

#include "shill/byte_string.h"
#include "shill/ip_address.h"

namespace shill {

// ArpPacket encapsulates the task of creating and parsing
// Address Resolution Protocol (ARP) packets for IP and
// IPv6 protocols on Ethernet (or Ethernet-like) networks.
class ArpPacket {
 public:
  ArpPacket();
  ArpPacket(const IPAddress &local_ip, const IPAddress &remote_ip,
            const ByteString &local_mac, const ByteString &remote_mac);
  virtual ~ArpPacket();

  // Parse a payload and save to local parameters.
  bool ParseReply(const ByteString &packet);

  // Output a payload from local parameters.
  bool FormatRequest(ByteString *packet) const;

  // Getters and seters.
  const IPAddress &local_ip_address() const { return local_ip_address_; }
  void set_local_ip_address(const IPAddress &address) {
    local_ip_address_ = address;
  }

  const IPAddress &remote_ip_address() const { return remote_ip_address_; }
  void set_remote_ip_address(const IPAddress &address) {
    remote_ip_address_ = address;
  }

  const ByteString &local_mac_address() const { return local_mac_address_; }
  void set_local_mac_address(const ByteString &address) {
    local_mac_address_ = address;
  }

  const ByteString &remote_mac_address() const { return remote_mac_address_; }
  void set_remote_mac_address(const ByteString &address) {
    remote_mac_address_ = address;
  }

 private:
  friend class ArpPacketTest;

  IPAddress local_ip_address_;
  IPAddress remote_ip_address_;
  ByteString local_mac_address_;
  ByteString remote_mac_address_;

  DISALLOW_COPY_AND_ASSIGN(ArpPacket);
};

}  // namespace shill

#endif  // SHILL_ARP_PACKET_H_
