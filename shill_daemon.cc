// Copyright (c) 2010 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdio.h>
#include <glib.h>

#include <algorithm>
#include <string>
#include <vector>

#include "shill/shill_daemon.h"
#include "shill/control_interface.h"
#include "shill/dbus_control.h"

using std::max;
using std::min;
using std::string;
using std::vector;

namespace shill {

static const char kTaggedFilePath[] = "/var/lib/shill";

// Daemon: Main for connection manager.  Starts main process and holds event
// loop.

Daemon::Daemon(Config *config, ControlInterface *control)
  : config_(config),
    control_(control),
    manager_(control_, &dispatcher_) {
}
Daemon::~Daemon() {}


void Daemon::Run() {
  (void) new EventDispatcherTest(&dispatcher_);
  GMainLoop* loop = g_main_loop_new(NULL, false);
  printf("Running main loop.\n");
  g_main_loop_run(loop);
}


}  // namespace shill
