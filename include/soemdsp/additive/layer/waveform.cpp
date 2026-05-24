#include "waveform.hpp"

namespace soemdsp::additive {

static void onShapeChangeCALLBACKHELPER(void* context) {
    auto* self = static_cast<Waveform*>(context);

    switch (convert::double_to_enum<Waveform::Shape>(self->shape_)) {
    case Waveform::Shape::SawSquare:
    case Waveform::Shape::DoubleSaw:
    case Waveform::Shape::MultiSaw:
    case Waveform::Shape::RoundedSquareDoubleSaw:
    case Waveform::Shape::SquareDoubleSaw:
    case Waveform::Shape::PulseCenter:
    case Waveform::Shape::PulseLeft:
    case Waveform::Shape::PulseRight:
    case Waveform::Shape::MultiPulse1:
    case Waveform::Shape::MultiPulse2:
    case Waveform::Shape::Square:
    case Waveform::Shape::TriSaw:
    case Waveform::Shape::RectifiedSine:
    case Waveform::Shape::RectifiedSineTri:
    case Waveform::Shape::Organ:
        self->shapeChanged(1, self->hm_->currentSize_);
        break;
    default:
        break;
    }
}

//parameters
Waveform::Waveform() {
    mod_.resize(convert::enumcount_to_sizet<Par>());
    mod_[convert::enum_to_sizet(Par::shape)].callback = &onShapeChangeCALLBACKHELPER;
    mod_[convert::enum_to_sizet(Par::modA)].callback  = &onShapeChangeCALLBACKHELPER;
}

void Waveform::shapeChanged(int startIndex, int endIndex) { //mod_, shape
    counter_ = 1;
    shape_   = round(mod_[convert::enum_to_sizet(Par::shape)]);

    for (n_ = startIndex; n_ < hm_->lastHarmonicToRender_; ++n_) {
        h_ = static_cast<double>(n_);

        switch (convert::double_to_enum<Shape>(shape_)) {
        case Shape::Sine:
            sine();
            break;
        case Shape::Sawtooth:
            sawtooth();
            break;
        case Shape::SawSquare:
            sawsquare(1.0 - mod_[convert::enum_to_sizet(Par::modA)]);
            break;
        case Shape::DoubleSaw:
            doublesaw(mod_[convert::enum_to_sizet(Par::modA)]);
            break;
        case Shape::MultiSaw:
            multisaw(mod_[convert::enum_to_sizet(Par::modA)]);
            break;
        case Shape::RoundedSquareDoubleSaw:
            roundedSquareDoubleSaw(mod_[convert::enum_to_sizet(Par::modA)]);
            break;
        case Shape::SquareDoubleSaw:
            squareDoubleSaw(mod_[convert::enum_to_sizet(Par::modA)]);
            break;
        case Shape::PulseCenter:
            pulseCenter(mod_[convert::enum_to_sizet(Par::modA)]);
            break;
        case Shape::PulseLeft:
            pulseLeft(mod_[convert::enum_to_sizet(Par::modA)]);
            break;
        case Shape::PulseRight:
            pulseRight(mod_[convert::enum_to_sizet(Par::modA)]);
            break;
        case Shape::MultiPulse1:
            multipulse1(mod_[convert::enum_to_sizet(Par::modA)]);
            break;
        case Shape::MultiPulse2:
            multipulse2(mod_[convert::enum_to_sizet(Par::modA)]);
            break;
        case Shape::Square:
            square(mod_[convert::enum_to_sizet(Par::modA)]);
            break;
        case Shape::TriSaw:
            trisaw(mod_[convert::enum_to_sizet(Par::modA)]);
            break;
        case Shape::Triangle:
            triangle();
            break;
        case Shape::RectifiedSine:
            rectifiedSine();
            break;
        case Shape::RectifiedSineTri:
            rectifiedSineTri(mod_[convert::enum_to_sizet(Par::modA)]);
            break;
        case Shape::Organ:
            organ(static_cast<int>(math::map0to1<double>(mod_[convert::enum_to_sizet(Par::modA)], 2.0, 13.0)));
            break;
            debug::FAIL();
        }
        amplitude_[n_] = a_;
        phase_[n_]     = p_;
    }
}
void Waveform::sine(double select) {
    size_t sel = static_cast<size_t>(99.0 * select + 1.0); //0 to 1 -> 1 to 100
    a_         = n_ == sel ? 1.0 : 0.0;
    p_         = 0.0;
}
void Waveform::sawtooth() {
    a_ = 1.0 / h_;
    p_ = math::isOdd(n_) ? 0.5 : 0.0;
}

void Waveform::sawsquare(double mix) {
    mix = 1.0 - mix;
    a_  = math::isOdd(n_) ? 1.0 / h_ : 1.0 / h_ * mix;
    p_  = 0.0;
}

void Waveform::rectifiedSine() {
    a_ = 1.0 / (h_ * h_);
    p_ = math::isOdd(n_) ? 0.25 : 0.75;
}

void Waveform::rectifiedSineTri(double mod) {
    const double hh{ h_ * h_ };
    a_ = math::sin(hh * 0.25 + mod) / hh;
    p_ = 0.25;
}

void Waveform::roundedSquareDoubleSaw(double mod) {
    const double hh{ h_ * h_ };
    mod = 0.125 + 0.75 * mod; //0 to 1 -> 0.125 to .875
    a_  = math::sin(hh * 0.25 + mod) / hh;
    p_  = 0.0;
}

void Waveform::squareDoubleSaw(double mod) {
    double hh{ h_ * h_ };
    mod = 0.125 + 0.75 * mod; //0 to 1 -> 0.125 to .875
    a_  = math::sin(hh * 0.25 + mod) / h_;
    p_  = 0.0;
}

void Waveform::doublesaw(double pwm) {
    pwm *= 0.5;
    a_ = math::cos(h_ * pwm) / h_;
    p_ = 0.0;
}

void Waveform::square(double phaseRotate) {
    phaseRotate *= 0.5;
    a_ = n_ % 2 == 1 ? 1.0 / h_ : 0.0;
    p_ = phaseRotate;
}

void Waveform::pulseCenter(double pwm) {
    pwm *= 0.5;
    a_ = math::sin(h_ * pwm) / h_;
    p_ = 0.25;
}

void Waveform::pulseLeft(double pwm) {
    pwm *= 0.5;
    a_ = math::sin(h_ * pwm) / h_;
    p_ = h_ * pwm + 0.25;
}

void Waveform::pulseRight(double pwm) {
    pwm *= 0.5;
    a_ = math::sin(h_ * pwm) / h_;
    p_ = h_ * -pwm + 0.25;
}

void Waveform::multipulse1(double pwm) {
    const double hh{ h_ * h_ };
    pwm *= 0.5;
    a_ = math::cos(hh * .45 + pwm) / h_;
    p_ = 0.0;
}

void Waveform::multipulse2(double pwm) {
    const double hh{ h_ * h_ };
    pwm *= 0.5;
    a_ = math::cos(hh * .475 + pwm) / h_;
    p_ = 0.0;
}

void Waveform::multisaw(double pwm) {
    const double hh{ h_ * h_ };
    pwm *= 0.5;
    a_ = math::cos(hh * 0.3 + pwm) / h_;
    p_ = 0.0;
}

void Waveform::trisaw(double pwm) {
    pwm = std::clamp<double>(pwm, 0.001, .999);
    a_  = math::sin(0.5 * h_ * pwm) / (pwm * (1.0 - pwm) * h_ * h_) * 0.2;
    p_  = 0.0;
}

void Waveform::triangle() {
    a_ = math::isOdd(n_) ? a_ = 1.0 / (h_ * h_) : 0.0;
    p_ = n_ % 4 == 1 ? 0.0 : 0.5;
}

void Waveform::organ(int octaves) {
    a_ = 0.0;
    if (n_ == static_cast<size_t>(counter_)) {
        a_ = 1.0 / h_;
        counter_ *= octaves;
    }
    p_ = 0.0;
}
} //namespace soemdsp::additive
