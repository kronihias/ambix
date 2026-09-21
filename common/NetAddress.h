/*
 ==============================================================================

 This file is part of the ambix Ambisonic plug-in suite.
 Copyright (c) 2013/2014 - Matthias Kronlachner
 www.matthiaskronlachner.com

 Permission is granted to use this software under the terms of:
 the GPL v2 (or any later version)

 Details of these licenses can be found at: www.gnu.org/licenses

 ambix is distributed in the hope that it will be useful, but WITHOUT ANY
 WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

 ==============================================================================

 NetAddress — same-machine detection, so an encoder and a visualizer sitting
 in one machine talk over loopback instead of over whatever LAN address the
 discovery broadcast happened to carry.

 Ported from mcfx (common/net/mcfx_net_socket.h), where the same pair of
 helpers fixed the same problem for mcfx_send/mcfx_receive. Backed by JUCE's
 IPAddress::getAllAddresses() here rather than a raw getifaddrs() walk.

 ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

namespace ambix { namespace net {

// Snapshot of this machine's IPv4 addresses, refreshed at most every 2 s.
// getAllAddresses() walks the interface list, and the discovery timers ask
// once per advertised peer per tick (2 Hz on both sides), so the answer is
// cached. 2 s is well inside the time it takes a DHCP renewal or an
// interface flap to matter.
//
// Unlike DiscoveryHub's singleton, this static does not need to be shared
// across translation units — under -fvisibility=hidden each TU may well get
// its own copy, and the only cost is a few extra interface walks. Do not
// "fix" it by moving it out of line; there is no shared state to protect.
inline juce::StringArray getCachedLocalIPv4Addresses()
{
    static juce::CriticalSection cacheLock;
    static juce::StringArray cached;
    static juce::uint32 cachedAtMs = 0;
    static bool primed = false;

    const juce::ScopedLock sl (cacheLock);
    const auto nowMs = juce::Time::getMillisecondCounter();
    if (! primed || nowMs - cachedAtMs > 2000)
    {
        cached.clear();
        for (const auto& a : juce::IPAddress::getAllAddresses (false))
            cached.add (a.toString());
        cachedAtMs = nowMs;
        primed     = true;   // an empty result (no interface up) is a real
                             // answer, not a reason to re-query every call
    }
    return cached;
}

// True when `host` is a dotted-quad IPv4 literal naming an address that is
// configured on this machine — including the whole 127/8 loopback block.
//
// Literals only: resolving a hostname would mean a blocking getaddrinfo on
// the message thread, and every caller that matters (NSD service addresses,
// the visualizer's own advertised IP, subscriber addresses) hands us a
// dotted quad anyway.
inline bool isOwnIPv4Address (const juce::String& host)
{
    if (host.isEmpty()) return false;
    if (host.startsWith ("127.")) return true;
    if (! host.containsOnly ("0123456789.")) return false;
    return getCachedLocalIPv4Addresses().contains (host);
}

// Rewrite a same-machine destination to 127.0.0.1.
//
// Sending to your own LAN address never puts a packet on the wire in the
// first place: the IP stack sees a locally-configured destination and routes
// it over the loopback interface. So this changes nothing about where the
// OSC traffic travels.
//
// What it removes is the *dependency* on that address staying valid. The
// address a peer latches onto comes from the NSD advertisement, and our
// JUCE_patches/juce_network_discovery.patch deliberately stamps the
// loopback broadcast with the first real interface address rather than
// 127.0.0.1 — otherwise the `address` attribute would flap between ticks.
// That keeps discovery stable but leaves same-machine peers addressing each
// other by a Wi-Fi address that roaming, a DHCP renewal or a dropped link
// can invalidate under them. 127.0.0.1 never goes stale, and loopback
// traffic is also exempt from the macOS local-network privacy gate.
//
// Idempotent: a loopback address in gives the same address back.
inline juce::String preferLoopbackIfLocal (const juce::String& host)
{
    return isOwnIPv4Address (host) ? juce::String ("127.0.0.1") : host;
}

inline juce::IPAddress preferLoopbackIfLocal (const juce::IPAddress& host)
{
    return isOwnIPv4Address (host.toString()) ? juce::IPAddress (127, 0, 0, 1)
                                              : host;
}

}} // namespace ambix::net
