# ESP32 File Transfer Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add `fileReady` and `requestFile` WS message handling to SocketClient so the ESP32 can download files pushed from the browser and upload files on browser request, all over HTTPS.

**Architecture:** Two new private methods (`_downloadFile`, `_uploadFile`) in SocketClient handle the HTTP transfers synchronously. Two new optional callbacks in `SocketClientConfig_t` let the application react to received files and supply upload data. `main.cpp` in home-notify wires the `onFileReceived` callback to log incoming files to Serial.

**Tech Stack:** Arduino / ESP32, `HTTPClient`, `WiFiClientSecure` (already included in SocketClient), PlatformIO (`pio run`, `pio run --target upload`), serial monitor at 115200 baud.

---

## File Map

| File | Change |
|------|--------|
| `SocketClient/src/SocketClientDefs.h` | Add `FileReceivedFunction` + `GetFileFunction` typedefs; add two optional fields to `SocketClientConfig_t` |
| `SocketClient/src/SocketClient.h` | Add two private member variables + two private method declarations |
| `SocketClient/src/SocketClient.cpp` | Wire new fields in `init()`; add branches in `gotMessageSocket()`; implement `_downloadFile()` and `_uploadFile()` |
| `home-notify/src/main.cpp` | Add `onFileReceived` lambda to the config struct |

---

### Task 1: Add typedefs and config fields

**Files:**
- Modify: `SocketClient/src/SocketClientDefs.h:62-92`

- [ ] **Step 1: Add the two new `std::function` typedefs after the existing ones (line 65)**

  Open `SocketClient/src/SocketClientDefs.h`. After the four existing `typedef std::function<...>` lines (which end with `ConnectedFunction`), add:

  ```cpp
  typedef std::function<void(const String &filename, const uint8_t *buf, size_t size)> FileReceivedFunction;
  typedef std::function<size_t(const String &filename, uint8_t *buf, size_t maxSize)>  GetFileFunction;
  ```

- [ ] **Step 2: Add two optional fields to `SocketClientConfig_t`**

  Inside the `SocketClientConfig_t` struct, after the `ConnectedFunction connected;` field in the `/* functions */` block, add:

  ```cpp
  FileReceivedFunction onFileReceived = nullptr; // called after successful download; nullptr = no-op
  GetFileFunction      getFile        = nullptr; // returns bytes to upload; nullptr = built-in test payload
  ```

- [ ] **Step 3: Build to verify no compile errors**

  ```bash
  cd C:\Users\gregt\Documents\5PlatformIO\Projects\home-notify
  pio run
  ```

  Expected: `SUCCESS` with no errors or warnings about `SocketClientDefs.h`.

- [ ] **Step 4: Commit**

  ```bash
  git -C "C:\Users\gregt\Documents\5PlatformIO\Projects\SocketClient" add src/SocketClientDefs.h
  git -C "C:\Users\gregt\Documents\5PlatformIO\Projects\SocketClient" commit -m "feat: add FileReceivedFunction and GetFileFunction typedefs and config fields"
  ```

---

### Task 2: Add private members and method declarations to SocketClient.h

**Files:**
- Modify: `SocketClient/src/SocketClient.h:68-88`

- [ ] **Step 1: Add two private member variables**

  Open `SocketClient/src/SocketClient.h`. In the `protected:` section, after the four existing function pointer members (`sendStatus`, `receivedCommand`, `entityChanged`, `connected` at lines 68–72), add:

  ```cpp
  FileReceivedFunction _onFileReceived;
  GetFileFunction      _getFile;
  ```

- [ ] **Step 2: Add two private method declarations**

  In the same `protected:` section, after `void _init();` (around line 79), add:

  ```cpp
  void _downloadFile(const String &transferId, const String &filename, size_t size);
  void _uploadFile(const String &filename);
  ```

