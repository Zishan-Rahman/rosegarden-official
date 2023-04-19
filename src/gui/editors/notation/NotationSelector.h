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

#ifndef RG_NOTATIONSELECTOR_H
#define RG_NOTATIONSELECTOR_H

#include "NotationTool.h"
#include "NotationMouseEvent.h"
#include "base/Event.h"

#include <QString>
#include <QPointF>


class QMouseEvent;
class QGraphicsRectItem;


namespace Rosegarden
{

class ViewElement;
class NotationWidget;
class NotationElement;
class EventSelection;
class Event;
class NotationStaff;

/**
 * Rectangular note selection
 */
class NotationSelector : public NotationTool
{
    Q_OBJECT

    friend class NotationToolBox;

public:

    ~NotationSelector() override;

    void handleLeftButtonPress(const NotationMouseEvent *) override;

    void handleRightButtonPress(const NotationMouseEvent *) override;

    FollowMode handleMouseMove(const NotationMouseEvent *) override;

    void handleMouseRelease(const NotationMouseEvent *) override;

    void handleMouseDoubleClick(const NotationMouseEvent *) override;

    virtual void handleMouseTripleClick(const NotationMouseEvent *);

    /**
     * Create the selection rect
     *
     * We need this because NotationScene deletes all scene items
     * along with it. This happens before the NotationSelector is
     * deleted, so we can't delete the selection rect in
     * ~NotationSelector because that leads to double deletion.
     */
    void ready() override;

    /**
     * Delete the selection rect.
     */
    void stow() override;

    /**
     * Respond to an event being deleted -- it may be the one the tool
     * is remembering as the current event.
     */
    virtual void handleEventRemoved(Event *event);

    /**
     * Useful to get the tool name from a NotationTool object
     */
    const QString getToolName() override { return ToolName(); }

    bool needsWheelEvents() override { return false; }

    static QString ToolName();

signals:
    void editElement(NotationStaff *, NotationElement *, bool advanced);

public slots:
    /**
     * Hide the selection rectangle
     *
     * Should be called after a cut or a copy has been
     * performed
     */
    void slotHideSelection();

    void slotInsertSelected();
    void slotEraseSelected();
//    void slotCollapseRests();
    void slotCollapseRestsHard();
    void slotRespellFlat();
    void slotRespellSharp();
    void slotRespellNatural();
    void slotCollapseNotes();
    void slotInterpret();
    void slotStaffAbove();
    void slotStaffBelow();
    void slotMakeInvisible();
    void slotMakeVisible();

    void slotClickTimeout();
    void slotMoveInsertionCursor();

protected:
    NotationSelector(NotationWidget *, bool ties = true);

    /**
     * Set the current selection on the parent NotationView
     */
    void setViewCurrentSelection(bool preview);

    /**
     * Look up the staff containing the given notation element
     */
//!!!    NotationStaff *getStaffForElement(NotationElement *elt);

    void drag(int x, int y, bool final);

    EventSelection *getEventsInSelectionRect();

    //--------------- Data members ---------------------------------

    QGraphicsRectItem *m_selectionRect;
    QPointF m_selectionOrigin;
    bool m_updateRect;

    NotationStaff *m_selectedStaff;
    NotationElement *m_clickedElement;
    bool m_clickedShift;
    bool m_startedFineDrag;

    EventSelection *m_selectionToMerge;

    long m_lastDragPitch;
    timeT m_lastDragTime;

    bool m_justSelectedBar;
    bool m_wholeStaffSelectionComplete;
    bool m_ties;

private:
    bool m_doubleClick;
    bool m_tripleClick;
    NotationStaff * m_pointerStaff;
    timeT m_pointerTime;
    // m_releaseTimer delays execution of mouse button release related code to *
    // allow the execution of a possible double click.
    QTimer *m_releaseTimer;
};

class NotationSelectorNoTies : public NotationSelector
{
    friend class NotationToolBox;
 public:
    static QString ToolName();

 private:
 explicit NotationSelectorNoTies(NotationWidget *widget) :
    NotationSelector(widget, false)
        {}

};


}

#endif
