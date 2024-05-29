
/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*- vi:set ts=8 sts=4 sw=4: */

/*
    Rosegarden
    A MIDI and audio sequencer and musical notation editor.
    Copyright 2000-2024 the Rosegarden development team.

    Other copyrights also apply to some parts of this work.  Please
    see the AUTHORS file and individual file headers for details.

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#ifndef RG_RETROGRADEINVERTCOMMAND_H
#define RG_RETROGRADEINVERTCOMMAND_H

#include "document/BasicCommand.h"

#include <QCoreApplication>


namespace Rosegarden
{


class EventSelection;


class RetrogradeInvertCommand : public BasicCommand
{
    Q_DECLARE_TR_FUNCTIONS(Rosegarden::RetrogradeInvertCommand)

public:
    RetrogradeInvertCommand(int semitones, EventSelection &selection) :
        BasicCommand(tr("Re&trograde Invert"), selection, true),
        m_selection(&selection),
        m_semitones(semitones)
    { }

protected:
    void modifySegment() override;

private:
    EventSelection *m_selection;// only used on 1st execute (cf bruteForceRedo)
    int m_semitones;
};


}

#endif
