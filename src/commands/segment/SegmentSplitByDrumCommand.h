
/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*- vi:set ts=8 sts=4 sw=4: */

/*
    Rosegarden
    A MIDI and audio sequencer and musical notation editor.
    Copyright 2000-2016 the Rosegarden development team.

    Other copyrights also apply to some parts of this work.  Please
    see the AUTHORS file and individual file headers for details.

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#ifndef RG_SEGMENTSPLITBYDRUMCOMMAND_H
#define RG_SEGMENTSPLITBYDRUMCOMMAND_H

#include "base/Segment.h"
#include "document/Command.h"
#include "gui/general/ClefIndex.h"

#include <QString>
#include <QCoreApplication>


namespace Rosegarden
{


class Composition;


class SegmentSplitByDrumCommand : public NamedCommand
{
    Q_DECLARE_TR_FUNCTIONS(Rosegarden::SegmentSplitByDrumCommand)

public:
    
    SegmentSplitByDrumCommand(Segment *segment);
    virtual ~SegmentSplitByDrumCommand();

    static QString getGlobalName()
        { return tr("Split by &Drum..."); }

    virtual void execute();
    virtual void unexecute();

private:
    Composition *m_composition;
    Segment *m_segment;
    Segment *m_newSegment;
    bool m_executed;
};


}

#endif