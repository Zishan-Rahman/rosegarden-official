/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*- vi:set ts=8 sts=4 sw=4: */

/*
    Rosegarden
    A MIDI and audio sequencer and musical notation editor.
    Copyright 2000-2016 the Rosegarden development team.
 
    This file is Copyright 2006
        Pedro Lopez-Cabanillas <plcl@users.sourceforge.net>
        D. Michael McIntyre <dmmcintyr@users.sourceforge.net>

    Other copyrights also apply to some parts of this work.  Please
    see the AUTHORS file and individual file headers for details.
 
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#define RG_MODULE_STRING "[TrackParameterBox]"

#include "TrackParameterBox.h"

#include "base/AudioPluginInstance.h"
#include "gui/general/ClefIndex.h"  // Clef enum
#include "gui/widgets/CollapsingFrame.h"
#include "base/Colour.h"
#include "base/ColourMap.h"
#include "base/Composition.h"
#include "misc/ConfigGroups.h"
#include "misc/Debug.h"
#include "base/Device.h"
#include "base/Exception.h"
#include "gui/general/GUIPalette.h"
#include "gui/widgets/InputDialog.h"
#include "base/Instrument.h"
#include "base/InstrumentStaticSignals.h"
#include "gui/widgets/LineEdit.h"
#include "base/MidiDevice.h"
#include "gui/dialogs/PitchPickerDialog.h"
#include "sound/PluginIdentifier.h"
#include "gui/general/PresetHandlerDialog.h"
#include "document/RosegardenDocument.h"
#include "RosegardenParameterBox.h"
#include "commands/segment/SegmentSyncCommand.h"
#include "gui/widgets/SqueezedLabel.h"
#include "base/StaffExportTypes.h"  // StaffTypes, Brackets
#include "base/Studio.h"
#include "base/Track.h"

#include <QCheckBox>
#include <QColor>
#include <QColorDialog>
#include <QComboBox>
#include <QDialog>
#include <QFontMetrics>
#include <QFrame>
#include <QGridLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPixmap>
#include <QPushButton>
#include <QSettings>
#include <QString>
#include <QWidget>


namespace Rosegarden
{


TrackParameterBox::TrackParameterBox(RosegardenDocument *doc,
                                     QWidget *parent) :
    RosegardenParameterBox(tr("Track"), tr("Track Parameters"), parent),
    m_doc(doc),
    m_selectedTrackId((int)NO_TRACK),
    m_lastInstrumentType(Instrument::InvalidInstrument),
    m_lowestPlayable(0),
    m_highestPlayable(127)
{
    setObjectName("Track Parameter Box");

    QFontMetrics metrics(m_font);
    const int width11 = metrics.width("12345678901");
    const int width20 = metrics.width("12345678901234567890");
    const int width22 = metrics.width("1234567890123456789012");
    const int width25 = metrics.width("1234567890123456789012345");

    // Set up default expansions for the collapsing elements.  These
    // CollapsingFrame objects keep track of their own settings per object name
    // internally, however, they do not provide a way to specify an initial
    // default setting, so we have to do that here.
    // See the "name" parameter to CollapsingFrame's ctor.
    // ??? CollapsingFrame should provide a way to pass a default so this
    //     code can be moved into its ctor.
    QSettings settings;
    settings.beginGroup(CollapsingFrameConfigGroup);

    // ??? Move these down with their widgets.  Then move into
    //     CollapsingFrame's ctor.
    const QString trackParametersPlayback = "trackparametersplayback";
    bool expanded = qStrToBool(settings.value(trackParametersPlayback, "true")) ;
    settings.setValue(trackParametersPlayback, expanded);

    const QString trackParametersRecord = "trackparametersrecord";
    expanded = qStrToBool(settings.value(trackParametersRecord, "false")) ;
    settings.setValue(trackParametersRecord, expanded);

    const QString trackParametersDefaults = "trackparametersdefaults";
    expanded = qStrToBool(settings.value(trackParametersDefaults, "false")) ;
    settings.setValue(trackParametersDefaults, expanded);

    const QString trackStaffGroup = "trackstaffgroup";
    expanded = qStrToBool(settings.value(trackStaffGroup, "false")) ;
    settings.setValue(trackStaffGroup, expanded);

    settings.endGroup();

    // Widgets

    // Label
    m_trackLabel = new SqueezedLabel(tr("<untitled>"), this);
    m_trackLabel->setAlignment(Qt::AlignCenter);
    m_trackLabel->setFont(m_font);

    // Playback parameters

    // Outer collapsing frame
    CollapsingFrame *playbackParametersFrame = new CollapsingFrame(
            tr("Playback parameters"), this, trackParametersPlayback);

    // Inner fixed widget
    // We need an inner widget so that we can have a layout.  The outer
    // CollapsingFrame already has its own layout.
    QWidget *playbackParameters = new QWidget(playbackParametersFrame);
    playbackParametersFrame->setWidget(playbackParameters);
    playbackParameters->setContentsMargins(3, 3, 3, 3);

    // Device
    QLabel *playbackDeviceLabel = new QLabel(tr("Device"), playbackParameters);
    playbackDeviceLabel->setFont(m_font);
    m_playbackDevice = new QComboBox(playbackParameters);
    m_playbackDevice->setToolTip(tr("<qt><p>Choose the device this track will use for playback.</p><p>Click <img src=\":pixmaps/toolbar/manage-midi-devices.xpm\"> to connect this device to a useful output if you do not hear sound</p></qt>"));
    m_playbackDevice->setMinimumWidth(width25);
    m_playbackDevice->setFont(m_font);
    connect(m_playbackDevice, SIGNAL(activated(int)),
            this, SLOT(slotPlaybackDeviceChanged(int)));

    // Instrument
    QLabel *instrumentLabel = new QLabel(tr("Instrument"), playbackParameters);
    instrumentLabel->setFont(m_font);
    m_instrument = new QComboBox(playbackParameters);
    m_instrument->setFont(m_font);
    m_instrument->setToolTip(tr("<qt><p>Choose the instrument this track will use for playback. (Configure the instrument in <b>Instrument Parameters</b>).</p></qt>"));
    m_instrument->setMaxVisibleItems(16);
    m_instrument->setMinimumWidth(width22);
    connect(m_instrument, SIGNAL(activated(int)),
            this, SLOT(slotInstrumentChanged(int)));

    // Archive
    QLabel *archiveLabel = new QLabel(tr("Archive"), playbackParameters);
    archiveLabel->setFont(m_font);
    m_archive = new QCheckBox(playbackParameters);
    m_archive->setFont(m_font);
    m_archive->setToolTip(tr("<qt><p>Check this to archive a track.  Archived tracks will not make sound.</p></qt>"));
    connect(m_archive, SIGNAL(clicked(bool)),
            this, SLOT(slotArchiveChanged(bool)));

    // Playback parameters layout

    // This automagically becomes playbackParameters's layout.
    QGridLayout *groupLayout = new QGridLayout(playbackParameters);
    groupLayout->setContentsMargins(5,0,0,5);
    groupLayout->setVerticalSpacing(2);
    groupLayout->setHorizontalSpacing(5);
    // Row 0: Device
    groupLayout->addWidget(playbackDeviceLabel, 0, 0);
    groupLayout->addWidget(m_playbackDevice, 0, 1);
    // Row 1: Instrument
    groupLayout->addWidget(instrumentLabel, 1, 0);
    groupLayout->addWidget(m_instrument, 1, 1);
    // Row 2: Archive
    groupLayout->addWidget(archiveLabel, 2, 0);
    groupLayout->addWidget(m_archive, 2, 1);
    // Let column 1 fill the rest of the space.
    groupLayout->setColumnStretch(1, 1);

    // Recording filters

    CollapsingFrame *recordingFiltersFrame = new CollapsingFrame(
            tr("Recording filters"), this, trackParametersRecord);

    QWidget *recordingFilters = new QWidget(recordingFiltersFrame);
    recordingFiltersFrame->setWidget(recordingFilters);
    recordingFilters->setContentsMargins(3, 3, 3, 3);

    // Device
    QLabel *recordDeviceLabel = new QLabel(tr("Device"), recordingFilters);
    recordDeviceLabel->setFont(m_font);
    m_recordingDevice = new QComboBox(recordingFilters);
    m_recordingDevice->setFont(m_font);
    m_recordingDevice->setToolTip(tr("<qt><p>This track will only record Audio/MIDI from the selected device, filtering anything else out</p></qt>"));
    m_recordingDevice->setMinimumWidth(width25);
    connect(m_recordingDevice, SIGNAL(activated(int)),
            this, SLOT(slotRecordingDeviceChanged(int)));

    // Channel
    QLabel *channelLabel = new QLabel(tr("Channel"), recordingFilters);
    channelLabel->setFont(m_font);
    m_recordingChannel = new QComboBox(recordingFilters);
    m_recordingChannel->setFont(m_font);
    m_recordingChannel->setToolTip(tr("<qt><p>This track will only record Audio/MIDI from the selected channel, filtering anything else out</p></qt>"));
    m_recordingChannel->setMaxVisibleItems(17);
    m_recordingChannel->setMinimumWidth(width11);
    connect(m_recordingChannel, SIGNAL(activated(int)),
            this, SLOT(slotRecordingChannelChanged(int)));

    // Thru Routing
    QLabel *thruLabel = new QLabel(tr("Thru Routing"), recordingFilters);
    thruLabel->setFont(m_font);
    m_thruRouting = new QComboBox(recordingFilters);
    m_thruRouting->setFont(m_font);
    //m_thruRouting->setToolTip(tr("<qt><p>Routing from the input device and channel to the instrument.</p></qt>"));
    m_thruRouting->setMinimumWidth(width11);
    m_thruRouting->addItem(tr("Auto"), Track::Auto);
    m_thruRouting->addItem(tr("On"), Track::On);
    m_thruRouting->addItem(tr("Off"), Track::Off);
    m_thruRouting->addItem(tr("When Armed"), Track::WhenArmed);
    connect(m_thruRouting, SIGNAL(activated(int)),
            this, SLOT(slotThruRoutingChanged(int)));

    // Recording filters layout

    groupLayout = new QGridLayout(recordingFilters);
    groupLayout->setContentsMargins(5,0,0,5);
    groupLayout->setVerticalSpacing(2);
    groupLayout->setHorizontalSpacing(5);
    // Row 0: Device
    groupLayout->addWidget(recordDeviceLabel, 0, 0);
    groupLayout->addWidget(m_recordingDevice, 0, 1);
    // Row 1: Channel
    groupLayout->addWidget(channelLabel, 1, 0);
    groupLayout->addWidget(m_recordingChannel, 1, 1);
    // Row 2: Thru Routing
    groupLayout->addWidget(thruLabel, 2, 0);
    groupLayout->addWidget(m_thruRouting, 2, 1);
    // Let column 1 fill the rest of the space.
    groupLayout->setColumnStretch(1, 1);

    // Staff export options

    CollapsingFrame *staffExportOptionsFrame = new CollapsingFrame(
            tr("Staff export options"), this, trackStaffGroup);

    QWidget *staffExportOptions = new QWidget(staffExportOptionsFrame);
    staffExportOptionsFrame->setWidget(staffExportOptions);
    staffExportOptions->setContentsMargins(2, 2, 2, 2);

    // Notation size (export only)
    //
    // NOTE: This is the only way to get a \small or \tiny inserted before the
    // first note in LilyPond export.  Setting the actual staff size on a
    // per-staff (rather than per-score) basis is something the author of the
    // LilyPond documentation has no idea how to do, so we settle for this,
    // which is not as nice, but actually a lot easier to implement.
    QLabel *notationSizeLabel = new QLabel(tr("Notation size:"), staffExportOptions);
    notationSizeLabel->setFont(m_font);
    m_notationSize = new QComboBox(staffExportOptions);
    m_notationSize->setFont(m_font);
    m_notationSize->setToolTip(tr("<qt><p>Choose normal, \\small or \\tiny font size for notation elements on this (normal-sized) staff when exporting to LilyPond.</p><p>This is as close as we get to enabling you to print parts in cue size</p></qt>"));
    m_notationSize->setMinimumWidth(width11);
    m_notationSize->addItem(tr("Normal"), StaffTypes::Normal);
    m_notationSize->addItem(tr("Small"), StaffTypes::Small);
    m_notationSize->addItem(tr("Tiny"), StaffTypes::Tiny);
    connect(m_notationSize, SIGNAL(activated(int)),
            this, SLOT(slotNotationSizeChanged(int)));

    // Bracket type
    // Staff bracketing (export only at the moment, but using this for GUI
    // rendering would be nice in the future!) //!!! 
    QLabel *bracketTypeLabel = new QLabel(tr("Bracket type:"), staffExportOptions);
    bracketTypeLabel->setFont(m_font);
    m_bracketType = new QComboBox(staffExportOptions);
    m_bracketType->setFont(m_font);
    m_bracketType->setToolTip(tr("<qt><p>Bracket staffs in LilyPond<br>(fragile, use with caution)</p><qt>"));
    m_bracketType->setMinimumWidth(width11);
    m_bracketType->addItem(tr("-----"), Brackets::None);
    m_bracketType->addItem(tr("[----"), Brackets::SquareOn);
    m_bracketType->addItem(tr("----]"), Brackets::SquareOff);
    m_bracketType->addItem(tr("[---]"), Brackets::SquareOnOff);
    m_bracketType->addItem(tr("{----"), Brackets::CurlyOn);
    m_bracketType->addItem(tr("----}"), Brackets::CurlyOff);
    m_bracketType->addItem(tr("{[---"), Brackets::CurlySquareOn);
    m_bracketType->addItem(tr("---]}"), Brackets::CurlySquareOff);
    connect(m_bracketType, SIGNAL(activated(int)),
            this, SLOT(slotBracketTypeChanged(int)));

    // Staff export options layout

    groupLayout = new QGridLayout(staffExportOptions);
    groupLayout->setContentsMargins(5,0,0,5);
    groupLayout->setVerticalSpacing(2);
    groupLayout->setHorizontalSpacing(5);
    groupLayout->setColumnStretch(1, 1);
    // Row 0: Notation size
    groupLayout->addWidget(notationSizeLabel, 0, 0, Qt::AlignLeft);
    groupLayout->addWidget(m_notationSize, 0, 1, 1, 2);
    // Row 1: Bracket type
    groupLayout->addWidget(bracketTypeLabel, 1, 0, Qt::AlignLeft);
    groupLayout->addWidget(m_bracketType, 1, 1, 1, 2);

    // Create segments with

    m_createSegmentsWithFrame = new CollapsingFrame(
            tr("Create segments with"), this, trackParametersDefaults);

    QWidget *createSegmentsWith = new QWidget(m_createSegmentsWithFrame);
    m_createSegmentsWithFrame->setWidget(createSegmentsWith);
    createSegmentsWith->setContentsMargins(3, 3, 3, 3);

    // Preset
    QLabel *presetLabel = new QLabel(tr("Preset"), createSegmentsWith);
    presetLabel->setFont(m_font);

    m_preset = new QLabel(tr("<none>"), createSegmentsWith);
    m_preset->setFont(m_font);
    m_preset->setObjectName("SPECIAL_LABEL");
    m_preset->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    m_preset->setMinimumWidth(width20);

    m_load = new QPushButton(tr("Load"), createSegmentsWith);
    m_load->setFont(m_font);
    m_load->setToolTip(tr("<qt><p>Load a segment parameters preset from our comprehensive database of real-world instruments.</p><p>When you create new segments, they will have these parameters at the moment of creation.  To use these parameters on existing segments (eg. to convert an existing part in concert pitch for playback on a Bb trumpet) use <b>Segments -> Convert notation for</b> in the notation editor.</p></qt>"));
    connect(m_load, SIGNAL(released()),
            SLOT(slotLoadPressed()));

    // Clef
    QLabel *clefLabel = new QLabel(tr("Clef"), createSegmentsWith);
    clefLabel->setFont(m_font);
    m_clef = new QComboBox(createSegmentsWith);
    m_clef->setFont(m_font);
    m_clef->setToolTip(tr("<qt><p>New segments will be created with this clef inserted at the beginning</p></qt>"));
    m_clef->setMinimumWidth(width11);
    m_clef->addItem(tr("treble", "Clef name"), TrebleClef);
    m_clef->addItem(tr("bass", "Clef name"), BassClef);
    m_clef->addItem(tr("crotales", "Clef name"), CrotalesClef);
    m_clef->addItem(tr("xylophone", "Clef name"), XylophoneClef);
    m_clef->addItem(tr("guitar", "Clef name"), GuitarClef);
    m_clef->addItem(tr("contrabass", "Clef name"), ContrabassClef);
    m_clef->addItem(tr("celesta", "Clef name"), CelestaClef);
    m_clef->addItem(tr("old celesta", "Clef name"), OldCelestaClef);
    m_clef->addItem(tr("french", "Clef name"), FrenchClef);
    m_clef->addItem(tr("soprano", "Clef name"), SopranoClef);
    m_clef->addItem(tr("mezzosoprano", "Clef name"), MezzosopranoClef);
    m_clef->addItem(tr("alto", "Clef name"), AltoClef);
    m_clef->addItem(tr("tenor", "Clef name"), TenorClef);
    m_clef->addItem(tr("baritone", "Clef name"), BaritoneClef);
    m_clef->addItem(tr("varbaritone", "Clef name"), VarbaritoneClef);
    m_clef->addItem(tr("subbass", "Clef name"), SubbassClef);
    m_clef->addItem(tr("twobar", "Clef name"), TwoBarClef);
    connect(m_clef, SIGNAL(activated(int)),
            this, SLOT(slotClefChanged(int)));

    // Transpose
    QLabel *transposeLabel = new QLabel(tr("Transpose"), createSegmentsWith);
    transposeLabel->setFont(m_font);
    m_transpose = new QComboBox(createSegmentsWith);
    m_transpose->setFont(m_font);
    m_transpose->setToolTip(tr("<qt><p>New segments will be created with this transpose property set</p></qt>"));
    connect(m_transpose, SIGNAL(activated(int)),
            SLOT(slotTransposeChanged(int)));

    int transposeRange = 48;
    for (int i = -transposeRange; i < transposeRange + 1; i++) {
        m_transpose->addItem(QString("%1").arg(i));
        if (i == 0)
            m_transpose->setCurrentIndex(m_transpose->count() - 1);
    }

    // Pitch
    QLabel *pitchLabel = new QLabel(tr("Pitch"), createSegmentsWith);
    pitchLabel->setFont(m_font);

    // Lowest playable note
    QLabel *lowestLabel = new QLabel(tr("Lowest"), createSegmentsWith);
    lowestLabel->setFont(m_font);

    m_lowest = new QPushButton(tr("---"), createSegmentsWith);
    m_lowest->setFont(m_font);
    m_lowest->setToolTip(tr("<qt><p>Choose the lowest suggested playable note, using a staff</p></qt>"));
    connect(m_lowest, SIGNAL(released()),
            SLOT(slotLowestPressed()));

    // Highest playable note
    QLabel *highestLabel = new QLabel(tr("Highest"), createSegmentsWith);
    highestLabel->setFont(m_font);

    m_highest = new QPushButton(tr("---"), createSegmentsWith);
    m_highest->setFont(m_font);
    m_highest->setToolTip(tr("<qt><p>Choose the highest suggested playable note, using a staff</p></qt>"));
    connect(m_highest, SIGNAL(released()),
            SLOT(slotHighestPressed()));

    updateHighLow();

    // Color
    QLabel *colorLabel = new QLabel(tr("Color"), createSegmentsWith);
    colorLabel->setFont(m_font);
    m_color = new QComboBox(createSegmentsWith);
    m_color->setFont(m_font);
    m_color->setToolTip(tr("<qt><p>New segments will be created using this color</p></qt>"));
    m_color->setEditable(false);
    m_color->setMaxVisibleItems(20);
    connect(m_color, SIGNAL(activated(int)),
            SLOT(slotColorChanged(int)));

    // "Create segments with" layout

    groupLayout = new QGridLayout(createSegmentsWith);
    groupLayout->setContentsMargins(5,0,0,5);
    groupLayout->setVerticalSpacing(2);
    groupLayout->setHorizontalSpacing(5);
    // Row 0: Preset/Load
    groupLayout->addWidget(presetLabel, 0, 0, Qt::AlignLeft);
    groupLayout->addWidget(m_preset, 0, 1, 1, 3);
    groupLayout->addWidget(m_load, 0, 4, 1, 2);
    // Row 1: Clef/Transpose
    groupLayout->addWidget(clefLabel, 1, 0, Qt::AlignLeft);
    groupLayout->addWidget(m_clef, 1, 1, 1, 2);
    groupLayout->addWidget(transposeLabel, 1, 3, 1, 2, Qt::AlignRight);
    groupLayout->addWidget(m_transpose, 1, 5, 1, 1);
    // Row 2: Pitch/Lowest/Highest
    groupLayout->addWidget(pitchLabel, 2, 0, Qt::AlignLeft);
    groupLayout->addWidget(lowestLabel, 2, 1, Qt::AlignRight);
    groupLayout->addWidget(m_lowest, 2, 2, 1, 1);
    groupLayout->addWidget(highestLabel, 2, 3, Qt::AlignRight);
    groupLayout->addWidget(m_highest, 2, 4, 1, 2);
    // Row 3: Color
    groupLayout->addWidget(colorLabel, 3, 0, Qt::AlignLeft);
    groupLayout->addWidget(m_color, 3, 1, 1, 5);

    groupLayout->setColumnStretch(1, 1);
    groupLayout->setColumnStretch(2, 2);

    // populate combo from doc colors
    slotDocColoursChanged();
    
    // Force a popluation of Record / Playback Devices (Playback was not populating).
    slotPopulateDeviceLists();

    // Connections

    // Detect when the document colours are updated
    connect(m_doc, SIGNAL(docColoursChanged()),
            this, SLOT(slotDocColoursChanged()));

    connect(Instrument::getStaticSignals().data(),
            SIGNAL(changed(Instrument *)),
            this,
            SLOT(slotInstrumentChanged(Instrument *)));

    m_doc->getComposition().addObserver(this);

    // Layout

    QGridLayout *mainLayout = new QGridLayout(this);
    mainLayout->setMargin(0);
    mainLayout->setSpacing(1);
    mainLayout->addWidget(m_trackLabel, 0, 0);
    mainLayout->addWidget(playbackParametersFrame, 1, 0);
    mainLayout->addWidget(recordingFiltersFrame, 2, 0);
    mainLayout->addWidget(staffExportOptionsFrame, 3, 0);
    mainLayout->addWidget(m_createSegmentsWithFrame, 4, 0);

    // Box

    setContentsMargins(2, 7, 2, 2);

    slotUpdateControls(-1);
}

void
TrackParameterBox::setDocument(RosegardenDocument *doc)
{
    if (m_doc != doc) {
        RG_DEBUG << "TrackParameterBox::setDocument\n";
        m_doc = doc;
        m_doc->getComposition().addObserver(this);
        slotPopulateDeviceLists();
    }
}

void
TrackParameterBox::slotPopulateDeviceLists()
{
    RG_DEBUG << "TrackParameterBox::slotPopulateDeviceLists()\n";
    populatePlaybackDeviceList();
    // Force a record device populate.
    m_lastInstrumentType = Instrument::InvalidInstrument;
    populateRecordingDeviceList();
    slotUpdateControls(-1);
}

void
TrackParameterBox::populatePlaybackDeviceList()
{
    RG_DEBUG << "TrackParameterBox::populatePlaybackDeviceList()\n";
    m_playbackDevice->clear();
    m_playbackDeviceIds.clear();
    m_instrument->clear();
    m_instrumentIds.clear();
    m_instrumentNames.clear();

    Studio &studio = m_doc->getStudio();

    // Get the list
    InstrumentList list = studio.getPresentationInstruments();
    InstrumentList::iterator it;
    int currentDevId = -1;

    for (it = list.begin(); it != list.end(); ++it) {

        if (! (*it))
            continue; // sanity check

        QString iname(QObject::tr((*it)->getName().c_str()));
        QString pname(QObject::tr((*it)->getProgramName().c_str()));
        Device *device = (*it)->getDevice();
        DeviceId devId = device->getId();

        if ((*it)->getType() == Instrument::SoftSynth) {
            iname.replace(QObject::tr("Synth plugin"), "");
            pname = "";
            AudioPluginInstance *plugin = (*it)->getPlugin
                                          (Instrument::SYNTH_PLUGIN_POSITION);
            if (plugin) {
                pname = strtoqstr(plugin->getProgram());
                QString identifier = strtoqstr(plugin->getIdentifier());
                if (identifier != "") {
                    QString type, soName, label;
                    PluginIdentifier::parseIdentifier
                    (identifier, type, soName, label);
                    if (pname == "") {
                        pname = strtoqstr(plugin->getDistinctiveConfigurationText());
                    }
                    if (pname != "") {
                        pname = QString("%1: %2").arg(label).arg(pname);
                    } else {
                        pname = label;
                    }
                }
            }
        }

        if (devId != (DeviceId)(currentDevId)) {
            currentDevId = int(devId);
            QString deviceName = QObject::tr(device->getName().c_str());
            m_playbackDevice->addItem(deviceName);
            m_playbackDeviceIds.push_back(currentDevId);
        }

        if (pname != "") iname += " (" + pname + ")";
        // cut off the redundant eg. "General MIDI Device" that appears in the
        // combo right above here anyway
        iname = iname.mid(iname.indexOf("#"), iname.length());
        m_instrumentIds[currentDevId].push_back((*it)->getId());
        m_instrumentNames[currentDevId].append(iname);

    }

    m_playbackDevice->setCurrentIndex(-1);
    m_instrument->setCurrentIndex(-1);
}

void
TrackParameterBox::populateRecordingDeviceList()
{
    RG_DEBUG << "TrackParameterBox::populateRecordingDeviceList()\n";

    Track *trk = getTrack();
    if (!trk)
        return;

    Instrument *inst = m_doc->getStudio().getInstrumentFor(trk);
    if (!inst)
        return ;

    // If we've changed instrument type, e.g. from Audio to MIDI,
    // reload the combos.
    if (m_lastInstrumentType != inst->getInstrumentType()) {
        m_lastInstrumentType = inst->getInstrumentType();

        m_recordingDevice->clear();
        m_recordingDeviceIds.clear();
        m_recordingChannel->clear();

        if (inst->getInstrumentType() == Instrument::Audio) {

            m_recordingDeviceIds.push_back(Device::NO_DEVICE);
            m_recordingDevice->addItem(tr("Audio"));
            m_recordingChannel->addItem(tr("Audio"));

            m_recordingDevice->setEnabled(false);
            m_recordingChannel->setEnabled(false);

            m_thruRouting->setCurrentIndex(0);
            m_thruRouting->setEnabled(false);

            // hide these for audio instruments
            m_createSegmentsWithFrame->setVisible(false);

        } else { // InstrumentType::Midi and InstrumentType::SoftSynth

            // show these if not audio instrument
            m_createSegmentsWithFrame->setVisible(true);

            m_recordingDeviceIds.push_back(Device::ALL_DEVICES);
            m_recordingDevice->addItem(tr("All"));

            DeviceList *devices = m_doc->getStudio().getDevices();
            DeviceListConstIterator it;
            for (it = devices->begin(); it != devices->end(); it++) {
                MidiDevice *dev =
                    dynamic_cast<MidiDevice*>(*it);
                if (dev) {
                    if (dev->getDirection() == MidiDevice::Record
                        && dev->isRecording()) {
                        QString deviceName = QObject::tr(dev->getName().c_str());
                        m_recordingDevice->addItem(deviceName);
                        m_recordingDeviceIds.push_back(dev->getId());
                    }
                }
            }

            m_recordingChannel->addItem(tr("All"));
            for (int i = 1; i < 17; ++i) {
                m_recordingChannel->addItem(QString::number(i));
            }

            m_recordingDevice->setEnabled(true);
            m_recordingChannel->setEnabled(true);
            m_thruRouting->setEnabled(true);
        }
    }

    if (inst->getInstrumentType() == Instrument::Audio) {
        m_recordingDevice->setCurrentIndex(0);
        m_recordingChannel->setCurrentIndex(0);
    } else {
        m_recordingDevice->setCurrentIndex(0);
        m_recordingChannel->setCurrentIndex((int)trk->getMidiInputChannel() + 1);
        for (unsigned int i = 0; i < m_recordingDeviceIds.size(); ++i) {
            if (m_recordingDeviceIds[i] == trk->getMidiInputDevice()) {
                m_recordingDevice->setCurrentIndex(i);
                break;
            }
        }
        m_thruRouting->setCurrentIndex((int)trk->getThruRouting());
    }
}

void
TrackParameterBox::updateHighLow()
{
    Composition &comp = m_doc->getComposition();
    Track *trk = comp.getTrackById(comp.getSelectedTrack());
    if (!trk)
        return ;

    trk->setHighestPlayable(m_highestPlayable);
    trk->setLowestPlayable(m_lowestPlayable);

    Accidental accidental = Accidentals::NoAccidental;

    Pitch highest(m_highestPlayable, accidental);
    Pitch lowest(m_lowestPlayable, accidental);

    QSettings settings;
    settings.beginGroup(GeneralOptionsConfigGroup);

    int base = settings.value("midipitchoctave", -2).toInt() ;
    settings.endGroup();

    bool includeOctave = false;

    // NOTE: this now uses a new, overloaded version of Pitch::getAsString()
    // that explicitly works with the key of C major, and does not allow the
    // calling code to specify how the accidentals should be written out.
    //
    // Separate the note letter from the octave to avoid undue burden on
    // translators having to retranslate the same thing but for a number
    // difference
    QString tmp = QObject::tr(highest.getAsString(includeOctave, base).c_str(), "note name");
    tmp += tr(" %1").arg(highest.getOctave(base));
    m_highest->setText(tmp);

    tmp = QObject::tr(lowest.getAsString(includeOctave, base).c_str(), "note name");
    tmp += tr(" %1").arg(lowest.getOctave(base));
    m_lowest->setText(tmp);

    m_preset->setEnabled(false);
}

void
TrackParameterBox::slotUpdateControls(int /*dummy*/)
{
    RG_DEBUG << "TrackParameterBox::slotUpdateControls()\n";

    // Device
    slotPlaybackDeviceChanged(-1);
    // Instrument
    slotInstrumentChanged(-1);

    Track *trk = getTrack();
    if (!trk)
        return;

    // Playback parameters
    m_archive->setChecked(trk->isArchived());

    // Create segments with
    m_clef->setCurrentIndex(trk->getClef());
    m_transpose->setCurrentIndex(m_transpose->findText(QString("%1").arg(trk->getTranspose())));
    m_color->setCurrentIndex(trk->getColor());
    m_highestPlayable = trk->getHighestPlayable();
    m_lowestPlayable = trk->getLowestPlayable();
    updateHighLow();
    // set this down here because updateHighLow just disabled the label
    m_preset->setText(strtoqstr(trk->getPresetLabel()));
    m_preset->setEnabled(true);

    // Staff export options
    m_notationSize->setCurrentIndex(trk->getStaffSize());
    m_bracketType->setCurrentIndex(trk->getStaffBracket());
}

