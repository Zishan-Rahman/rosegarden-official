/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*- vi:set ts=8 sts=4 sw=4: */

/*
    Rosegarden
    A MIDI and audio sequencer and musical notation editor.
 
    This program is Copyright 2000-2007
        Guillaume Laurent   <glaurent@telegraph-road.org>,
        Chris Cannam        <cannam@all-day-breakfast.com>,
        Richard Bown        <richard.bown@ferventsoftware.com>
 
    This file is Copyright 2006
	D. Michael McIntyre <dmmcintyr@users.sourceforge.net>

    The moral rights of Guillaume Laurent, Chris Cannam, and Richard
    Bown to claim authorship of this work have been asserted.
 
    Other copyrights also apply to some parts of this work.  Please
    see the AUTHORS file and individual file headers for details.
 
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/


#include "PresetElement.h"

#include "misc/Debug.h"
#include <qstring.h>


namespace Rosegarden
{

PresetElement::PresetElement(QString name,
                             int clef,
                             int transpose,
                             int highAm,
                             int lowAm,
                             int highPro,
                             int lowPro) :
        m_name (name),
        m_clef (clef),
        m_transpose (transpose),
        m_highAm (highAm),
        m_lowAm (lowAm),
        m_highPro (highPro),
        m_lowPro (lowPro)
{
    RG_DEBUG << "PresetElement::PresetElement(" << endl
    << "    name = " << name << endl
    << "    clef = " << clef << endl
    << "    trns.= " << transpose << endl
    << "    higH = " << highAm << endl
    << "    lowA = " << lowAm << endl
    << "    higP = " << highPro << endl
    << "    lowP = " << lowPro << ")" << endl;
}

PresetElement::~PresetElement()
{
    // nothing to do
}

}
