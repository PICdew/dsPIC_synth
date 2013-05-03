
#include "../h/fixed.h"

fixed fl2fp(float val){
    return (fixed)(val * FIXED_SCALE);
}

float fp2fl(fixed val){
    return((float)val)/FIXED_SCALE;
}