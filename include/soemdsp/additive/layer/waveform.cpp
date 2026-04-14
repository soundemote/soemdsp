#include "waveform.hpp"

namespace soemdsp::additive {
// parameters

Waveform::Waveform() {
    mod_.resize(Enum::count<Par>());

    mod_[Enum::sizeT(Par::shape)].changed_ = [this]() {
        shapeChanged(1, hm_->currentSize_);
    };

    mod_[Enum::sizeT(Par::modA)].changed_  = [this]() {
        switch (Enum::get<Shape>(shape_)) {
        case Shape::SawSquare:
        case Shape::DoubleSaw:
        case Shape::MultiSaw:
        case Shape::RoundedSquareDoubleSaw:
        case Shape::SquareDoubleSaw:
        case Shape::PulseCenter:
        case Shape::PulseLeft:
        case Shape::PulseRight:
        case Shape::MultiPulse1:
        case Shape::MultiPulse2:
        case Shape::Square:
        case Shape::TriSaw:
        case Shape::RectifiedSine:
        case Shape::RectifiedSineTri:
        case Shape::Organ:
            shapeChanged(1, hm_->currentSize_);
            break;
        default:
            break;
        }
    };
}

void Waveform::shapeChanged(int startIndex, int endIndex) { // mod_, shape
    counter_ = 1;
    shape_   = round(mod_[Enum::sizeT(Par::shape)]);

    for (n_ = startIndex; n_ < hm_->lastHarmonicToRender_; ++n_) {
        h_ = toDouble(n_);

        switch (Enum::get<Shape>(shape_)) {
        case Shape::Sine:
            sine();
            break;
        case Shape::Sawtooth:
            sawtooth();
            break;
        case Shape::SawSquare:
            sawsquare(1.0 - mod_[Enum::sizeT(Par::modA)]);
            break;
        case Shape::DoubleSaw:
            doublesaw(mod_[Enum::sizeT(Par::modA)]);
            break;
        case Shape::MultiSaw:
            multisaw(mod_[Enum::sizeT(Par::modA)]);
            break;
        case Shape::RoundedSquareDoubleSaw:
            roundedSquareDoubleSaw(mod_[Enum::sizeT(Par::modA)]);
            break;
        case Shape::SquareDoubleSaw:
            squareDoubleSaw(mod_[Enum::sizeT(Par::modA)]);
            break;
        case Shape::PulseCenter:
            pulseCenter(mod_[Enum::sizeT(Par::modA)]);
            break;
        case Shape::PulseLeft:
            pulseLeft(mod_[Enum::sizeT(Par::modA)]);
            break;
        case Shape::PulseRight:
            pulseRight(mod_[Enum::sizeT(Par::modA)]);
            break;
        case Shape::MultiPulse1:
            multipulse1(mod_[Enum::sizeT(Par::modA)]);
            break;
        case Shape::MultiPulse2:
            multipulse2(mod_[Enum::sizeT(Par::modA)]);
            break;
        case Shape::Square:
            square(mod_[Enum::sizeT(Par::modA)]);
            break;
        case Shape::TriSaw:
            trisaw(mod_[Enum::sizeT(Par::modA)]);
            break;
        case Shape::Triangle:
            triangle();
            break;
        case Shape::RectifiedSine:
            rectifiedSine();
            break;
        case Shape::RectifiedSineTri:
            rectifiedSineTri(mod_[Enum::sizeT(Par::modA)]);
            break;
        case Shape::Organ:
            organ(toInt(map0to1<double>(mod_[Enum::sizeT(Par::modA)], 2.0, 13.0)));
            break;
            SE_SWITCH_STATEMENT_FAILURE
        }
        amplitude_[n_] = a_;
        phase_[n_]     = p_;
    }

}
void Waveform::sine(double select) {
    size_t sel = toSizeT(99.0 * select + 1.0); // 0 to 1 -> 1 to 100
    a_ = n_ == sel ? 1.0 : 0.0;
    p_ = 0.0;
}
void Waveform::sawtooth() {
    a_ = 1.0 / h_;
    p_ = isOdd(n_) ? 0.5 : 0.0;
}

void Waveform::sawsquare(double mix) {
    mix = 1.0 - mix;
    a_  = isOdd(n_) ? 1.0 / h_ : 1.0 / h_ * mix;
    p_  = 0.0;
}

void Waveform::rectifiedSine() {
    a_ = 1.0 / (h_ * h_);
    p_ = isOdd(n_) ? 0.25 : 0.75;
}

void Waveform::rectifiedSineTri(double mod) {
    const double hh{ h_ * h_ };
    a_ = soemdsp::sin(hh * 0.25 + mod) / hh;
    p_ = 0.25;
}

void Waveform::roundedSquareDoubleSaw(double mod) {
    const double hh{ h_ * h_ };
    mod = 0.125 + 0.75 * mod; // 0 to 1 -> 0.125 to .875
    a_  = soemdsp::sin(hh * 0.25 + mod) / hh;
    p_  = 0.0;
}

void Waveform::squareDoubleSaw(double mod) {
    double hh{ h_ * h_ };
    mod = 0.125 + 0.75 * mod; // 0 to 1 -> 0.125 to .875
    a_  = soemdsp::sin(hh * 0.25 + mod) / h_;
    p_  = 0.0;
}

void Waveform::doublesaw(double pwm) {
    pwm *= 0.5;
    a_ = soemdsp::cos(h_ * pwm) / h_;
    p_ = 0.0;
}

void Waveform::square(double phaseRotate) {
    phaseRotate *= 0.5;
    a_ = n_ % 2 == 1 ? 1.0 / h_ : 0.0;
    p_ = phaseRotate;
}

void Waveform::pulseCenter(double pwm) {
    pwm *= 0.5;
    a_ = soemdsp::sin(h_ * pwm) / h_;
    p_ = 0.25;
}

void Waveform::pulseLeft(double pwm) {
    pwm *= 0.5;
    a_ = soemdsp::sin(h_ * pwm) / h_;
    p_ = h_ * pwm + 0.25;
}

void Waveform::pulseRight(double pwm) {
    pwm *= 0.5;
    a_ = soemdsp::sin(h_ * pwm) / h_;
    p_ = h_ * -pwm + 0.25;
}

void Waveform::multipulse1(double pwm) {
    const double hh{ h_ * h_ };
    pwm *= 0.5;   
    a_ = soemdsp::cos(hh * .45 + pwm) / h_;
    p_ = 0.0;
}

void Waveform::multipulse2(double pwm) {
    const double hh{ h_ * h_ };
    pwm *= 0.5;    
    a_ = soemdsp::cos(hh * .475 + pwm) / h_;
    p_ = 0.0;
}

void Waveform::multisaw(double pwm) {
    const double hh{ h_ * h_ };
    pwm *= 0.5;    
    a_ = soemdsp::cos(hh * 0.3 + pwm) / h_;
    p_ = 0.0;
}

void Waveform::trisaw(double pwm) {
    pwm = std::clamp<double>(pwm, 0.001, .999);
    a_  = soemdsp::sin(0.5 * h_ * pwm) / (pwm * (1.0 - pwm) * h_ * h_) * 0.2;
    p_  = 0.0;
}

void Waveform::triangle() {
    a_ = isOdd(n_) ? a_ = 1.0 / (h_ * h_) : 0.0;
    p_ = n_ % 4 == 1 ? 0.0 : 0.5;
}

void Waveform::organ(int octaves) {
    a_ = 0.0;
    if (n_ == toSizeT(counter_)) {
        a_ = 1.0 / h_;
        counter_ *= octaves;
    }
    p_ = 0.0;
}
} // namespace soemdsp::additive
