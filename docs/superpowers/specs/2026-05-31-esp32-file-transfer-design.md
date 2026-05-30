# ESP32 File Transfer — Device-Side Design Spec

**Date:** 2026-05-31
**Status:** Approved

---

## Overview

Device-side implementation of the file transfer feature defined in the server spec (`2026-05-30-file-transfer-design.md`). The device receives files pushed from the browser and uploads files on browser request. All HTTP logic lives in **SocketClient**; home-notify wires optional callbacks but adds no HTTP code of its own.

Max payload: **4096 bytes**. Transport: **HTTPS** to `api.sensordata.space:443`. Auth: `x-mac-address` header (device MAC, checked via `isOwned` on the server).

---

## Architecture

Two new WS message types arrive at the device. SocketClient dispatches them to private methods that execute the HTTP transfer synchronously (blocking in the loop for ~1–2 s, acceptable for this device).

### Browser → Device (download)

```
Server WS → device: { message: 'fileReady', transferId, filename, size }
    ↓  gotMessageSocket() new branch
SocketClient::_downloadFile(transferId, filename, size)
    ↓  HTTPS GET /api/devices/files/:transferId  (x-mac-address header)
    ↓  reads ≤ 4 KB response bytes
    ↓  logs filename, size, first 32 bytes to Serial
    ↓  calls _onFileReceived(filename, buf, actualLen)  ← optional app callback
```

### Browser requests file from Device (upload)

```
Server WS → device: { message: 'requestFile', filename? }
    ↓  gotMessageSocket() new branch
SocketClient::_uploadFile(filename)
    ↓  calls _getFile(filename, buf, 4096) → actualSize  ← optional app callback
    ↓  (if _getFile is nullptr: uses built-in "Hello from device!" test payload)
    ↓  builds multipart/form-data body in heap buffer
    ↓  HTTPS POST /api/devices/files/upload  (x-mac-address header)
    ↓  logs HTTP response code to Serial
```

---

## Component Changes

### `SocketClientDefs.h`

Two new typedef:

```cpp
typedef std::function<void(const String &filename, const uint8_t *buf, size_t size)> FileReceivedFunction;
typedef std::function<size_t(const String &filename, uint8_t *buf, size_t maxSize)>  GetFileFunction;
```

Two new optional fields in `SocketClientConfig_t`:

```cpp
FileReceivedFunction onFileReceived = nullptr; // called after successful download
GetFileFunction      getFile        = nullptr; // returns bytes to upload; nullptr → test payload
```

### `SocketClient.h`

Private members:

```cpp
FileReceivedFunction _onFileReceived;
GetFileFunction      _getFile;
```

Private methods:

```cpp
void _downloadFile(const String &transferId, const String &filename, size_t size);
void _uploadFile(const String &filename);
```

### `SocketClient.cpp`

**`init()`** — assign the two new config fields (same pattern as existing callbacks).

**`gotMessageSocket()`** — two new `else if` branches appended after existing handlers:

```cpp
} else if (strcmp(_doc["message"], "fileReady") == 0) {
    _downloadFile(_doc["transferId"].as<String>(),
                  _doc["filename"].as<String>(),
                  _doc["size"].as<size_t>());
} else if (strcmp(_doc["message"], "requestFile") == 0) {
    _uploadFile(_doc["filename"].as<String>());
}
```

**`_downloadFile(transferId, filename, size)`**:

- Create `WiFiClientSecure` with `setInsecure()`
- `HTTPClient::begin()` → `https://<_socketHostURL>/api/devices/files/<transferId>`
- Add header `x-mac-address: WiFi.macAddress()`
- `GET` → check HTTP 200
- `getStream().readBytes(buf, min(size, 4096))` → use actual bytes read
- Log filename, actual size, first 32 bytes to Serial
- Call `_onFileReceived(filename, buf, actualLen)` if set
- `http.end()`

**`_uploadFile(filename)`**:

- If `_getFile` is set: call it to fill `buf`, get `fileSize`
- Otherwise: copy `"Hello from device!"` into `buf`, set `fileSize`
- Build multipart body:

```
--ESP32Boundary\r\n
Content-Disposition: form-data; name="file"; filename="<filename or 'upload.bin'>"\r\n
Content-Type: application/octet-stream\r\n
\r\n
<file bytes>
\r\n--ESP32Boundary--\r\n
```

- `malloc` a single buffer (header + file bytes + footer, ≤ ~4.5 kB); if null, log and return
- Create `WiFiClientSecure` with `setInsecure()`
- `HTTPClient::begin()` → `https://<_socketHostURL>/api/devices/files/upload`
- Add header `x-mac-address: WiFi.macAddress()`
- Add header `Content-Type: multipart/form-data; boundary=ESP32Boundary`
- `POST(body, totalLen)`
- Log HTTP response code to Serial
- `free(body)`, `http.end()`

### `main.cpp` (home-notify)

Add `onFileReceived` lambda to `SocketClientConfig_t`:

```cpp
.onFileReceived = [](const String &filename, const uint8_t *buf, size_t size) {
    Serial.printf("[FileTransfer] received '%s' (%u bytes)\n", filename.c_str(), size);
    Serial.print("[FileTransfer] first bytes: ");
    for (size_t i = 0; i < min(size, (size_t)32); i++) {
        Serial.printf("%02X ", buf[i]);
    }
    Serial.println();
},
```

Leave `getFile` unset (`nullptr`) — SocketClient uses the built-in test payload.

---

## Error Handling

| Condition | Behaviour |
|-----------|-----------|
| HTTP status ≠ 200 on download | Log code to Serial; discard partial data; do not call `_onFileReceived` |
| HTTP status ≠ 200/201 on upload | Log code to Serial; no retry |
| `malloc` returns null | Log OOM to Serial; skip upload |
| `readBytes` returns fewer bytes than declared `size` | Use actual count; log discrepancy |
| SSL | `setInsecure()` — same trust level as OTA manager |
| `_doc["size"]` > 4096 | Clamp read to 4096 bytes |

---

## WS Messages Handled

| Message | Fields consumed |
|---------|----------------|
| `fileReady` | `transferId` (String), `filename` (String), `size` (size_t) |
| `requestFile` | `filename` (String, may be null/empty) |

These are the only new branches added to `gotMessageSocket`. No changes to existing message handling.
