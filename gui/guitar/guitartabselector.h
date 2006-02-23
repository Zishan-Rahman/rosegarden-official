// -*- c-basic-offset: 4 -*-

/*
    Rosegarden-4
    A sequencer and musical notation editor.
 
    This program is Copyright 2000-2005
        Guillaume Laurent   <glaurent@telegraph-road.org>,
        Chris Cannam        <cannam@all-day-breakfast.com>,
        Richard Bown        <bownie@bownie.com>
 
    The moral right of the authors to claim authorship of this work
    has been asserted.
 
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/
#ifndef GUITAR_TAB_SELECTOR_H_
#define GUITAR_TAB_SELECTOR_H_

#include "fingering.h"
#include "chordmap.h"
#include "fingers.h"

#include <kdialogbase.h>

#include <qtextedit.h>

namespace Rosegarden
{
    class Event;
}

class GuitarTabSelectorDialog : public KDialogBase
{
    Q_OBJECT

public:

    //! Constructor
    GuitarTabSelectorDialog(QWidget *parent);

    //! Destructor
    ~GuitarTabSelectorDialog();

    void init (void);

    guitar::Fingering getArrangement (void) const;

    void setArrangement (guitar::Fingering* chord);

signals:

    //! Display a given chord fingering
    void displayChord (guitar::Fingering*);

public slots:

    //! Begining displaying the given chord to the QPainter object
    virtual void slotDisplayChord ();

    //! Update Modifier list with interval values from the given scale
    virtual void slotSetupModifierList (int index);

    //! Update Suffix list with distance values from the given interval
    virtual void slotSetupSuffixList (int index);

    //! Update Version list with version values from the given distance
    virtual void slotSetupVersionList (int index);

    //! Save chord map to user chord directory
    virtual void slotSaveChords ();

    //! Code to call guitar chord editor with presently selected chord
    virtual void modifyChord();

    //! Code to call guitar chord editor to create a new chord
    virtual void createChord();

    //! Clear the display Chord
    virtual void clearChord();

private:

    //! Setup first list of scales
    void setupNameList ( void );

    //! Create editor components and display them via the QPainter object
    void populate (void);

    //! Add Chords from all chord list files to chord map
    void setupChordMap (void);

    //! Find all chord list files on the system
    std::vector<QString> getAvailableChordFiles (void);

    //! Read directory for guitar chord files.
    std::vector<QString> readDirectory (QString chordDir);

    //! List of Chords
    guitar::ChordMap* m_chordMap;

    //! Guitar object
    guitar::Guitar* m_guitar;

    //! Fingering constructor object
    guitar::FingeringConstructor* m_fingering;

    //! Presently selected arrangement
    guitar::Fingering* m_arrangement;

    // Chord data
    QListBox* m_scaleList;
    QListBox* m_modifierList;
    QListBox* m_suffixList;
    QListBox* m_versionList;

    QTextEdit* m_aliases;

    /** Setup control variable - Used to prevent the signal setupModifierList from
      being executed during startup. This fixes the interval list being filled
      on startup */
    bool m_setup;
};

#endif /* GUITAR_TAB_EDITOR_H_ */
