// $Id: fileinjecttrafficmanager.cpp ??? 2014-03-08 $

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

#include <limits>
#include <sstream>
#include <fstream>

#include "packet_reply_info.hpp"
#include "random_utils.hpp"
#include "fileinjecttrafficmanager.hpp"

FileInjectTrafficManager::FileInjectTrafficManager( const Configuration &config, 
					  const vector<Network *> & net )
: TrafficManager(config, net), _last_id(-1), _last_pid(-1), 
   _overall_min_fileInject_time(0), _overall_avg_fileInject_time(0), 
   _overall_max_fileInject_time(0)
{
  _time_ratio = config.GetFloat("time_ratio");
  _max_outstanding = config.GetInt ("max_outstanding_requests");  

  _fileInject_time = new Stats( this, "fileInject_time", 1.0, 1000 );
  _stats["fileInject_time"] = _fileInject_time;
  
  string sent_packets_out_file = config.GetStr( "sent_packets_out" );
  if(sent_packets_out_file == "") {
    _sent_packets_out = NULL;
  } else {
    _sent_packets_out = new ofstream(sent_packets_out_file.c_str());
  }
}

FileInjectTrafficManager::~FileInjectTrafficManager( )
{
  delete _fileInject_time;
  if(_sent_packets_out) delete _sent_packets_out;
}

void FileInjectTrafficManager::_RetireFlit( Flit *f, int dest )
{
  _last_id = f->id;
  _last_pid = f->pid;
  TrafficManager::_RetireFlit(f, dest);
}

void FileInjectTrafficManager::_ClearStats( )
{
  TrafficManager::_ClearStats();
  _fileInject_time->Clear( );
}

void FileInjectTrafficManager::_GeneratePacketFromFile( int cl, int time )
{
  Flit * f;
  int source;
  int packet_destination;
  int size;
  int next_cycle;
  bool exit_generating_loop = false;

  bool record = false;
  
  while (!exit_generating_loop) {
    int pid = _cur_pid++;
    assert(_cur_pid);
    bool watch = gWatchOut && (_packets_to_watch.count(pid) > 0);
    Flit::FlitType packet_type = Flit::ANY_TYPE;
    *_injectionFile >> source;
    if ( (source < 0) || (source >= _nodes) ){
      cout << "\n====== By s.h.seyyedaghaei@gmail.com ======\n" << "Error in file: " << injectionFileAddress << "\n\nSource: " << source << "\n";
      Error("The source in your injection file is an invalid source node!");
    }
    *_injectionFile >> packet_destination;
    if ( (packet_destination < 0) || (packet_destination >= _nodes) ){
      cout << "\n====== By s.h.seyyedaghaei@gmail.com ======\n" << "Error in file: " << injectionFileAddress << "\n\nDestination: " << packet_destination << "\n";
      Error("The above destination in your injection file is an invalid destination");
    }
    *_injectionFile >> size;
    
    if ( ( _sim_state == running ) ||
         ( ( _sim_state == draining ) && ( time < _drain_time ) ) ) {
        record = _measure_stats[cl];
    }
    
    int subnetwork = ((packet_type == Flit::ANY_TYPE) ? 
                      RandomInt(_subnets-1) :
                      _subnet[packet_type]);
    
    if ( watch ) { 
        *gWatchOut << "====== From file " << injectionFileAddress << " ======\n"<< GetSimTime() << " | "
                   << "node" << source << " | "
                   << "Enqueuing packet " << pid
                   << " at time " << time
                   << "." << endl;
    }
    
    for ( int i = 0; i < size; ++i ) {
      f = Flit::New();
      f->id     = _cur_id++;
      assert(_cur_id);
      f->pid    = pid;
      f->watch  = watch | (gWatchOut && (_flits_to_watch.count(f->id) > 0));
      f->subnetwork = subnetwork;
      f->src    = source;
      f->ctime   = time;
      f->record = record;
      f->cl = cl;
      
      _total_in_flight_flits[f->cl].insert(make_pair(f->id, f));
      if(record) {
          _measured_in_flight_flits[f->cl].insert(make_pair(f->id, f));
      }
      
      if(gTrace){
	cout<<"New Flit "<<f->src<<endl;
      }
      f->type = packet_type;

      if ( i == 0 ) { // Head flit
	f->head = true;
	//packets are only generated to nodes smaller or equal to limit
	f->dest = packet_destination;
      } else {
	f->head = false;
	f->dest = -1;
      }
    
      f->pri = 0;

      if ( i == ( size - 1 ) ) { // Tail flit
	f->tail = true;
      } else {
	f->tail = false;
      }
    
      f->vc  = -1;

      if ( f->watch ) {
	*gWatchOut << "====== From file " << injectionFileAddress << " ======\n" << GetSimTime() << " | "
                       << "node" << source << " | "
                       << "Enqueuing flit " << f->id
                       << " (packet " << f->pid
                       << ") at time " << time
                       << "." << endl;
      }

      _partial_packets[source][cl].push_back(f);
    }
    *_injectionFile >> next_cycle;

    next_cycle *= _time_ratio;
    if ( next_cycle != _generate_start_cycle || (*_injectionFile).eof()){
       if ( (next_cycle < _generate_start_cycle) && (!((*_injectionFile).eof())) ){
	Error("Error in the injection file, the cycles for packet generating must be incremental.");
       }
      exit_generating_loop = true;
      _generate_start_cycle = next_cycle; 
    }
  }
}

