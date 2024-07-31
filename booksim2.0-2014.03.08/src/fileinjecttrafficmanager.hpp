// $Id: fileinjecttrafficmanager.hpp ??? 2014-03-08 dub $

/*
 Copyright (c) 2007-2012, Trustees of The Leland Stanford Junior University
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

 Redistributions of source code must retain the above copyright notice, this 
 list of conditions and the following disclaimer.
 Redistributions in binary form must reproduce the above copyright notice, this
 list of conditions and the following disclaimer in the documentation and/or
 other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 
 added by s.h.seyyedaghaei@gmail.com at the University of Tehran
 this is added to get the injected traffic from files,
 each line of the file must be in following format
 <timeOfPacketgeneration> <idOfSourceNode> <idOfDistinationNode> <theSizeOfPacket>
 
 example: for a 2*2 mesh we want send a packet with size 5 flits from node 0 to node 3 at 10th cylce,
          we must have the following line into injection file.
          
          .
          .
          10 0 3 5
          .
          .
          note that the generation cycle must be incremental.
*/

#ifndef _FILEINJECTTRAFFICMANAGER_HPP_
#define _FILEINJECTTRAFFICMANAGER_HPP_

#include <iostream>

#include "config_utils.hpp"
#include "stats.hpp"
#include "trafficmanager.hpp"

class FileInjectTrafficManager : public TrafficManager {

protected:

  ifstream *_injectionFile;
  ifstream *_timeSlotsFile;
  int _generate_start_cycle;
  float _time_ratio;

  int _max_outstanding;
  int _last_id;
  int _last_pid;

  Stats * _fileInject_time;
  double _overall_min_fileInject_time;
  double _overall_avg_fileInject_time;
  double _overall_max_fileInject_time;

  ostream * _sent_packets_out;

  virtual void _Inject();
  void _GeneratePacketFromFile(int cl, int time);
  virtual void _RetireFlit( Flit *f, int dest );

  virtual void _ClearStats( );
  virtual bool _SingleSim( );

  virtual void _UpdateOverallStats( );

  virtual string _OverallStatsCSV(int c = 0) const;

public:

  FileInjectTrafficManager( const Configuration &config, const vector<Network *> & net );
  virtual ~FileInjectTrafficManager( );

  virtual void WriteStats( ostream & os = cout ) const;
  virtual void DisplayStats( ostream & os = cout ) const;
  virtual void DisplayOverallStats( ostream & os = cout ) const;

};

#endif
