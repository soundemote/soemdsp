#pragma once

#include <type_traits>
#include <stdexcept>
#include <functional>

/*
This class allows for pointing values to other values and replaces basic types such as double and int. The reason for having seperate write (w) and read (r) is so that you can point to another value
and if you never do, you can write directly to the var class variable. If you do point to another variable, you cannot then write to that other variable, it would be read only.
*/
template<typename value_t>
struct Wire {
    static_assert(
      std::is_same<value_t, double>::value || std::is_same<value_t, int>::value || std::is_same<value_t, float>::value || std::is_same<value_t, bool>::value,
      "Wire only supports double, int, float, and bool");
 
    Wire() {}
    Wire(value_t initialVaue) // t to var
      : w(initialVaue) { 
    }
    Wire(value_t initialValue, std::function<void()> changedFunc)
      : w(initialValue)
      , changed_(changedFunc) {}
    Wire(std::function<void()> changedFunc)
      : changed_(changedFunc) {}

    std::function<void()> changed_ = []() {};
    void operator()() {
        changed_();
    }    

    // set location of initial value
    void pointTo(value_t* master) {
        r = master;
    }
    // set location of initial value
    void pointTo(Wire<value_t>& master) {
        r  = master.r;
    }
    // re-point variable to itself / undo connections
    void disconnect() {
        r  = &w;
    }
    
    // read
    operator value_t() const { return *r; } // copies value from var to t // IN CASE OF CRASH: check that you aren't trying to use an un-constructed Wire to instantiate another Wire!!!    

    // write
    Wire& operator=(const value_t& rhs) {  // copies value from t to var
        w = rhs;
        return *this;
    }
    Wire<value_t>& operator=(const Wire<value_t>& rhs) { // copies value from var to var
        w = *rhs.r;
        return *this;
    }

    Wire<value_t>& operator+=(value_t increment) {
        w += increment;
        return *this;
    }
    Wire<value_t>& operator-=(value_t increment) {
        w -= increment;
        return *this;
    }
    Wire<value_t>& operator*=(value_t increment) {
        w *= increment;
        return *this;
    }
    Wire<value_t>& operator/=(value_t increment) {
        w /= increment;
        return *this;
    }

    value_t w{}; // WRITE
    value_t* r{ &w }; // READ
};

template<typename value_t>
struct WireMod : public Wire<value_t> {
    static_assert(
      std::is_same<value_t, double>::value || std::is_same<value_t, int>::value || std::is_same<value_t, float>::value || std::is_same<value_t, bool>::value,
      "Wire only supports double, int, float, and bool");

    WireMod() {}
    WireMod(value_t initialValue)
      : Wire<value_t>(initialValue) {}

    // Set location of initial value from another WireMod.
    void pointTo(value_t* master) {
        this->r = master;
    }
    void pointTo(WireMod<value_t>& master) {
        this->r = master.r;
        mr      = master.mr;
    }
    // Set location of modulation value from a pointer.
    void pointModTo(value_t* master) {
        mr = master;
    }
    // Revert both the base and modulation pointers to their local storage.
    void disconnect() {
        this->r = &this->w;
        mr      = &mw;
    }

    // Implicit conversion: returns the sum of the base value and the modulation.
    operator value_t() const {
        return *this->r + *mr;
    }

    value_t mw{}; // MOD WRITE (local modulation storage)
    value_t* mr{ &mw }; // MOD READ (pointer to modulation value)
};