void
TrackParameterBox::trackChanged(const Composition *, Track *track)
{
    if (!track)
        return;

    if (track->getId() != (unsigned)m_selectedTrackId)
        return;

    // Update the track name in case it has changed.
    selectedTrackNameChanged();
}

void
TrackParameterBox::tracksDeleted(const Composition *, std::vector<TrackId> &trackIds)
{
    //RG_DEBUG << "TrackParameterBox::tracksDeleted(), selected is " << m_selectedTrackId;

    // For each deleted track
    for (unsigned i = 0; i < trackIds.size(); ++i) {
        // If this is the selected track
        if ((int)trackIds[i] == m_selectedTrackId) {
            selectedTrackChanged2();
            return;
        }
    }
}

void
TrackParameterBox::trackSelectionChanged(const Composition *, TrackId)
{
    selectedTrackChanged2();
}

void
TrackParameterBox::selectedTrackChanged2()
{
    RG_DEBUG << "selectedTrackChanged2()";

    Composition &comp = m_doc->getComposition();
    TrackId newTrack = comp.getSelectedTrack();
    if ((int)newTrack != m_selectedTrackId) {
        m_preset->setEnabled(true);
        m_selectedTrackId = newTrack;
        selectedTrackNameChanged();
        slotUpdateControls(-1);
    }
}

