
// CMC here
// separate different compilations of whetstone.cpp which will utilize
// various ARM fp features ie neon, vfp, or "normal"
#ifdef ANDROID
#ifdef ANDROID_NEON
   // add CXXFLAGS/CFLAGS for gcc:  -DANDROID_NEON -mfloat-abi=softfp -mfpu=neon
   #include <arm_neon.h>
#endif // ANDROID_NEON

namespace android_neon {
    int whetstone(double& flops, double& cpu_time, double min_cpu_time);
}

namespace android_vfp {
    int whetstone(double& flops, double& cpu_time, double min_cpu_time);
}

#endif // ANDROID

