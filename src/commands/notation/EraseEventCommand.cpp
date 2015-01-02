/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*- vi:set ts=8 sts=4 sw=4: */

/*
    Rosegarden
    A MIDI and audio sequencer and musical notation editor.
    Copyright 2000-2015 the Rosegarden development team.
 
    Other copyrights also apply to some parts of this work.  Please
    see the AUTHORS file and individual file headers for details.
 
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/


#include "EraseEventCommand.h"

#include "misc/Strings.h"
#include "base/Event.h"
#include "base/NotationTypes.h"
#include "base/Segment.h"
#include "base/SegmentNotationHelper.h"
#include "document/BasicCommand.h"
#include "gui/editors/notation/NotationProperties.h"


namespace Rosegarden
{

EraseEventCommand::EraseEventCommand(Segment &segment,
                                     Event *event,
                                     bool collapseRest) :
        BasicCommand(strtoqstr(makeName(event->getType())),
                     segment,
                     event->getAbsoluteTime(),
                     event->getAbsoluteTime() + event->getDuration(),
                     true),
        m_collapseRest(collapseRest),
        m_event(event),
        m_relayoutEndTime(getEndTime())
{
    // nothing
}

EraseEventCommand::~EraseEventCommand()
{
    // nothing
}

std::string
EraseEventCommand::makeName(std::string e)
{
    std::string n = "Erase ";
    n += (char)toupper(e[0]);
    n += e.substr(1);
    return n;
}

timeT
EraseEventCommand::getRelayoutEndTime()
{
    return m_relayoutEndTime;
}

void
EraseEventCommand::modifySegment()
{
    SegmentNotationHelper helper(getSegment());

    if (m_event->isa(Clef::EventType) ||
            m_event->isa(Key ::EventType)) {

        m_relayoutEndTime = helper.segment().getEndTime();

    } else if (m_event->isa(Indication::EventType)) {

        try {
            Indication indication(*m_event);
            if (indication.isOttavaType()) {

                for (Segment::iterator i = getSegment().findTime
                                           (m_event->getAbsoluteTime());
                        i != getSegment().findTime
                        (m_event->getAbsoluteTime() + indication.getIndicationDuration());
                        ++i) {
                    (*i)->unset(NotationProperties::OTTAVA_SHIFT);
                }
            }
        } catch (...) {}
    }

    helper.deleteEvent(m_event, m_collapseRest);
}

}
