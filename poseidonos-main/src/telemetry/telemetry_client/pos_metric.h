/*
 *   BSD LICENSE
 *   Copyright (c) 2021 Samsung Electronics Corporation
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *     * Neither the name of Samsung Electronics Corporation nor the names of
 *       its contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "src/telemetry/telemetry_id.h"
#include <map>
#include <chrono>
#include <iomanip>
#include <list>
#include <sstream>
#include <string>
#include <unordered_map>
#include <set>
#include <vector>

namespace pos
{
struct POSHistogramValue
{
    // TODO: RESERVED
    std::map<std::string, uint64_t> buckets; // insert duplicated key?
    int64_t sum;
    uint64_t count;
};

enum POSMetricTypes {
    MT_COUNT,
    MT_GAUGE,
    MT_HISTOGRAM, // TODO: RESERVED
    MT_NUM_TYPE = MT_HISTOGRAM
};

class POSMetricValue
{
public:
    POSMetricValue(void)
    {
        count = 0;
        gauge = 0;
        // histogram = nullptr
    }
    // POSHistogramValue* histogram;
    uint64_t count;
    int64_t gauge;
};

typedef std::unordered_map<std::string, std::string> MetricLabelMap;

class POSMetric
{
public:
    POSMetric(std::string name, POSMetricTypes type);
    virtual ~POSMetric(void) = default;
    virtual void SetName(std::string name_);
    // virtual void SetTime(time_t time_);
    virtual void SetType(POSMetricTypes type_);
    virtual void SetCountValue(uint64_t count_);
    virtual void SetGaugeValue(int64_t gauge_);
    virtual int AddLabel(std::string key, std::string value);

    virtual std::string GetName(void);
    virtual POSMetricTypes GetType(void);
    // virtual time_t GetTime(void);
    virtual uint64_t GetCountValue(void);
    virtual int64_t GetGaugeValue(void);
    virtual MetricLabelMap* GetLabelList(void);

private:
    POSMetricTypes type;       // Mandatory
    std::string name;          // Mandatory
    // time_t time;               // optional (preset)
    POSMetricValue value;      // Mandatory
    MetricLabelMap labelList;  // optional (default: publisher's name)
};

typedef std::vector<POSMetric> POSMetricVector;

} // namespace pos
