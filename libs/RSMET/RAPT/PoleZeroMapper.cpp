#include "PoleZeroMapper.h"
#include "../RSLib/Polynomial.h"

namespace RAPT {

    using RSLib::rsPolynomial;

/*
ToDo:
-Implement also the discrete versions of the frequency transformations
(Constantinides formulas) -Implement this:
https://vicanek.de/articles/BiquadFits.pdf and maybe MZTi, too -Maybe use as
matching frequencies, 0, fs/2, fc, fu, fu/2 where fu is the lower bandedge freq.
  If there is no such thing, maybe use fu = fc/2. Avoid using the upper bandedge
because that may actually go above fs/2 -Maybe we can also derive formulas that
match the magnitude only at 4 frequencies and match the phase at fc. ...or maybe
match magnitude at 3 frequencies and phase at fc and fs/2 -Maybe rename
bilinearAnalogToDigital to bilinear_s2z - or have a seperate class
 TimeDiscretizaionMapper (or something) and the methods can be named bilinear
(for s -> z) and invBilinear (for z -> s) -Implement also Impulse-Invariant,
(improved) Matched-Z, 3-point Magnitude Match, 5-point Magnitude Match, etc.
-Implement Phase-Invariant-Method (PIM) and Magnitude-Invariant-Method (MIM) to
map poles and zeros from s-plane to z-plane (and maybe back, if possible), see
https://soar.wichita.edu/handle/10057/1564


-Many links for filter design formulas are collected here
 https://www.kvraudio.com/forum/viewtopic.php?f=33&t=569113


*/
} // namespace RAPT