void
TrackParameterBox::selectedTrackNameChanged()
{
    RG_DEBUG << "TrackParameterBox::selectedTrackNameChanged()";

    Track *trk = getTrack();
    if (!trk)
        return;

    QString trackName = strtoqstr(trk->getLabel());
    if (trackName.isEmpty())
        trackName = tr("<untitled>");
    else
        trackName.truncate(20);
    int trackNum = trk->getPosition() + 1;
    m_trackLabel->setText(tr("[ Track %1 - %2 ]").arg(trackNum).arg(trackName));
}

void
TrackParameterBox::slotPlaybackDeviceChanged(int index)
{
    RG_DEBUG << "TrackParameterBox::slotPlaybackDeviceChanged(" << index << ")\n";
    DeviceId devId;
    if (index == -1) {
        Track *trk = getTrack();
        if (!trk)
            return;

        Instrument *inst = m_doc->getStudio().getInstrumentFor(trk);
        if (!inst)
            return ;
        devId = inst->getDevice()->getId();
        int pos = -1;
        IdsVector::const_iterator it;
        // this works because we don't have a usable index, and we're having to
        // figure out what to set to.  the external change was to 10001, so we
        // hunt through our mangled data structure to find that.  this jibes
        // with the notion that our own representation of what's what does not
        // remotely match the TB menu representation.  Our data is the same, but
        // in a completely different and utterly nonsensical order, so we can
        // find it accurately if we know what we're hunting for, otherwise,
        // we're fucked
        for (it = m_playbackDeviceIds.begin(); it != m_playbackDeviceIds.end(); ++it) {
            pos++;
            if ((*it) == devId) break;
        }

        m_playbackDevice->setCurrentIndex(pos);
    } else {
        devId = m_playbackDeviceIds[index];
    }

    // used to be "General MIDI Device #7" now we change to "QSynth Device" and
    // we want to remember the #7 bit
    int previousIndex = m_instrument->currentIndex();

    // clear the instrument combo and re-populate it from the new device
    m_instrument->clear();
    m_instrument->addItems(m_instrumentNames[devId]);

    // try to keep the same index (the #7 bit) as was in use previously, unless
    // the new instrument has fewer indices available than the previous one did,
    // in which case we just go with the highest valid index available
    if (previousIndex > m_instrument->count()) previousIndex = m_instrument->count();

    populateRecordingDeviceList();

    if (index != -1) {
        m_instrument->setCurrentIndex(previousIndex);
        slotInstrumentChanged(previousIndex);
    }
}

