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

#ifndef ROSEGARDENGUIVIEW_H
#define ROSEGARDENGUIVIEW_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif 

// include files for Qt

#include <qvbox.h>
#include <list>
#include <vector>

#include "rosedebug.h"

#include "AudioFile.h"
#include "MappedCommon.h"
#include "Track.h"
#include "Event.h" // for time
#include "SequencerDataBlock.h" // for LevelInfo

namespace Rosegarden { 
    class Composition; 
    class MappedEvent;
    class SimpleRulerScale;
    class SegmentSelection;
}

class QVBoxLayout;
class QScrollView;
class RosegardenGUIDoc;
class TrackEditor;
class KPrinter;
class KDockWidget;
class SegmentParameterBox;
class InstrumentParameterBox;
class MultiViewCommandHistory;
class KCommand;
class NotationView;
class MatrixView;
class EventView;
class SequencerMapper;

/**
 * The RosegardenGUIView class provides the view widget for the
 * RosegardenGUIApp instance.  The View instance inherits QWidget as a
 * base class and represents the view object of a KTMainWindow. As
 * RosegardenGUIView is part of the docuement-view model, it needs a
 * reference to the document object connected with it by the
 * RosegardenGUIApp class to manipulate and display the document
 * structure provided by the RosegardenGUIDoc class.
 * 	
 * @author Source Framework Automatically Generated by KDevelop, (c) The KDevelop Team.
 * @version KDevelop version 0.4 code generation
 */
class RosegardenGUIView : public QVBox
{
    Q_OBJECT
public:

    /**p
     * Constructor for the main view
     */
    RosegardenGUIView(bool showTrackLabels,
                      SegmentParameterBox*,
                      InstrumentParameterBox*,
                      QWidget *parent = 0,
                      const char *name=0);

    /**
     * Destructor for the main view
     */
    ~RosegardenGUIView();

    /**
     * returns a pointer to the document connected to the view
     * instance. Mind that this method requires a RosegardenGUIApp
     * instance as a parent widget to get to the window document
     * pointer by calling the RosegardenGUIApp::getDocument() method.
     *
     * @see RosegardenGUIApp#getDocument
     */
    RosegardenGUIDoc* getDocument() const;

    /**
     * Command history
     */
    MultiViewCommandHistory* getCommandHistory();

    TrackEditor* getTrackEditor() { return m_trackEditor; }
    
    /**
     * contains the implementation for printing functionality
     */
    void print(Rosegarden::Composition*, bool previewOnly = false);

    // the following aren't slots because they're called from
    // RosegardenGUIApp

    /**
     * Select a tool at the SegmentCanvas
     */
    void selectTool(QString toolName);

    /**
     * These two are just-passing-through methods called from
     * the GUI when it has key presses that the SegmentCanvas
     * or anything else downstairsis interested in.
     *
     */
    void setShift(bool value);
    void setControl(bool value);

    /**
     * Show a Segment as it records - remove the SegmentItem
     * when no longer needed
     */
    void showRecordingSegmentItem(Rosegarden::Segment* segment);
    void deleteRecordingSegmentItem();

    /**
     * Show output levels
     */
    void showVisuals(const Rosegarden::MappedEvent *mE);

    void updateMeters(SequencerMapper *mapper);

    /**
     * Change zoom size -- set the RulerScale's units-per-pixel to size
     */
    void setZoomSize(double size);

    void initChordNameRuler();
    
    bool haveSelection();
    Rosegarden::SegmentSelection getSelection();
    void updateSelectionContents();

public slots:
    void slotEditSegment(Rosegarden::Segment*);
    void slotEditSegmentNotation(Rosegarden::Segment*);
    void slotEditSegmentsNotation(std::vector<Rosegarden::Segment*>);
    void slotEditSegmentMatrix(Rosegarden::Segment*);
    void slotEditSegmentsMatrix(std::vector<Rosegarden::Segment*>);
    void slotEditSegmentEventList(Rosegarden::Segment*);
    void slotEditSegmentsEventList(std::vector<Rosegarden::Segment*>);
    void slotEditTriggerSegment(int);
    void slotEditSegmentAudio(Rosegarden::Segment*);
    void slotSegmentAutoSplit(Rosegarden::Segment*);
    void slotEditRepeat(Rosegarden::Segment*, Rosegarden::timeT);
    void slotEditTempos(Rosegarden::timeT);
    void slotEditMetadata(QString);

