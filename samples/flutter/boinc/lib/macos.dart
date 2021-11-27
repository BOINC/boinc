// Copyright (c) 2019, the Dart project authors.  Please see the AUTHORS file
// for details. All rights reserved. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.

import 'dart:ffi' as ffi;
import 'package:ffi/ffi.dart';


int systemMacOS(String command) {
  var result = system(command);
  return result;
}

/*
#include <stdlib.h>
int system(const char *string);

https://man.openbsd.org/system.3
*/

// C header typedef:
typedef SystemC = ffi.Int32 Function(ffi.Pointer<Utf8> command);

// Dart header typedef
typedef SystemDart = int Function(ffi.Pointer<Utf8> command);

int system(String command) {
  // Load `stdlib`. On MacOS this is in libSystem.dylib.
  final dylib = ffi.DynamicLibrary.open('/usr/lib/libSystem.dylib');

  // Look up the `system` function.
  final systemP = dylib.lookupFunction<SystemC, SystemDart>('system');

  // Allocate a pointer to a Utf8 array containing our command.
  final cmdP = command.toNativeUtf8();

  // Invoke the command, and free the pointer.
  int result = systemP(cmdP);
  calloc.free(cmdP);

  return result;
}
