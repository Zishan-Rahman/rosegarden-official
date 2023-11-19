/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*- vi:set ts=8 sts=4 sw=4: */

/*
    Rosegarden
    A sequencer and musical notation editor.
    Copyright 2000-2022 the Rosegarden development team.
    See the AUTHORS file for more details.

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#ifndef RG_LV2UTILS_H
#define RG_LV2UTILS_H

#include <QMutex>

#include <map>
#include <lilv/lilv.h>
#include <lv2/urid/urid.h>
#include <lv2/worker/worker.h>

#include "base/Instrument.h"

namespace Rosegarden
{

class LV2PluginInstance;
class AudioPluginLV2GUI;

/// LV2 utils
/**
 * LV2Utils encapsulate the lv2 world and supports communcation
 * between LV2 plugins and LV2 uis (different threads),
 */

class LV2Utils
{
 public:
    /// Singleton
    static LV2Utils *getInstance();

    LV2Utils(LV2Utils &other) = delete;
    void operator=(const LV2Utils &) = delete;

    LV2_URID uridMap(const char *uri);
    const char* uridUnmap(LV2_URID urid);

    LV2_URID_Map m_map;
    LV2_URID_Unmap m_unmap;

    enum LV2PortType {LV2CONTROL, LV2AUDIO, LV2MIDI};
    enum LV2PortProtocol {LV2FLOAT, LV2ATOM};

    struct LV2PortData
    {
        QString name;
        LV2PortType portType;
        LV2PortProtocol portProtocol;
        bool isInput;
        float min;
        float max;
        float def;
        int displayHint;
    };

    struct LV2PluginData
    {
        QString name;
        QString label; // abbreviated name
        QString pluginClass;
        QString author;
        bool isInstrument;
        std::vector<LV2PortData> ports;
    };

    // key type for maps
    struct PluginPosition
    {
        InstrumentId instrument;
        int position;
        bool operator<(const PluginPosition &p) const
        {
            if (instrument < p.instrument) return true;
            if (instrument > p.instrument) return false;
            if (position < p.position) return true;
            return false;
        }
    };

    // interface for a worker class
    typedef LV2_Worker_Status (*ScheduleWork)(LV2_Worker_Schedule_Handle handle,
                                              uint32_t size,
                                              const void* data);

    typedef LV2_Worker_Status (*RespondWork)(LV2_Worker_Respond_Handle handle,
                                             uint32_t size,
                                             const void *data);

    struct WorkerJob
    {
        uint32_t size;
        const void* data;
    };

    class Worker
    {
    public:
        virtual ~Worker() {};
        virtual ScheduleWork getScheduler() = 0;
        virtual WorkerJob* getResponse(const LV2Utils::PluginPosition& pp) = 0;
    };

    const std::map<QString, LV2PluginData>& getAllPluginData() const;
    const LilvPlugin* getPluginByUri(const QString& uri) const;
    LV2PluginData getPluginData(const QString& uri) const;
    const LilvUIs* getPluginUIs(const QString& uri) const;
    LilvNode* makeURINode(const QString& uri) const;

    void lock();
    void unlock();

    void registerPlugin(InstrumentId instrument,
                        int position,
                        LV2PluginInstance* pluginInstance);
    void registerGUI(InstrumentId instrument,
                     int position,
                     AudioPluginLV2GUI* gui);

    void registerWorker(Worker* worker);

    void unRegisterPlugin(InstrumentId instrument,
                          int position);
    void unRegisterGUI(InstrumentId instrument,
                       int position);

    void unRegisterWorker();

    int numInstances(InstrumentId instrument,
                     int position) const;

    Worker* getWorker() const;

    void runWork(const PluginPosition& pp,
                 uint32_t size,
                 const void* data,
                 LV2_Worker_Respond_Function resp);

 private:
    /// Singleton.  See getInstance().
    LV2Utils();
    ~LV2Utils();

    //urid map
    std::map<std::string, int> m_uridMap;
    std::map<int, std::string> m_uridUnmap;
    int m_nextId;

    QMutex m_mutex;
    LilvWorld* m_world;
    const LilvPlugins* m_plugins;
    std::map<QString, LV2PluginData> m_pluginData;

    struct LV2UPlugin
    {
        LV2PluginInstance* pluginInstance;
        AudioPluginLV2GUI* gui;
        LV2UPlugin() {pluginInstance = nullptr; gui = nullptr;}
    };

    typedef std::map<PluginPosition, LV2UPlugin> PluginGuiMap;
    PluginGuiMap m_pluginGuis;

    Worker* m_worker;
};

}

#endif // RG_LV2UTILS_H