void
TrackParameterBox::slotInstrumentChanged(int index)
{
    RG_DEBUG << "TrackParameterBox::slotInstrumentChanged(" << index << ")\n";
    DeviceId devId;
    Instrument *inst;
    if (index == -1) {
        Composition &comp = m_doc->getComposition();
        Track *trk = comp.getTrackById(comp.getSelectedTrack());
        if (!trk)
            return ;
        inst = m_doc->getStudio().getInstrumentFor(trk);
        if (!inst)
            return ;
        devId = inst->getDevice()->getId();

        int pos = -1;
        IdsVector::const_iterator it;
        for (it = m_instrumentIds[devId].begin(); it != m_instrumentIds[devId].end(); ++it) {
            pos++;
            if ((*it) == trk->getInstrument()) break;
        }
        m_instrument->setCurrentIndex(pos);
    } else {
        devId = m_playbackDeviceIds[m_playbackDevice->currentIndex()];

        // Calculate an index to use in Studio::getInstrumentFromList() which
        // gets emitted to TrackButtons, and TrackButtons actually does the work
        // of assigning the track to the instrument, for some bizarre reason.
        //
        // This new method for calculating the index works by:
        //
        // 1. for every play device combo index between 0 and its current index, 
        //
        // 2. get the device that corresponds with that combo box index, and
        //
        // 3. figure out how many instruments that device contains, then
        //
        // 4. Add it all up.  That's how many slots we have to jump over to get
        //    to the point where the instrument combo box index we're working
        //    with here will target the correct instrument in the studio list.
        //
        // I'm sure this whole architecture seemed clever once, but it's an
        // unmaintainable pain in the ass is what it is.  We changed one
        // assumption somewhere, and the whole thing fell on its head,
        // swallowing two entire days of my life to put back with the following
        // magic lines of code:
        int prepend = 0;
        for (int n = 0; n < m_playbackDevice->currentIndex(); n++) {
            DeviceId id = m_playbackDeviceIds[n];
            Device *dev = m_doc->getStudio().getDevice(id);

            // get the number of instruments belonging to the device (not the
            // studio)
            InstrumentList il = dev->getPresentationInstruments();
            prepend += il.size();
        }

        index += prepend;

        // emit the index we've calculated, relative to the studio list
        RG_DEBUG << "TrackParameterBox::slotInstrumentChanged() index = " << index << "\n";
        if (m_doc->getComposition().haveTrack(m_selectedTrackId)) {
            // TrackButtons does the rest of the work for us.
            // ??? Why not make the change directly to the Composition, then
            //     fire off an existing CompositionObserver notification?
            emit instrumentSelected(m_selectedTrackId, index);
        }
    }
}

