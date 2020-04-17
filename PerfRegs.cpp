//
// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2018 Western Digital Corporation or its affiliates.
// 
// This program is free software: you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation, either version 3 of the License, or (at your option)
// any later version.
// 
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
// more details.
// 
// You should have received a copy of the GNU General Public License along with
// this program. If not, see <https://www.gnu.org/licenses/>.
//

#include <algorithm>
#include "PerfRegs.hpp"


using namespace WdRiscv;


PerfRegs::PerfRegs(unsigned numCounters)
{
  // 29 counters: MHPMCOUNTER3 to MHPMCOUNTER31
  counters_.resize(29);

  config(numCounters, unsigned(EventNumber::_End));
}


void
PerfRegs::config(unsigned numCounters, unsigned maxEventId)
{
  assert(numCounters < counters_.size());

  eventOfCounter_.resize(numCounters);
  enableUser_.resize(numCounters);
  enableMachine_.resize(numCounters);
  modified_.resize(numCounters);

  unsigned numEvents = std::max(unsigned(EventNumber::_End), maxEventId) + 1;
  countersOfEvent_.resize(numEvents);
}


bool
PerfRegs::applyPerfEventAssign()
{
  if (not hasPending_)
    return false;

  hasPending_ = false;

  if (pendingCounter_ >= eventOfCounter_.size())
    return false;

  // Disassociate counter from its previous event.
  EventNumber prevEvent = eventOfCounter_.at(pendingCounter_);
  if (prevEvent != EventNumber::None)
    {
      auto& vec = countersOfEvent_.at(size_t(prevEvent));
      vec.erase(std::remove(vec.begin(), vec.end(), pendingCounter_), vec.end());
    }

  if (size_t(pendingEvent_) >= countersOfEvent_.size())
    return false;

  if (pendingEvent_ != EventNumber::None)
    countersOfEvent_.at(size_t(pendingEvent_)).push_back(pendingCounter_);

  eventOfCounter_.at(pendingCounter_) = pendingEvent_;
  enableUser_.at(pendingCounter_) = pendingUser_;
  enableMachine_.at(pendingCounter_) = pendingMachine_;

  return true;
}


void
PerfRegs::reset()
{
  eventOfCounter_.assign(eventOfCounter_.size(), EventNumber::None);

  for (auto& vec : countersOfEvent_)
    vec.clear();
}
