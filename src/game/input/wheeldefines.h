#ifndef WHEEL_DEFINES
#define WHEEL_DEFINES

#ifdef RAD_FORCE_FEEDBACK
enum eForceTypes
{
    CONSTANT_FORCE, 
    RAMP_FORCE,
    SQUARE, 
    SINE,
    TRIANGLE,
    SAWTOOTH_UP, 
    SAWTOOTH_DOWN, 
    SPRING, 
    DAMPER,
    INERTIA, 
    FRICTION, 
    CUSTOM_FORCE 
};
#endif

#endif