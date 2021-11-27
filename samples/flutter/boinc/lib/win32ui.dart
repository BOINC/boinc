// Copyright (c) 2020, the Dart project authors.  Please see the AUTHORS file
// for details. All rights reserved. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.

import 'dart:ffi';

import 'package:ffi/ffi.dart';

main() {
  messageBox('こんにちは窓', 'Hello Windows');
}

/* MessageBoxW is the UTF16 (wchar_t) version of MessageBox.
int MessageBoxW(
  HWND    hWnd,
  LPCWSTR lpText,
  LPCWSTR lpCaption,
  UINT    uType
);
 */
typedef MessageBoxC = Int32 Function(
  Pointer hwnd,
  Pointer<Utf16> lpText,
  Pointer<Utf16> lpCaption,
  Uint32 uType,
);
typedef MessageBoxDart = int Function(
  Pointer parentWindow,
  Pointer<Utf16> message,
  Pointer<Utf16> caption,
  int type,
);

const MB_ABORTRETRYIGNORE = 0x00000002;
const MB_CANCELTRYCONTINUE = 0x00000006;
const MB_HELP = 0x00004000;
const MB_OK = 0x00000000;
const MB_OKCANCEL = 0x00000001;
const MB_RETRYCANCEL = 0x00000005;
const MB_YESNO = 0x00000004;
const MB_YESNOCANCEL = 0x00000003;

const MB_ICONEXCLAMATION = 0x00000030;
const MB_ICONWARNING = 0x00000030;
const MB_ICONINFORMATION = 0x00000040;
const MB_ICONASTERISK = 0x00000040;
const MB_ICONQUESTION = 0x00000020;
const MB_ICONSTOP = 0x00000010;
const MB_ICONERROR = 0x00000010;
const MB_ICONHAND = 0x00000010;

int messageBox(String message, String caption) {
  // Load user32.
  final user32 = DynamicLibrary.open('user32.dll');

  // Look up the `MessageBoxW` function.
  final messageBoxP =
      user32.lookupFunction<MessageBoxC, MessageBoxDart>('MessageBoxW');

  // Allocate pointers to Utf16 arrays containing the command arguments.
  final messageP = message.toNativeUtf16();
  final captionP = caption.toNativeUtf16();

  // Invoke the command, and free the pointers.
  final result =
      messageBoxP(nullptr, messageP, captionP, MB_OK | MB_ICONINFORMATION);
  calloc.free(messageP);
  calloc.free(captionP);

  return result;
}
