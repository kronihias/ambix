/*
 ==============================================================================

 This file is part of the ambix Ambisonic plug-in suite.
 Copyright (c) 2013/2014 - Matthias Kronlachner
 www.matthiaskronlachner.com

 Permission is granted to use this software under the terms of:
 the GPL v2 (or any later version)

 ==============================================================================
*/

#include "DiscoveryHub.h"

namespace ambix { namespace net {

DiscoveryHub& DiscoveryHub::instance()
{
    // Single, process-wide singleton. Defined here (not inline in the
    // header) so all translation units that include the header reach the
    // *same* storage even when JUCE compiles plugins with
    // -fvisibility=hidden — defining a function-local static in an inline
    // header function gives each TU its own copy and the shared-listener
    // trick would silently fail.
    static DiscoveryHub h;
    return h;
}

}} // namespace ambix::net
