#include "rosic_EnvelopeGenerator_elanEdit.h"

using namespace rosicElanEdit;

//-------------------------------------------------------------------------------------------------
// construction/destruction:

rosicElanEdit::EnvelopeGenerator::EnvelopeGenerator()
{
//	modtype = ModType::ENV;
  // The BreakpointModulator baseclass initially has 5 breakpoints defining an ADSR envelope. Here, 
  // we set up times, levels, shapes, etc. of these breakpoints:
  modifyBreakpoint(0, 0.0,  0.0, rosic::ModBreakpoint::ANALOG, 1.0); // start
  modifyBreakpoint(1, 0.25, 1.0, rosic::ModBreakpoint::ANALOG, 1.0); // peak
  modifyBreakpoint(2, 1.0,  0.5, rosic::ModBreakpoint::ANALOG, 1.0); // sustain-loop start
  modifyBreakpoint(3, 2.0,  0.5, rosic::ModBreakpoint::ANALOG, 1.0); // sustain-loop end
  modifyBreakpoint(4, 3.0,  0.0, rosic::ModBreakpoint::ANALOG, 1.0); // end
  setLoopMode(true);
  setLoopStartIndex(2);
  setLoopEndIndex(3);
}