- [ ] **Step 3: Build to verify**

  ```bash
  cd C:\Users\gregt\Documents\5PlatformIO\Projects\home-notify
  pio run
  ```

  Expected: `SUCCESS`. (Methods are declared but not yet defined — the linker will complain only if they're called, which they aren't yet.)

- [ ] **Step 4: Commit**

  ```bash
  git -C "C:\Users\gregt\Documents\5PlatformIO\Projects\SocketClient" add src/SocketClient.h
  git -C "C:\Users\gregt\Documents\5PlatformIO\Projects\SocketClient" commit -m "feat: declare _downloadFile and _uploadFile private methods"
  ```

---

### Task 3: Wire init(), add gotMessageSocket() branches, add stub method definitions

**Files:**
- Modify: `SocketClient/src/SocketClient.cpp:222-275, 423-446`

This task makes the code compile end-to-end with stubs. Tasks 4 and 5 replace the stubs with real logic.

- [ ] **Step 1: Assign new config fields in `init()`**

  In `SocketClient::init(const SocketClientConfig_t *config)` (around line 423), after the four existing `ASSIGN_IF_NOT_NULLPTR` calls, add:

  ```cpp
  ASSIGN_IF_NOT_NULLPTR(_onFileReceived, config->onFileReceived);
  ASSIGN_IF_NOT_NULLPTR(_getFile,        config->getFile);
  ```

- [ ] **Step 2: Add branches to `gotMessageSocket()`**

  In `SocketClient::gotMessageSocket()` (around line 270), after the `} else if (strcmp(_doc["message"], "update") == 0 {` block's closing brace, add:

  ```cpp
  } else if (strcmp(_doc["message"], "fileReady") == 0) {
      _downloadFile(
          _doc["transferId"].as<String>(),
          _doc["filename"].as<String>(),
          _doc["size"].as<size_t>()
      );
  } else if (strcmp(_doc["message"], "requestFile") == 0) {
      _uploadFile(_doc["filename"].as<String>());
  }
  ```

- [ ] **Step 3: Add stub definitions at the bottom of SocketClient.cpp**

  At the end of `SocketClient.cpp`, after `getVersion()`, add:

  ```cpp
  void SocketClient::_downloadFile(const String &transferId, const String &filename, size_t size) {
      SC_LOGD(WS_TAG, "TODO: download '%s' transferId=%s", filename.c_str(), transferId.c_str());
  }

  void SocketClient::_uploadFile(const String &filename) {
      SC_LOGD(WS_TAG, "TODO: upload for filename='%s'", filename.c_str());
  }
  ```

- [ ] **Step 4: Build to verify clean compile**

  ```bash
  cd C:\Users\gregt\Documents\5PlatformIO\Projects\home-notify
  pio run
  ```

  Expected: `SUCCESS` with no linker errors.

- [ ] **Step 5: Commit**

  ```bash
  git -C "C:\Users\gregt\Documents\5PlatformIO\Projects\SocketClient" add src/SocketClient.cpp
  git -C "C:\Users\gregt\Documents\5PlatformIO\Projects\SocketClient" commit -m "feat: wire file transfer config fields and add gotMessageSocket dispatch branches (stubs)"
  ```

---

### Task 4: Implement `_downloadFile()`

**Files:**
- Modify: `SocketClient/src/SocketClient.cpp` (replace stub)