void FileInjectTrafficManager::_Inject(){

   for ( int c = 0; c < _classes; ++c ) {
       if ( _generate_start_cycle == _time && !(*_injectionFile).eof() ){
          _GeneratePacketFromFile(c,_generate_start_cycle);
       }
    }
}

bool FileInjectTrafficManager::_SingleSim( )
{
  if(injectionFileAddress) {
    _packet_seq_no.assign(_nodes, 0);
    _last_id = -1;
    _last_pid = -1;
    _sim_state = running;
    int start_time = _time;
    cout << "Injection file " << injectionFileAddress << " started ..." << endl;
    _injectionFile = new ifstream(injectionFileAddress);
    *_injectionFile >> _generate_start_cycle;
    _generate_start_cycle *= _time_ratio;
    _time = _generate_start_cycle;
    do {
      _Step();
      if(_sent_packets_out) {
	*_sent_packets_out << _packet_seq_no << endl;
      }
    } while(!(*_injectionFile).eof());
    cout << "Injection file finished. Time used is " << _time - start_time << " cycles." << endl;

    int sent_time = _time;
    cout << "Waiting for injection to complete..." << endl;

    int empty_steps = 0;
    
    bool packets_left = false;
    for(int c = 0; c < _classes; ++c) {
      packets_left |= !_total_in_flight_flits[c].empty();
    }
    
    while( packets_left ) { 
      _Step( ); 
      
      ++empty_steps;
      
      if ( empty_steps % 1000 == 0 ) {
	_DisplayRemaining( ); 
	cout << ".";
      }
      
      packets_left = false;
      for(int c = 0; c < _classes; ++c) {
	packets_left |= !_total_in_flight_flits[c].empty();
      }
    }
    cout << endl;
    cout << "injection completed. Time used is " << _time - sent_time << " cycles." << endl
	 << "Last packet was " << _last_pid << ", last flit was " << _last_id << "." << endl;

    _fileInject_time->AddSample(_time - start_time);

    cout << _sim_state << endl;

    UpdateStats();
    DisplayStats();
  }
  else
  {
    cout << "\nError: You didn't specify a injection file as the second argument\n\n";
    return 0;
  }
  _sim_state = draining;
  _drain_time = _time;
  return 1;
}

void FileInjectTrafficManager::_UpdateOverallStats() {
  TrafficManager::_UpdateOverallStats();
  _overall_min_fileInject_time += _fileInject_time->Min();
  _overall_avg_fileInject_time += _fileInject_time->Average();
  _overall_max_fileInject_time += _fileInject_time->Max();
}
  
string FileInjectTrafficManager::_OverallStatsCSV(int c) const
{
  ostringstream os;
  os << TrafficManager::_OverallStatsCSV(c) << ','
     << _overall_min_fileInject_time / (double)_total_sims << ','
     << _overall_avg_fileInject_time / (double)_total_sims << ','
     << _overall_max_fileInject_time / (double)_total_sims;
  return os.str();
}

void FileInjectTrafficManager::WriteStats(ostream & os) const
{
  TrafficManager::WriteStats(os);
  os << "fileInject_time = " << _fileInject_time->Average() << ";" << endl;
}    

void FileInjectTrafficManager::DisplayStats(ostream & os) const {
  TrafficManager::DisplayStats();
  os << "Minimum fileInject duration = " << _fileInject_time->Min() << endl;
  os << "Average fileInject duration = " << _fileInject_time->Average() << endl;
  os << "Maximum fileInject duration = " << _fileInject_time->Max() << endl;
}

void FileInjectTrafficManager::DisplayOverallStats(ostream & os) const {
  TrafficManager::DisplayOverallStats(os);
  os << "Overall min fileInject duration = " << _overall_min_fileInject_time / (double)_total_sims
     << " (" << _total_sims << " samples)" << endl
     << "Overall min fileInject duration = " << _overall_avg_fileInject_time / (double)_total_sims
     << " (" << _total_sims << " samples)" << endl
     << "Overall min fileInject duration = " << _overall_max_fileInject_time / (double)_total_sims
     << " (" << _total_sims << " samples)" << endl;
}
