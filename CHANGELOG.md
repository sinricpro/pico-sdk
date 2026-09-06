# Changelog

## [2.0.0]

- BREAKING CHANGE: `sinricpro_begin()` no longer returns the result of the cloud connection. It brings up local control, attempts the connection, and returns true either way; an unreachable cloud is no longer fatal. Callers that treated `false` as "no network" should check `sinricpro_is_connected()` instead.
- BREAKING CHANGE: the response `message` field carried a generated UUID. It now carries status text - `"OK"`, `"Device did not handle request"` or `"Signature is invalid"` - matching the other SinricPro SDKs.
- feat: Local control. The device answers signed SinricPro commands over the LAN on UDP 3333, so it keeps working while the SinricPro cloud is unreachable. Requests are dispatched through the same device handlers as cloud requests, so existing sketches need no changes.
- feat: The listener binds to any address and joins `224.9.9.9`, so it takes unicast, multicast and subnet broadcast. Broadcast matters: it is how the app finds a device on networks that drop multicast.
- fix: The outgoing queue was gated on the websocket being connected, so a LAN reply could never be sent while the cloud was down - the one case local control exists for. Each message is now routed by its own origin.
- fix: `sinricpro_begin()` returned the result of the cloud connection, so an unreachable cloud looked fatal to the caller. It now brings up local control first and returns true; check `sinricpro_is_connected()` for cloud state.
- fix: A request that failed signature verification was dropped without a reply. A LAN request now gets a signed "Signature is invalid" response, letting a client tell a wrong app secret from an unreachable device.
- fix: WiFi power management is set to `CYW43_PERFORMANCE_PM` when local control starts. Under the cyw43 default the station sleeps through frames the AP buffers for the broadcast group, and the device was never discovered - measured 0/6 before the change and 6/6 after, on the same board and network.
- fix: `LWIP_IGMP` was not defined in `lwipopts.h`, so it defaulted off and `igmp_joingroup()` was compiled out. The multicast join would have silently done nothing.


Two notes for this port:

- `lwipopts.h` must define `LWIP_IGMP 1`. Without it the multicast group join is
  compiled out and does nothing, silently.
- WiFi power management is switched to `CYW43_PERFORMANCE_PM` when local
  control starts. The cyw43 default lets the station sleep through the
  frames an AP buffers for the broadcast group, which is exactly how the
  app discovers a device. The cost is idle current, not throughput.
- The lwIP receive callback only copies the datagram and its sender. Signature
  checking and device dispatch happen in `sinricpro_handle()`, on your own loop
  - doing that work in lwIP's context blocks the stack.

There is no mDNS announcement yet; the app discovers the device by broadcasting
a signed probe, which needs nothing extra here.

## [1.0.0]

- Initial release.