- [ ] **Step 1: Replace the `_downloadFile` stub with the real implementation**

  Replace the entire stub body with:

  ```cpp
  void SocketClient::_downloadFile(const String &transferId, const String &filename, size_t size) {
      WiFiClientSecure client;
      client.setInsecure();
      HTTPClient http;

      String url = String("https://") + _socketHostURL + "/api/devices/files/" + transferId;
      if (!http.begin(client, url)) {
          SC_LOGE(WS_TAG, "download: http.begin failed");
          return;
      }
      http.addHeader("x-mac-address", WiFi.macAddress());

      int code = http.GET();
      if (code != HTTP_CODE_OK) {
          SC_LOGE(WS_TAG, "download: HTTP %d", code);
          http.end();
          return;
      }

      size_t clampedSize = (size < 4096) ? size : 4096;
      uint8_t *buf = (uint8_t *)malloc(clampedSize);
      if (!buf) {
          SC_LOGE(WS_TAG, "download: OOM");
          http.end();
          return;
      }

      size_t actual = http.getStream().readBytes(buf, clampedSize);
      http.end();

      SC_LOGD(WS_TAG, "download: '%s' %u/%u bytes", filename.c_str(), actual, clampedSize);
      Serial.printf("[FileTransfer] received '%s' (%u bytes)\n", filename.c_str(), actual);
      Serial.print("[FileTransfer] first bytes: ");
      size_t preview = (actual < 32) ? actual : 32;
      for (size_t i = 0; i < preview; i++) {
          Serial.printf("%02X ", buf[i]);
      }
      Serial.println();

      if (_onFileReceived) {
          _onFileReceived(filename, buf, actual);
      }

      free(buf);
  }
  ```

- [ ] **Step 2: Build**

  ```bash
  cd C:\Users\gregt\Documents\5PlatformIO\Projects\home-notify
  pio run
  ```

  Expected: `SUCCESS`.

- [ ] **Step 3: Commit**

  ```bash
  git -C "C:\Users\gregt\Documents\5PlatformIO\Projects\SocketClient" add src/SocketClient.cpp
  git -C "C:\Users\gregt\Documents\5PlatformIO\Projects\SocketClient" commit -m "feat: implement _downloadFile — HTTPS GET with mac auth, logs bytes to Serial"
  ```

---

### Task 5: Implement `_uploadFile()`

**Files:**
- Modify: `SocketClient/src/SocketClient.cpp` (replace stub)

- [ ] **Step 1: Replace the `_uploadFile` stub with the real implementation**

  Replace the entire stub body with:

  ```cpp
  void SocketClient::_uploadFile(const String &filename) {
      const String boundary = "ESP32Boundary";
      String fname = filename.isEmpty() ? "upload.bin" : filename;

      // Get file content
      uint8_t *fileBuf = (uint8_t *)malloc(4096);
      if (!fileBuf) {
          SC_LOGE(WS_TAG, "upload: OOM (fileBuf)");
          return;
      }

      size_t fileSize = 0;
      if (_getFile) {
          fileSize = _getFile(fname, fileBuf, 4096);
      } else {
          const char *testPayload = "Hello from device!";
          fileSize = strlen(testPayload);
          memcpy(fileBuf, testPayload, fileSize);
      }

      // Build multipart body
      String header = "--" + boundary + "\r\n"
                      "Content-Disposition: form-data; name=\"file\"; filename=\"" + fname + "\"\r\n"
                      "Content-Type: application/octet-stream\r\n"
                      "\r\n";
      String footer = "\r\n--" + boundary + "--\r\n";

      size_t totalLen = header.length() + fileSize + footer.length();
      uint8_t *body = (uint8_t *)malloc(totalLen);
      if (!body) {
          SC_LOGE(WS_TAG, "upload: OOM (body)");
          free(fileBuf);
          return;
      }

      memcpy(body,                                   header.c_str(),  header.length());
      memcpy(body + header.length(),                 fileBuf,         fileSize);
      memcpy(body + header.length() + fileSize,      footer.c_str(),  footer.length());
      free(fileBuf);

      WiFiClientSecure client;
      client.setInsecure();
      HTTPClient http;

      String url = String("https://") + _socketHostURL + "/api/devices/files/upload";
      if (!http.begin(client, url)) {
          SC_LOGE(WS_TAG, "upload: http.begin failed");
          free(body);
          return;
      }
      http.addHeader("x-mac-address", WiFi.macAddress());
      http.addHeader("Content-Type", "multipart/form-data; boundary=" + boundary);

      int code = http.POST(body, totalLen);
      SC_LOGD(WS_TAG, "upload: HTTP %d", code);
      Serial.printf("[FileTransfer] upload response: %d\n", code);

      free(body);
      http.end();
  }
  ```