void
TrackParameterBox::slotArchiveChanged(bool checked)
{
    //RG_DEBUG << "slotArchiveChanged(" << checked << ")";

    Track *track = getTrack();

    if (!track)
        return;

    track->setArchived(checked);
    m_doc->slotDocumentModified();

    // Notify observers
    Composition &comp = m_doc->getComposition();
    comp.notifyTrackChanged(track);
}

void
TrackParameterBox::slotRecordingDeviceChanged(int index)
{
    RG_DEBUG << "TrackParameterBox::slotRecordingDeviceChanged(" << index << ")";

    Track *trk = getTrack();
    if (!trk)
        return;

    Instrument *inst = m_doc->getStudio().getInstrumentFor(trk);
    if (!inst) return ;
    if (inst->getInstrumentType() == Instrument::Audio) {
        //Not implemented yet
    } else {
        trk->setMidiInputDevice(m_recordingDeviceIds[index]);
    }
}

void
TrackParameterBox::slotRecordingChannelChanged(int index)
{
    RG_DEBUG << "TrackParameterBox::slotRecordingChannelChanged(" << index << ")";

    Track *trk = getTrack();
    if (!trk)
        return;

    Instrument *inst = m_doc->getStudio().getInstrumentFor(trk);
    if (!inst) return ;
    if (inst->getInstrumentType() == Instrument::Audio) {
        //Not implemented yet
    } else {
        trk->setMidiInputChannel(index - 1);
    }
}

