/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*- vi:set ts=8 sts=4 sw=4: */

/*
    Rosegarden
    A sequencer and musical notation editor.
    Copyright 2000-2024 the Rosegarden development team.
    See the AUTHORS file for more details.

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#include "Device.h"

#include "base/Controllable.h"
#include "base/MidiDevice.h"
#include "base/SoftSynthDevice.h"
#include "misc/Debug.h"
#include "document/RosegardenDocument.h"
#include "base/Composition.h"


namespace Rosegarden
{


const DeviceId Device::NO_DEVICE = 10000;
const DeviceId Device::ALL_DEVICES = 10001;
// "external controller" port.
const DeviceId Device::EXTERNAL_CONTROLLER = 10002;


Device::~Device()
{
    //SEQUENCER_DEBUG << "~Device";
    InstrumentList::iterator it = m_instruments.begin();
    // For each Instrument
    for (; it != m_instruments.end(); ++it) {
        (*it)->sendWholeDeviceDestroyed();
        delete (*it);
    }
        
}

// Return a Controllable if we are a subtype that also inherits from
// Controllable, otherwise return nullptr
Controllable *
Device::getControllable()
{
    Controllable *c = dynamic_cast<MidiDevice *>(this);
    if (!c) {
        c = dynamic_cast<SoftSynthDevice *>(this);
    }
    // Even if it's zero, return it now.
    return c;
}

// Base case: Device itself doesn't know AllocateChannels so gives nullptr.
// @author Tom Breton (Tehom)
AllocateChannels *
Device::getAllocator()
{ return nullptr; }

void
Device::sendChannelSetups()
{
    // For each Instrument, send channel setup
    for (InstrumentList::iterator it = m_instruments.begin();
         it != m_instruments.end();
         ++it) {
        (*it)->sendChannelSetup();
    }
}

InstrumentId
Device::getAvailableInstrument(const Composition *composition) const
{
    InstrumentList instruments = getPresentationInstruments();
    if (instruments.empty())
        return NoInstrument;

    if (!composition)
        composition = &RosegardenDocument::currentDocument->getComposition();

    // Assume not found.
    InstrumentId firstInstrumentID{NoInstrument};

    // For each instrument on the device
    for (const Instrument *instrument : instruments) {
        if (!instrument)
            continue;

        const InstrumentId instrumentID = instrument->getId();

        // If we've not found the first one yet, save it in case we don't
        // find anything available.
        if (firstInstrumentID == NoInstrument)
            firstInstrumentID = instrumentID;

        // If this instrumentID is not in use, return it.
        if (!composition->hasTrack(instrumentID))
            return instrumentID;
    }

    // Return the first instrumentID for this device.
    return firstInstrumentID;
}


}
