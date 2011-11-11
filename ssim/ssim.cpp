// ssim - simulator for distributed storage

#include <stdio.h>
#include <set>

#include "des.h"

using std::set;

#define K 15
    // this many packets per meta-packet
#define N 10
    // need this many to reconstruct the meta-packet
#define META_K   15
    // similar, meta-packets per file
#define META_N   10

#define HOSTS_PER_DAY   10.
#define HOST_LIFE_MEAN  100.*86400

SIMULATOR sim;

inline double drand() {
    return (double)rand()/(double)RAND_MAX;
}

// place-holder
double ran_exp(double mean) {
    // gsl_ran_exponential(mean);
    return (drand() + .5)*mean;
}

struct HOST;

set<HOST*> hosts;

struct HOST : public EVENT {
    double upload_bytes_sec;
    double download_bytes_sec;
    virtual void handle() {
        // the host has departed
        //
        set<HOST*>::iterator i = hosts.find(this);
        hosts.erase(i);
    }
};

struct HOST_ARRIVAL : public EVENT {
    virtual void handle() {
        HOST* h = new HOST;
        h->t = t + ran_exp(HOST_LIFE_MEAN);
        hosts.insert(h);
        sim.insert(h);
        t += ran_exp(86400./HOSTS_PER_DAY);
        sim.insert(this);
    }
};

struct REPORT_STATS : public EVENT {
    virtual void handle() {
        printf("%f: %d hosts\n", t, hosts.size());
        t += 86400;
        sim.insert(this);
    }
};

// a packet is associated with at most one host
//
struct PACKET {
    enum {DOWNLOADING, PRESENT, UPLOADING} state;
    HOST* host;
    bool present;       // present on server
    virtual void handle() {
        // transfer has finished
        //
    }
};

struct META_PACKET {
    vector<PACKET*> packets;
    int npackets_present;
};

struct DFILE : EVENT {
    vector<META_PACKET*> meta_packets;
    set<HOST*> unused_hosts;
        // hosts that don't have any packets of this file
    int nmeta_packets_present;
    virtual void handle() {
        for (int i=0; i<META_K; i++) {
            META_PACKET* mp = new META_PACKET;
            mp->npackets_present = K;
            meta_packets.push_back(mp);
            for (int j=0; j<K; j++) {
                PACKET* p = new PACKET;
                p->present = true;
                mp->packets.push_back(p);
            }
        }
    }
};

set<DFILE*> dfiles;

int main() {
    HOST_ARRIVAL *h = new HOST_ARRIVAL;
    h->t = 0;
    sim.insert(h);
    REPORT_STATS* r = new REPORT_STATS;
    r->t = 0;
    sim.insert(r);

    sim.simulate(200*86400);
}