void
TrackParameterBox::slotThruRoutingChanged(int index)
{
    Track *track = getTrack();
    if (!track)
        return;

    Instrument *inst = m_doc->getStudio().getInstrumentFor(track);
    if (!inst)
        return;

    if (inst->getInstrumentType() == Instrument::Midi)
        track->setThruRouting(static_cast<Track::ThruRouting>(index));
}

void
TrackParameterBox::slotInstrumentChanged(Instrument * /*instrument*/)
{
    //RG_DEBUG << "TrackParameterBox::slotInstrumentChanged()";
    populatePlaybackDeviceList();
    slotUpdateControls(-1);
}

void
TrackParameterBox::slotClefChanged(int clef)
{
    RG_DEBUG << "TrackParameterBox::slotClefChanged(" << clef << ")";

    Track *trk = getTrack();
    if (!trk)
        return;

    trk->setClef(clef);
    m_preset->setEnabled(false);
}

void
TrackParameterBox::transposeChanged(int transpose)
{
    RG_DEBUG << "TrackParameterBox::transposeChanged(" << transpose << ")";

    Track *trk = getTrack();
    if (!trk)
        return;

    trk->setTranspose(transpose);
    m_preset->setEnabled(false);
}

void
TrackParameterBox::slotTransposeChanged(int index)
{
    transposeTextChanged(m_transpose->itemText(index));
}

