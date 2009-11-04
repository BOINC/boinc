// structure for tracking the recent mean and variance
// of a distribution that may change over time
//
// We maintain the mean/var in two ways:
// 1) over the entire history of samples, using the algorithm from
//    http://en.wikipedia.org/wiki/Algorithms_for_calculating_variance
// 2) as an exponentially-smoothed recent average
//
// When the number of samples is small we use 1).
// Then we switch to 2).

#define MIN_SAMPLES    50
    // after this many samples, use exponential average
#define SAMPLE_WEIGHT    0.005
    // new samples get this weight in exp avg
#define SAMPLE_LIMIT    10
    // cap samples at recent_mean*10

struct AVERAGE {
    int n;
    double mean;
    double sum_var;
        // sample variance is this divided by (n-1)
    double recent_mean;
    double recent_var;

    void update(double sample) {
        if (sample < 0) return;
        if (n > MIN_SAMPLES) {
            if (sample > recent_mean*SAMPLE_LIMIT) {
                sample = recent_mean*SAMPLE_LIMIT;
            }
        }
        n++;
        double delta = sample - mean;
        mean += delta/n;
        sum_var += delta*(sample-mean);

        if (n < MIN_SAMPLES) {
            recent_mean = mean;
            recent_var = sum_var/n;
        } else {
            // update recent averages
            delta = sample - recent_mean;
            recent_mean += SAMPLE_WEIGHT*delta;
            double d2 = delta*delta - recent_var;
            recent_var += SAMPLE_WEIGHT*d2;
        }
    }

    void clear() {
        n = 0;
        mean = 0;
        sum_var = 0;
        recent_mean = 0;
        recent_var = 0;
    }
};
