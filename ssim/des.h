#include <vector>
#include <algorithm>

using std::vector;

struct EVENT {
    double t;
    virtual void handle(){}
};

bool compare(EVENT* e1, EVENT* e2) {
    return (e1->t > e2->t);
}

struct SIMULATOR {
    vector<EVENT*> events;
    void insert(EVENT* e) {
        events.push_back(e);
        push_heap(events.begin(), events.end(), compare);
    }
    void simulate(double dur) {
        while (events.size()) {
            EVENT* e = events.front();
            pop_heap(events.begin(), events.end(), compare);
            events.pop_back();
            if (e->t > dur) break;
            e->handle();
        }
    }
};