void
TrackParameterBox::transposeTextChanged(QString text)
{
    // ??? inline into only caller.

    if (text.isEmpty()) return;
    int value = text.toInt();
    transposeChanged(value);
}

void
TrackParameterBox::slotDocColoursChanged()
{
    RG_DEBUG << "TrackParameterBox::slotDocColoursChanged()";

    m_color->clear();
    m_colourList.clear();
    // Populate it from composition.m_segmentColourMap
    ColourMap temp = m_doc->getComposition().getSegmentColourMap();

    unsigned int i = 0;

    for (RCMap::const_iterator it = temp.begin(); it != temp.end(); ++it) {
        QString qtrunc(QObject::tr(it->second.second.c_str()));
        QPixmap colour(15, 15);
        colour.fill(GUIPalette::convertColour(it->second.first));
        if (qtrunc == "") {
            m_color->addItem(colour, tr("Default"), i);
        } else {
            // truncate name to 25 characters to avoid the combo forcing the
            // whole kit and kaboodle too wide (This expands from 15 because the
            // translators wrote books instead of copying the style of
            // TheShortEnglishNames, and because we have that much room to
            // spare.)
            if (qtrunc.length() > 25)
                qtrunc = qtrunc.left(22) + "...";
            m_color->addItem(colour, qtrunc, i);
        }
        m_colourList[it->first] = i; // maps colour number to menu index
        ++i;
    }

    m_addColourPos = i;
    m_color->addItem(tr("Add New Color"), m_addColourPos);

    // remove the item we just inserted; this leaves the translation alone, but
    // eliminates the useless option
    //
    //!!! fix after release
    m_color->removeItem(m_addColourPos);

    m_color->setCurrentIndex(0);
}

