// Copyright (c) 2013 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SHILL_CRYPTO_UTIL_PROXY_H_
#define SHILL_CRYPTO_UTIL_PROXY_H_

#include <string>
#include <vector>

#include <base/basictypes.h>
#include <base/cancelable_callback.h>
#include <base/memory/weak_ptr.h>
#include <base/stringprintf.h>
#include <gtest/gtest_prod.h>  // for FRIEND_TEST

#include "shill/callbacks.h"
#include "shill/error.h"
#include "shill/io_handler.h"
#include "shill/minijail.h"
#include "shill/proto_bindings/shims/protos/crypto_util.pb.h"

namespace shill {

class EventDispatcher;
class FileIO;
class GLib;
class ProcessKiller;

class CryptoUtilProxy : public base::SupportsWeakPtr<CryptoUtilProxy> {
 public:
  static const char kCommandVerify[];
  static const char kCommandEncrypt[];
  static const char kCryptoUtilShimPath[];

  CryptoUtilProxy(EventDispatcher *dispatcher, GLib *glib);
  virtual ~CryptoUtilProxy();

  // Verify credentials for the currently connected endpoint of
  // |connected_service|.  This is a fairly expensive/time consuming operation.
  // Returns true if we've succeeded in kicking off a job to an external shim
  // to verify credentials.  |result_callback| will be called with the actual
  // result of the job, either true, or false with a descriptive error.
  //
  // |certificate| should be a device certificate in PEM format.
  // |public_key| is a base64 encoded DER RSAPublicKey format public key.
  // |nonce| has no particular format requirements.
  // |signed_data| is the base64 encoded signed string given by the device.
  // |destination_udn| has no format requirements.
  // |ssid| has no constraints.
  // |bssid| should be in the human readable format: 00:11:22:33:44:55.
  virtual bool VerifyDestination(const std::string &certificate,
                                 const std::string &public_key,
                                 const std::string &nonce,
                                 const std::string &signed_data,
                                 const std::string &destination_udn,
                                 const std::vector<uint8_t> &ssid,
                                 const std::string &bssid,
                                 const ResultBoolCallback &result_callback,
                                 Error *error);

  // Encrypt |data| under |public_key|.  This is a fairly time consuming
  // process.  Returns true if we've succeeded in kicking off a job to an
  // external shim to sign the data.  |result_callback| will be called with the
  // results of the operation: an empty string and a descriptive error or the
  // base64 encoded bytes of the encrypted data.
  //
  // |public_key| is a base64 encoded DER RSAPublicKey format public key.
  // |data| has no particular format requirements.
  virtual bool EncryptData(const std::string &public_key,
                           const std::string &data,
                           const ResultStringCallback &result_callback,
                           Error *error);
 private:
  friend class CryptoUtilProxyTest;
  friend class MockCryptoUtilProxy;
  FRIEND_TEST(CryptoUtilProxyTest, BasicAPIUsage);
  FRIEND_TEST(CryptoUtilProxyTest, FailuresReturnValues);
  FRIEND_TEST(CryptoUtilProxyTest, OnlyOneInstanceInFlightAtATime);
  FRIEND_TEST(CryptoUtilProxyTest, ShimLifeTime);
  FRIEND_TEST(CryptoUtilProxyTest, TimeoutsTriggerFailure);
  FRIEND_TEST(CryptoUtilProxyTest, ShimCleanedBeforeCallback);

  static const char kDestinationVerificationUser[];
  static const int kShimJobTimeoutMilliseconds;

  // Helper method for parsing the proto buffer return codes sent back by the
  // shim.
  static bool ParseResponseReturnCode(int proto_return_code, Error *e);

  // Kick off a run of the shim to verify credentials or sign data.  Callers
  // pass in the command they want to run on the shim (literally a command line
  // argument to the shim), and a handler to handle the result.  The handler is
  // called both on errors, timeouts, and success.  Behind the scenes, we first
  // send |input| down to the shim through a pipe to its stdin, then wait for
  // bytes to comes back over a pipe connected to the shim's stdout.
  virtual bool StartShimForCommand(const std::string &command,
                                   const std::string &input,
                                   const StringCallback &result_handler);
  // This is the big hammer we use to clean up past shim state.
  virtual void CleanupShim();
  virtual void OnShimDeath();

  // Callbacks that handle IO operations between shill and the shim.  Called by
  // GLib on changes in file descriptor state.
  void HandleShimStdinReady(int fd);
  void HandleShimOutput(InputData *data);
  void HandleShimError(const Error &error);
  void HandleShimTimeout();
  // Used to handle the final result of both operations.  |result| is a
  // seriallized protocol buffer or an empty string on error.  On error,
  // |error| is filled in with an appropriate error condition.
  // |result_callback| is a callback from the original caller (the manager).
  void HandleVerifyResult(const ResultBoolCallback &result_handler,
                          const std::string &result,
                          const Error &error);
  void HandleEncryptResult(const ResultStringCallback &result_handler,
                           const std::string &result,
                           const Error &error);

  EventDispatcher *dispatcher_;
  GLib *glib_;
  Minijail *minijail_;
  ProcessKiller *process_killer_;
  FileIO *file_io_;
  std::string input_buffer_;
  std::string::const_iterator next_input_byte_;
  std::string output_buffer_;
  int shim_stdin_;
  int shim_stdout_;
  pid_t shim_pid_;
  scoped_ptr<IOHandler> shim_stdin_handler_;
  scoped_ptr<IOHandler> shim_stdout_handler_;
  StringCallback result_handler_;
  base::CancelableClosure shim_job_timeout_callback_;

  DISALLOW_COPY_AND_ASSIGN(CryptoUtilProxy);
};

}  // namespace shill

#endif  // SHILL_CRYPTO_UTIL_PROXY_H_
