// structure for tracking the recent mean
// of a distribution that may change over time
//

#include <math.h>

#define MIN_SAMPLES    50000
    // after this many samples, use exponential average
#define SAMPLE_WEIGHT    0.005
    // new samples get this weight in exp avg
#define SAMPLE_LIMIT    20
    // cap samples at recent_mean*limit

struct AVERAGE {
    double n;       // double to avoid integer overflow
    double sum;
    double exp_avg;

    void update(double sample) {
        double delta, limit;
        if (sample < 0) return;
        if (n > MIN_SAMPLES) {
            if (sample > exp_avg*SAMPLE_LIMIT) {
                printf(" truncating sample:  %.0fG exp_avg %.0fG\n",
                    sample/1e9, exp_avg/1e9
                );
                sample = exp_avg*SAMPLE_LIMIT;
            }
        } else {
            double x = sum/n;
            if (sample > x*SAMPLE_LIMIT) {
                printf(" truncating sample: %.0fG avg %.0fG\n",
                    sample/1e9, x/1e9
                );
                sample = x*SAMPLE_LIMIT;
            }
        }
        n++;

        sum += sample;
        if (n < MIN_SAMPLES) {
            exp_avg = sum/n;
        } else {
            delta = sample - exp_avg;
            exp_avg += SAMPLE_WEIGHT*delta;
        }
    }
    inline double get_mean() {
        return exp_avg;
    }

    void clear() {
        n = 0;
        sum = 0;
        exp_avg = 0;
    }
};
