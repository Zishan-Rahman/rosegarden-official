// -*- c-basic-offset: 4 -*-

/*
    Rosegarden-4
    A sequencer and musical notation editor.

    This program is Copyright 2000-2003
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

#include "mixer.h"
#include "rosegardenguidoc.h"
#include "sequencermapper.h"
#include "rosedebug.h"
#include "studiocontrol.h"

#include "Studio.h"
#include "AudioLevel.h"

#include <klocale.h>
#include <kconfig.h>
#include <kstdaction.h>
#include <kaction.h>
#include <kglobal.h>
#include <kstddirs.h>

#include <qlayout.h>

MixerWindow::MixerWindow(QWidget *parent,
			 RosegardenGUIDoc *document) :
    KMainWindow(parent, "mixerwindow"),
    m_document(document),
    m_studio(&document->getStudio())
{
    QFont font;
    font.setPointSize(font.pointSize() * 8 / 10);
    font.setBold(false);
    setFont(font);

    setBackgroundMode(Qt::PaletteMidlight);

    QFrame *mainBox = new QFrame(this);
    setCentralWidget(mainBox);
    QHBoxLayout *mainLayout = new QHBoxLayout(mainBox, 4, 4);

    setCaption(i18n("Mixer"));

    Rosegarden::InstrumentList instruments = m_studio->getPresentationInstruments();

    int count = 1;
    for (Rosegarden::InstrumentList::iterator i = instruments.begin();
	 i != instruments.end(); ++i) {
	
	if ((*i)->getType() == Rosegarden::Instrument::Audio) {

	    AudioFaderWidget *fader = 
		new AudioFaderWidget(mainBox, AudioFaderWidget::FaderStrip,
				     QString("%1").arg(count));

	    connect(fader->m_fader, SIGNAL(faderChanged(float)),
		    this, SLOT(slotFaderLevelChanged(float)));

	    mainLayout->addWidget(fader);
	    m_faders[(*i)->getId()] = fader;
	    ++count;
	}
    }
    
    Rosegarden::BussList busses = m_studio->getBusses();
    
    count = 1;
    for (Rosegarden::BussList::iterator i = busses.begin();
	 i != busses.end(); ++i) {

	if (i == busses.begin()) continue; // that one's the master

	AudioFaderWidget *fader = 
	    new AudioFaderWidget(mainBox, AudioFaderWidget::FaderStrip,
				 QString("sub%1").arg(count), false);

	connect(fader->m_fader, SIGNAL(faderChanged(float)),
		this, SLOT(slotFaderLevelChanged(float)));

	mainLayout->addWidget(fader);
	m_submasters.push_back(fader);
	++count;
    }

    if (busses.size() > 0) {
	AudioFaderWidget *fader = 
	    new AudioFaderWidget
	    (mainBox, AudioFaderWidget::FaderStrip, "M", false);
	connect(fader->m_fader, SIGNAL(faderChanged(float)),
		this, SLOT(slotFaderLevelChanged(float)));

	mainLayout->addWidget(fader);
	m_master = fader;
    }
}

MixerWindow::~MixerWindow()
{

}


void
MixerWindow::slotFaderLevelChanged(float dB)
{
    const QObject *s = sender();

    Rosegarden::BussList busses = m_studio->getBusses();
    
    if (s == m_master->m_fader) {

	if (busses.size() > 0) {
	    Rosegarden::StudioControl::setStudioObjectProperty
		(Rosegarden::MappedObjectId((*busses.begin())->getMappedId()),
		 Rosegarden::MappedAudioBuss::Level,
		 Rosegarden::MappedObjectValue(dB));
	}

	return;
    } 

    int index = 1;

    for (FaderVector::iterator i = m_submasters.begin();
	 i != m_submasters.end(); ++i) {

	if (s == (*i)->m_fader) {
	    if (busses.size() > index) {
		Rosegarden::StudioControl::setStudioObjectProperty
		    (Rosegarden::MappedObjectId(busses[index]->getMappedId()),
		     Rosegarden::MappedAudioBuss::Level,
		     Rosegarden::MappedObjectValue(dB));
	    }

	    return;
	}

	++index;
    }

    for (FaderMap::iterator i = m_faders.begin();
	 i != m_faders.end(); ++i) {
	
	if (s == i->second->m_fader) {
	    Rosegarden::StudioControl::setStudioObjectProperty
		(Rosegarden::MappedObjectId
		 (m_studio->getInstrumentById(i->first)->getMappedId()),
		 Rosegarden::MappedAudioFader::FaderLevel,
		 Rosegarden::MappedObjectValue(dB));
	}
    }	    

}

void
MixerWindow::closeEvent(QCloseEvent *e)
{
    emit closing();
    KMainWindow::closeEvent(e);
}

void
MixerWindow::updateMeters(SequencerMapper *mapper)
{
    for (FaderMap::iterator i = m_faders.begin(); i != m_faders.end(); ++i) {

	Rosegarden::InstrumentId id = i->first;
	AudioFaderWidget *fader = i->second;
	if (!fader) continue;

	Rosegarden::LevelInfo info;
	if (!mapper->getInstrumentLevel(id, info)) continue;

	RG_DEBUG << "MixerWindow::updateMeters: id " << id << ", level left "
		 << info.level << endl;
	
	// The values passed through are long-fader values
	float dBleft = Rosegarden::AudioLevel::fader_to_dB
	    (info.level, 127, Rosegarden::AudioLevel::LongFader);

	if (fader->isStereo()) {
	    float dBright = Rosegarden::AudioLevel::fader_to_dB
		(info.levelRight, 127, Rosegarden::AudioLevel::LongFader);

	    fader->m_vuMeter->setLevel(dBleft, dBright);

	} else {
	    fader->m_vuMeter->setLevel(dBleft);
	}
    }

    for (unsigned int i = 0; i < m_submasters.size(); ++i) {

	AudioFaderWidget *fader = m_submasters[i];
	if (!fader) continue;

	Rosegarden::LevelInfo info;
	if (!mapper->getSubmasterLevel(i, info)) continue;

	RG_DEBUG << "MixerWindow::updateMeters: sub " << i << ", level left "
		 << info.level << endl;
	
	// The values passed through are long-fader values
	float dBleft = Rosegarden::AudioLevel::fader_to_dB
	    (info.level, 127, Rosegarden::AudioLevel::LongFader);
	float dBright = Rosegarden::AudioLevel::fader_to_dB
	    (info.levelRight, 127, Rosegarden::AudioLevel::LongFader);

	fader->m_vuMeter->setLevel(dBleft, dBright);
    }

    Rosegarden::LevelInfo masterInfo;
    if (mapper->getMasterLevel(masterInfo)) {

	float dBleft = Rosegarden::AudioLevel::fader_to_dB
	    (masterInfo.level, 127, Rosegarden::AudioLevel::LongFader);
	float dBright = Rosegarden::AudioLevel::fader_to_dB
	    (masterInfo.levelRight, 127, Rosegarden::AudioLevel::LongFader);

	m_master->m_vuMeter->setLevel(dBleft, dBright);
    }
}

