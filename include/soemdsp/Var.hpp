#pragma once

#include <type_traits>
#include <stdexcept>

/*
This class allows for pointing values to other values and replaces basic types such as double and int. The reason for
variable write (w) and variable read (r) is so that you can point to another value and if you never do, you can write
directly to the var class variable. If you do point to another variable, you cannot then write to that other variable,
it would be read only.
*/
template<typename t>
class Wire {
    static_assert(
      std::is_same<t, double>::value || std::is_same<t, int>::value || std::is_same<t, float>::value ||
        std::is_same<t, bool>::value,
      "Wire only supports double, int, float, and bool");
  public:    
    Wire() {}
    Wire(t initialVaue)
      : w(initialVaue) {
    } // t to var

    // get initial + modulation value
    t getWithMod() {
        return *r + *mr;
    }
    // get modulation value only
    t getOnlyMod() {
        return *mr;
    }

    // set location of initial value
    void pointTo(t* master) {
        r = master;
    }
    // set location of initial value
    void pointTo(Wire<t>& master) {
        r  = master.r;
    }
    // set location of modulation value
    void pointModTo(Wire<t>& master) {
        mr = master.mr;
    }
    // set location of modulation value
    void pointModTo(t* master) {
        mr = master;
    }
    // set location of initial and mod value
    void pointAllTo(Wire<t>& master) {
        r  = master.r;
        mr = master.mr;
    }
    // re-point variable to itself / undo connections
    void disconnect() {
        r  = &w;
        mr = &mw;
    }
    
    // read
    operator t() const { return *r; } // copies value from var to t
    // IN CASE OF CRASH: check that you aren't trying to use an un-constructed Wire to instantiate another Wire!!!

    // write
    Wire& operator=(const t& rhs) {  // copies value from t to var
        w = rhs;
        return *this;
    }
    Wire<t>& operator=(const Wire<t>& rhs) { // copies value from var to var
        w = *rhs.r;
        return *this;
    }

    Wire<t>& operator+=(t increment) {
        w += increment;
        return *this;
    }
    Wire<t>& operator-=(t increment) {
        w -= increment;
        return *this;
    }
    Wire<t>& operator*=(t increment) {
        w *= increment;
        return *this;
    }
    Wire<t>& operator/=(t increment) {
        w /= increment;
        return *this;
    }

    t w{};        // WRITE
    t* r{ &w };   // READ
    t mw{};       // modulation WRITE
    t* mr{ &mw }; // modulation READ
};