void
TrackParameterBox::slotColorChanged(int index)
{
    RG_DEBUG << "TrackParameterBox::slotColorChanged(" << index << ")";

    Track *trk = getTrack();
    if (!trk)
        return;

    trk->setColor(index);

    if (index == m_addColourPos) {
        ColourMap newMap = m_doc->getComposition().getSegmentColourMap();
        QColor newColour;
        bool ok = false;
        
        QString newName = InputDialog::getText(this,
                                               tr("New Color Name"),
                                               tr("Enter new name:"),
                                               LineEdit::Normal,
                                               tr("New"), &ok);
        
        if ((ok == true) && (!newName.isEmpty())) {
//             QColorDialog box(this, "", true);
//             int result = box.getColor(newColour);
            
            //QRgb QColorDialog::getRgba(0xffffffff, &ok, this);
            QColor newColor = QColorDialog::getColor(Qt::white, this);

            if (newColor.isValid()) {
                Colour newRColour = GUIPalette::convertColour(newColour);
                newMap.addItem(newRColour, qstrtostr(newName));
                slotDocColoursChanged();
            }
        }
        // Else we don't do anything as they either didn't give a name
        // or didn't give a colour
    }
}

void
TrackParameterBox::slotHighestPressed()
{
    RG_DEBUG << "TrackParameterBox::slotHighestPressed()";

    if (m_selectedTrackId == (int)NO_TRACK) return;
    Composition &comp = m_doc->getComposition();
    if (!comp.haveTrack(m_selectedTrackId)) {
        m_selectedTrackId = (int)NO_TRACK;
        return;
    }

    PitchPickerDialog dialog(0, m_highestPlayable, tr("Highest playable note"));

    if (dialog.exec() == QDialog::Accepted) {
        m_highestPlayable = dialog.getPitch();
        updateHighLow();
    }

    m_preset->setEnabled(false);
}

void
TrackParameterBox::slotLowestPressed()
{
    RG_DEBUG << "TrackParameterBox::slotLowestPressed()";

    if (m_selectedTrackId == (int)NO_TRACK) return;
    Composition &comp = m_doc->getComposition();
    if (!comp.haveTrack(m_selectedTrackId)) {
        m_selectedTrackId = (int)NO_TRACK;
        return;
    }

    PitchPickerDialog dialog(0, m_lowestPlayable, tr("Lowest playable note"));

    if (dialog.exec() == QDialog::Accepted) {
        m_lowestPlayable = dialog.getPitch();
        updateHighLow();
    }

    m_preset->setEnabled(false);
}

void
TrackParameterBox::slotLoadPressed()
{
    RG_DEBUG << "TrackParameterBox::slotLoadPressed()";

    //PresetHandlerDialog dialog(this);
    PresetHandlerDialog dialog(0); // no parent means no style from group box parent, but what about popup location?

    Track *trk = getTrack();
    if (!trk)
        return;

    try {
        if (dialog.exec() == QDialog::Accepted) {
            m_preset->setText(dialog.getName());
            trk->setPresetLabel(qstrtostr(dialog.getName()));
            if (dialog.getConvertAllSegments()) {
                Composition &comp = m_doc->getComposition();
                SegmentSyncCommand* command = new SegmentSyncCommand(
                        comp.getSegments(), m_selectedTrackId,
                        dialog.getTranspose(), dialog.getLowRange(), 
                        dialog.getHighRange(),
                        clefIndexToClef(dialog.getClef()));
                CommandHistory::getInstance()->addCommand(command);
            }
            m_clef->setCurrentIndex(dialog.getClef());
                     
            m_transpose->setCurrentIndex(m_transpose->findText(QString("%1").arg(dialog.getTranspose())));

            m_highestPlayable = dialog.getHighRange();
            m_lowestPlayable = dialog.getLowRange();
            updateHighLow();
            slotClefChanged(dialog.getClef());
            transposeChanged(dialog.getTranspose());

            // the preceding slots will have set this disabled, so we
            // re-enable it until it is subsequently re-disabled by the
            // user overriding the preset, calling one of the above slots
            // in the normal course
            m_preset->setEnabled(true);
        }
    } catch (Exception e) {
        //!!! This should be a more verbose error to pass along the
        // row/column of the corruption, but I can't be bothered to work
        // that out just at the moment.  Hopefully this code will never
        // execute anyway.
        QMessageBox::warning(0, tr("Rosegarden"), tr("The instrument preset database is corrupt.  Check your installation."));
    }

}

void
TrackParameterBox::slotNotationSizeChanged(int index)
{
    RG_DEBUG << "TrackParameterBox::sotStaffSizeChanged()";

    Track *trk = getTrack();
    if (!trk)
        return;

    trk->setStaffSize(index);
}


void
TrackParameterBox::slotBracketTypeChanged(int index)
{
    RG_DEBUG << "TrackParameterBox::sotStaffBracketChanged()";

    Track *trk = getTrack();
    if (!trk)
        return;

    trk->setStaffBracket(index);
}

Track *
TrackParameterBox::getTrack()
{
    if (m_selectedTrackId == (int)NO_TRACK)
        return NULL;

    if (!m_doc)
        return NULL;

    Composition &comp = m_doc->getComposition();
    // If the track is gone, bail.
    if (!comp.haveTrack(m_selectedTrackId)) {
        m_selectedTrackId = (int)NO_TRACK;
        return NULL;
    }

    return comp.getTrackById(m_selectedTrackId);
}


}
