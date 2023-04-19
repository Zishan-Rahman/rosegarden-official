/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*- vi:set ts=8 sts=4 sw=4: */

/*
    Rosegarden
    A MIDI and audio sequencer and musical notation editor.
    Copyright 2000-2023 the Rosegarden development team.
 
    Other copyrights also apply to some parts of this work.  Please
    see the AUTHORS file and individual file headers for details.
 
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/


/*
 * MarkParallelCommand.h
 *
 *  Created on: Mar 25, 2015
 *      Author: lambache
 */

#ifndef MARKPARALLELCOMMAND_H
#define MARKPARALLELCOMMAND_H

#include "document/BasicCommand.h"

#include <QCoreApplication>


namespace Rosegarden
{


/// ??? This "command" does nothing and is used only to trigger a repaint.  Get rid of it.
class MarkParallelCommand : public BasicCommand
{
    Q_DECLARE_TR_FUNCTIONS(Rosegarden::MarkParallelCommand)

public:
    MarkParallelCommand(Segment &segment, timeT begin, timeT end) :
        BasicCommand(tr("Mark Parallel"), segment, begin, end)
    {
    }

protected:
    void modifySegment() override  { }

};


}

#endif
