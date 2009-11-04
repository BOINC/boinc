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
    double n;       // double to avoid integer overflow
    double weight;
        // if n < MIN_SAMPLES, sum of weights
        // else recent average weight
    double mean;
        // if n < MIN_SAMPLES, weighted sum of samples
        // else recent average
    double var;
        // if n < MIN_SAMPLES, weighted sum of vars
        // else recent var

    void update(double sample, double w) {
        double delta;
        if (sample < 0) return;
        if (n > MIN_SAMPLES) {
            if (sample > mean*SAMPLE_LIMIT) {
                sample = mean*SAMPLE_LIMIT;
            }
        }
        n++;

        if (n < MIN_SAMPLES) {
            weight += w;
            mean += w*sample;
            delta = (sample - mean/weight);
            var += delta*delta*w;
        } else if (n == MIN_SAMPLES) {
            mean /= weight;
            var /= weight;
            weight /= n;
        } else {
            // update recent averages
            if (w > weight*10) w = weight*10;
            double rel_weight = w/weight;
            double sample_weight = SAMPLE_WEIGHT * rel_weight;

            delta = sample - mean;
            mean += sample_weight*delta;

            double d2 = delta*delta - var;
            var += sample_weight*d2;

            weight += SAMPLE_WEIGHT*(w - weight);
        }
    }
    double get_mean() {
        if (n < MIN_SAMPLES) {
            return mean/weight;
        } else {
            return mean;
        }
    }

    void clear() {
        n = 0;
        weight = 0;
        mean = 0;
        var = 0;
    }
};
