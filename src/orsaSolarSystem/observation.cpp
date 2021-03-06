#include <orsaSolarSystem/observation.h>

#include <orsaSolarSystem/obleq.h>

using namespace orsa;
using namespace orsaSolarSystem;

// based on an email by Alan Harris on MPML (Jan 16, 2007)
// http://tech.groups.yahoo.com/group/mpml/message/18665
double orsaSolarSystem::MPC_band_correction(const char band) {
    double c=0.0; // correction
    switch (band) {
        case 'U': c=-1.10; break;
        case ' ': c=-0.80; break; // empty = B
        case 'B': c=-0.80; break;
        case 'V': c= 0.00; break;
        case 'g': c=+0.01; break;
        case 'r': c=+0.23; break;
        case 'R': c=+0.40; break;
        case 'C': c=+0.40; break; // C = clear = R
        case 'W': c=+0.40; break;
        case 'z': c=+0.70; break;
        case 'I': c=+0.80; break;
        case 'J': c=+1.20; break;
        case 'i': c=+1.22; break;
        default:
            ORSA_DEBUG("band not found: [%c]",band);
    }
    // ORSA_DEBUG("band: [%c]   correction: %g",band,c);
    return c;
}

orsa::Vector orsaSolarSystem::observationDirection(const orsaSolarSystem::OpticalObservation * obs) {
    double s_ra,  c_ra;
    double s_dec, c_dec;
    orsa::sincos(obs->ra,  &s_ra,  &c_ra);
    orsa::sincos(obs->dec, &s_dec, &c_dec);
    return orsaSolarSystem::equatorialToEcliptic() *
        orsa::Vector(c_dec*c_ra,
                     c_dec*s_ra,
                     s_dec);
}
