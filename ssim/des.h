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
    double now;
    void insert(EVENT* e) {
        events.push_back(e);
        push_heap(events.begin(), events.end(), compare);
    }
    void remove(EVENT* e) {
        vector<EVENT*>::iterator i;
        for (i=events.begin(); i!=events.end(); i++) {
            if (*i == e) {
                events.erase(i);
                make_heap(events.begin(), events.end(), compare);
                break;
            }
        }
    }
    void simulate(double dur) {
        while (events.size()) {
            EVENT* e = events.front();
            pop_heap(events.begin(), events.end(), compare);
            events.pop_back();
            now = e->t;
            if (now > dur) break;
            e->handle();
        }
    }
};