- [ ] **Step 2: Build**

  ```bash
  cd C:\Users\gregt\Documents\5PlatformIO\Projects\home-notify
  pio run
  ```

  Expected: `SUCCESS`.

- [ ] **Step 3: Commit**

  ```bash
  git -C "C:\Users\gregt\Documents\5PlatformIO\Projects\SocketClient" add src/SocketClient.cpp
  git -C "C:\Users\gregt\Documents\5PlatformIO\Projects\SocketClient" commit -m "feat: implement _uploadFile — multipart HTTPS POST with mac auth"
  ```

---

### Task 6: Wire `onFileReceived` callback in home-notify

**Files:**
- Modify: `home-notify/src/main.cpp:95-109`

- [ ] **Step 1: Add `onFileReceived` to the config struct**

  Open `home-notify/src/main.cpp`. In the `SocketClientConfig_t config = { ... }` initialiser (around line 95), add `onFileReceived` after the existing `connected` field:

  ```cpp
  .onFileReceived = [](const String &filename, const uint8_t *buf, size_t size) {
      Serial.printf("[FileTransfer] received '%s' (%u bytes)\n", filename.c_str(), size);
      Serial.print("[FileTransfer] first bytes: ");
      size_t preview = (size < 32) ? size : 32;
      for (size_t i = 0; i < preview; i++) {
          Serial.printf("%02X ", buf[i]);
      }
      Serial.println();
  },
  ```

  Leave `getFile` absent (defaults to `nullptr`) — SocketClient will use the `"Hello from device!"` test payload.

- [ ] **Step 2: Build**

  ```bash
  cd C:\Users\gregt\Documents\5PlatformIO\Projects\home-notify
  pio run
  ```

  Expected: `SUCCESS`.

- [ ] **Step 3: Commit**

  ```bash
  git -C "C:\Users\gregt\Documents\5PlatformIO\Projects\home-notify" add src/main.cpp
  git -C "C:\Users\gregt\Documents\5PlatformIO\Projects\home-notify" commit -m "feat: wire onFileReceived callback — logs filename and hex bytes to Serial"
  ```

---

### Task 7: Flash and verify end-to-end

**Files:** none (verification only)

- [ ] **Step 1: Flash and open monitor**

  ```bash
  cd C:\Users\gregt\Documents\5PlatformIO\Projects\home-notify
  pio run --target upload && pio device monitor
  ```

  Expected on boot: normal startup messages, `---Setup complete---`, WebSocket connects.

- [ ] **Step 2: Verify `fileReady` flow (Browser → Device)**

  In the web UI, send a small file (e.g., a `.txt` containing `Hello`) to the device via the "Send to Device" button.

  Expected serial output (within ~2 s):
  ```
  D ( WS ) Got data: {"message":"fileReady","transferId":"<uuid>","filename":"hello.txt","size":5}
  [FileTransfer] received 'hello.txt' (5 bytes)
  [FileTransfer] first bytes: 48 65 6C 6C 6F
  ```

  `48 65 6C 6C 6F` = ASCII `Hello`. If you see a different byte count, compare against the file you uploaded.

- [ ] **Step 3: Verify `requestFile` flow (Browser requests from Device)**

  In the web UI, click "Request File" (no filename hint needed for the test payload).

  Expected serial output:
  ```
  D ( WS ) Got data: {"message":"requestFile"}
  [FileTransfer] upload response: 200
  ```

  In the browser, the "Download" card should appear. Download the file — its content should be `Hello from device!`.

  If the response code is 403: the device MAC is not recognised by the server. Check that the device is registered and `isOwned` passes for this MAC.

  If the response code is 413: the payload exceeded 4096 bytes — should not happen with the test payload.
