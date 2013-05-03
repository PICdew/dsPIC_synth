
#ifndef FIXED_H
#define	FIXED_H

typedef long fixed;

#define FIXED_SHIFT              8
#define FIXED_SCALE              (1 << FIXED_SHIFT)
#define i2fp(a)                  ((a) << FIXED_SHIFT)
#define fp2i(a)                  ((a) >> FIXED_SHIFT)
#define fp_add(a,b)              ((a)+(b))
#define fp_sub(a,b)              ((a)-(b))
#define FP_FRACMASK              FIXED_SCALE-1;
#define fp_mul(x,y)              ((fixed)(((long)(x) * (long)(y)) >> FIXED_SHIFT))
#define mul16(x,y)              ((fixed)(((long long)(x) * (long long)(y)) >> 16))
#define mul13(x,y)              ((fixed)(((long)(x) * (long)(y)) >> 13))
#define FP_ONE                   (1 << FIXED_SHIFT)

extern fixed fl2fp(float val);
extern float fp2fl(fixed val);

#endif	/* FIXED_H */

