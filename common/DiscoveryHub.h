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

 DiscoveryHub — process-wide singleton that owns ONE
 juce::NetworkServiceDiscovery::AvailableServiceList per (service-UID,
 broadcast-port) pair, and fans out onChange notifications to every
 subscriber for that pair.

 Why this exists: AvailableServiceList listens on a fixed UDP port. When
 two plugin instances live in the same process and each create their own
 listener on the same port, both succeed in binding (JUCE sets
 SO_REUSEADDR), but on macOS the kernel only delivers each incoming
 broadcast packet to ONE of them. Symptom: only one plugin instance ever
 sees discovered peers — every other instance flickers between
 "subscriber present" and "subscriber gone".

 The hub takes the listener out of every plugin instance and hosts it
 once per process. Each plugin (in browse mode) registers a callback via
 subscribe() and reads the snapshot via getServices(). Reference-counted:
 the underlying AvailableServiceList is created on first subscribe and
 destroyed when the last subscriber goes away.

 Lifted from mcfx (which lifted it from sonolink) — same author, same
 license, same exact macOS bug.

 ==============================================================================
*/

#pragma once

#include "JuceHeader.h"

#include <algorithm>
#include <functional>
#include <map>
#include <vector>

namespace ambix { namespace net {

class DiscoveryHub
{
public:
    // Out-of-line definition (in DiscoveryHub.cpp) so that under JUCE's
    // default -fvisibility=hidden every translation unit reaches the SAME
    // singleton instance — defining it inline in a header would give each
    // TU its own copy and the shared-listener trick would silently fail.
    static DiscoveryHub& instance();

    using ChangeCallback = std::function<void()>;

    // RAII handle returned by subscribe(). Destroying it unregisters and,
    // if the refcount drops to 0, tears down the underlying NSD listener.
    class Subscription
    {
    public:
        Subscription() = default;
        explicit Subscription (int id) : id_ (id) {}
        ~Subscription() { reset(); }

        Subscription (Subscription&& o) noexcept : id_ (o.id_) { o.id_ = 0; }
        Subscription& operator= (Subscription&& o) noexcept
        {
            if (this != &o) { reset(); id_ = o.id_; o.id_ = 0; }
            return *this;
        }
        Subscription (const Subscription&) = delete;
        Subscription& operator= (const Subscription&) = delete;

        void reset()
        {
            if (id_ != 0)
            {
                DiscoveryHub::instance().unsubscribe (id_);
                id_ = 0;
            }
        }

    private:
        int id_ = 0;
    };

    Subscription subscribe (const juce::String& serviceUid, int broadcastPort,
                            ChangeCallback cb)
    {
        const juce::ScopedLock sl (lock_);

        auto& entry = lists_[Key { serviceUid, broadcastPort }];
        if (entry.refCount == 0)
        {
            entry.list = std::make_unique<juce::NetworkServiceDiscovery::AvailableServiceList> (
                serviceUid, broadcastPort);
            const auto uidCopy  = serviceUid;
            const auto portCopy = broadcastPort;
            entry.list->onChange = [this, uidCopy, portCopy]() {
                fireChange (Key { uidCopy, portCopy });
            };
        }
        ++entry.refCount;

        const int id = ++nextId_;
        subs_.push_back ({ id, serviceUid, broadcastPort, std::move (cb) });
        return Subscription { id };
    }

    std::vector<juce::NetworkServiceDiscovery::Service>
    getServices (const juce::String& serviceUid, int broadcastPort) const
    {
        const juce::ScopedLock sl (lock_);
        auto it = lists_.find (Key { serviceUid, broadcastPort });
        if (it == lists_.end() || ! it->second.list) return {};
        return it->second.list->getServices();
    }

    void refresh()
    {
        std::vector<Sub> snapshotSubs;
        {
            const juce::ScopedLock sl (lock_);
            for (auto& [key, entry] : lists_)
            {
                if (entry.list) entry.list->onChange = nullptr;
                entry.list = std::make_unique<juce::NetworkServiceDiscovery::AvailableServiceList> (
                    key.uid, key.port);
                const auto k = key;
                entry.list->onChange = [this, k]() { fireChange (k); };
            }
            snapshotSubs = subs_;
        }
        for (auto& s : snapshotSubs)
            if (s.cb) juce::MessageManager::callAsync (s.cb);
    }

    void unsubscribe (int id)
    {
        const juce::ScopedLock sl (lock_);
        auto it = std::find_if (subs_.begin(), subs_.end(),
                                [id] (const Sub& s) { return s.id == id; });
        if (it == subs_.end()) return;

        const auto uid  = it->uid;
        const auto port = it->port;
        subs_.erase (it);

        auto entryIt = lists_.find (Key { uid, port });
        if (entryIt == lists_.end()) return;
        if (--entryIt->second.refCount <= 0)
        {
            if (entryIt->second.list) entryIt->second.list->onChange = nullptr;
            entryIt->second.list.reset();
            lists_.erase (entryIt);
        }
    }

private:
    DiscoveryHub() = default;
    ~DiscoveryHub()
    {
        const juce::ScopedLock sl (lock_);
        for (auto& [key, entry] : lists_)
        {
            if (entry.list) entry.list->onChange = nullptr;
            entry.list.reset();
        }
        lists_.clear();
        subs_.clear();
    }

    struct Key
    {
        juce::String uid;
        int          port = 0;
        bool operator< (const Key& o) const noexcept
        {
            if (port != o.port) return port < o.port;
            return uid.compare (o.uid) < 0;
        }
    };

    struct ListEntry
    {
        std::unique_ptr<juce::NetworkServiceDiscovery::AvailableServiceList> list;
        int refCount = 0;
    };

    struct Sub
    {
        int            id = 0;
        juce::String   uid;
        int            port = 0;
        ChangeCallback cb;
    };

    void fireChange (const Key& key)
    {
        std::vector<ChangeCallback> toFire;
        {
            const juce::ScopedLock sl (lock_);
            for (auto& s : subs_)
                if (s.uid == key.uid && s.port == key.port && s.cb)
                    toFire.push_back (s.cb);
        }
        // onChange runs on the NSD thread; bounce to the message thread so
        // subscribers can update UI directly.
        for (auto& cb : toFire)
            juce::MessageManager::callAsync (cb);
    }

    mutable juce::CriticalSection lock_;
    std::map<Key, ListEntry>      lists_;
    std::vector<Sub>              subs_;
    int                           nextId_ = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DiscoveryHub)
};

}} // namespace ambix::net