    /**
     * Highlight all the Segments on a Track because the Track has
     * been selected * We have to ensure we create a Selector object
     * before we can highlight * these tracks.
     *
     * Called by signal from Track selection routine to highlight
     * all available Segments on a Track
     */
    void slotSelectTrackSegments(int);

    void slotSelectAllSegments();

    void slotUpdateInstrumentParameterBox(int id);

    // This is called from the canvas (actually the selector tool) moving out
    //
    void slotSelectedSegments(const Rosegarden::SegmentSelection &segments);

    // And this one from the user interface going down
    //
    void slotSetSelectedSegments(const Rosegarden::SegmentSelection &segments);

    void slotShowRulers(bool);

    void slotShowTempoRuler(bool);

    void slotShowChordNameRuler(bool);

    void slotShowPreviews(bool);

    /**
     * Update previews for a particular instrument, if audio
     */
    void slotUpdateAudioPreviews(Rosegarden::InstrumentId id);

    void slotAddTracks(unsigned int, Rosegarden::InstrumentId);

    void slotDeleteTracks(std::vector<Rosegarden::TrackId> tracks);

    void slotAddAudioSegmentCurrentPosition(Rosegarden::AudioFileId,
                                            const Rosegarden::RealTime &startTime,
                                            const Rosegarden::RealTime &endTime);

    void slotAddAudioSegmentDefaultPosition(Rosegarden::AudioFileId,
                                            const Rosegarden::RealTime &startTime,
                                            const Rosegarden::RealTime &endTime);

    void slotAddAudioSegment(Rosegarden::AudioFileId audioId,
                             Rosegarden::TrackId trackId,
                             Rosegarden::timeT position,
                             const Rosegarden::RealTime &startTime,
                             const Rosegarden::RealTime &endTime);

    void slotDroppedAudio(QString audioDesc);
    void slotDroppedNewAudio(QString audioDesc);

    /*
     * Commands
     *
     */
    void slotAddCommandToHistory(KCommand *command);

    /*
     * Change the Instrument Label
     */
    void slotChangeInstrumentLabel(Rosegarden::InstrumentId id, QString label);

    /*
     * Change the Track Label
     */
    void slotChangeTrackLabel(Rosegarden::TrackId id, QString label);

    /*
     * Set the mute button on the track buttons and on the instrument
     * parameter box
     */
    void slotSetMuteButton(Rosegarden::TrackId track, bool value);

    /*
     * Set mute, record and solo by instrument id
     */
    void slotSetMute(Rosegarden::InstrumentId, bool);
    void slotSetRecord(Rosegarden::InstrumentId, bool);
    void slotSetSolo(Rosegarden::InstrumentId, bool);

    /**
     * To indicate that we should track the recording segment (despite
     * no commands being issued on it)
     */
    void slotUpdateRecordingSegment(Rosegarden::Segment *segment,
				    Rosegarden::timeT updatedFrom);

    /*
     * A manual fudgy way of creating a view update for certain
     * semi-static data (devices/instrument labels mainly)
     */
    void slotSynchroniseWithComposition();

signals:
    void activateTool(QString toolName);

    void stateChange(QString, bool);

    // Inform that we've got a SegmentSelection
    //
    void segmentsSelected(const Rosegarden::SegmentSelection&);

    void toggleSolo(bool);

    /**
     * Current used to dispatch things like track select changes, solo, etc...
     * to edit views
     */
    void compositionStateUpdate();
    

    /**
     * This signal is used to dispatch a notification for a request to
     * set the step-by-step-editing target window to all candidate targets,
     * so that they can either know that their request has been granted
     * (if they match the QObject passed) or else deactivate any step-by-
     * step editing currently active in their own window (otherwise).
     */
    void stepByStepTargetRequested(QObject *);

    /*
     * Add an audio file at the sequencer - when we drop a new file
     * on the segment canvas.
     */
    void addAudioFile(Rosegarden::AudioFileId);

    void checkTrackAssignments();

    void instrumentLevelsChanged(Rosegarden::InstrumentId,
				 const Rosegarden::LevelInfo &);

protected:

    NotationView *createNotationView(std::vector<Rosegarden::Segment *>);
    MatrixView   *createMatrixView  (std::vector<Rosegarden::Segment *>);
    EventView    *createEventView   (std::vector<Rosegarden::Segment *>);

    //--------------- Data members ---------------------------------

    Rosegarden::SimpleRulerScale  *m_rulerScale;
    TrackEditor			  *m_trackEditor;

    SegmentParameterBox		  *m_segmentParameterBox;
    InstrumentParameterBox	  *m_instrumentParameterBox;

};

#endif // ROSEGARDENGUIVIEW_H
