#include <bits/stdc++.h>
using namespace std;

double wtime() {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec + t.tv_nsec * 1e-9;
}

// Helper to parse perf_output.txt and store values
map<string, long long> read_perf_events(const string &filename) {
    ifstream fin(filename);
    string line;
    map<string, long long> events;

    while (getline(fin, line)) {
        stringstream ss(line);
        string value, event;
        getline(ss, value, ','); // first field = value
        getline(ss, event, ','); // skip second empty
        getline(ss, event, ','); // third field = event name

        try {
            long long val = stoll(value);
            // remove trailing '/' if present
if (!event.empty() && event.back() == '/')
    event.pop_back();

events[event] = val;

        } catch (...) {
            continue; // skip lines with "<not supported>"
        }
    }
    return events;
}

int main() {
    cpu_set_t set;
CPU_ZERO(&set);
CPU_SET(0, &set); // bind to core 0
if (sched_setaffinity(0, sizeof(set), &set) == -1) {
    perror("sched_setaffinity");
}
    int N =4096,b=64;
    int i,j,k,ii,jj,kk;
    vector<vector<double>> A(N, vector<double>(N, 0.0));
    vector<vector<double>> B(N, vector<double>(N, 1.0));
    vector<vector<double>> C(N, vector<double>(N, 1.0));

    double t0 = wtime();
    for(k=0; k<N; k=k+b)
        for(i=0; i<N; i=i+b){
            for (j=0; j < N; j=j+b)
                for(ii=i; ii<i+b; ii++)
                    for(jj=j; jj < j+b; jj++)
                        for(kk=k; kk < k+b; kk++)
                                A[ii][jj] += B[ii][kk] * C[kk][jj];
            }
   
    double t1 = wtime();

    // FLOPs
    double flops = 2.0 * N * N * N;
    double gflops = flops / (t1 - t0) / 1e9;

    double checksum = 0;
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            checksum += A[i][j];

    cout << "N = " << N << endl;
    cout << "Time = " << (t1 - t0) << " seconds\n";
    cout << "Checksum = " << checksum << endl;
    cout << "GFLOPS = " << gflops << endl;

    // Read perf events
    auto events = read_perf_events("perf_output.txt");

    if (!events.empty()) {
        if (events.count("cpu_core/L1-dcache-loads")) {
            long long l1_loads = events["cpu_core/L1-dcache-loads"];
            double AI_L1loads=flops/(l1_loads*64.0);
            cout << "L1 Loads = " << l1_loads << endl;
             cout << "AI  (L1 loads)="<<AI_L1loads<<endl;


        }
        if (events.count("cpu_core/L1-dcache-load-misses")) {
            long long l1_misses = events["cpu_core/L1-dcache-load-misses"];
            double bytes_L1 = l1_misses * 64.0;
            double AI_L1 = flops / bytes_L1;
            cout << "L1 Misses = " << l1_misses << endl;
            cout << "AI (L1 misses) = " << AI_L1 << " FLOPs/Byte\n";
        }
        if (events.count("cpu_core/L2_RQSTS.MISS")) {
                long long l2_misses = events["cpu_core/L2_RQSTS.MISS"];
                double bytes_L2 = l2_misses * 64.0;
                double AI_L2 = flops / bytes_L2;
                cout << "L2 Misses = " << l2_misses << endl;
                cout << "AI (L2) = " << AI_L2 << " FLOPs/Byte\n";
        
            }
        if (events.count("cpu_core/LLC-load-misses")) {
            long long l3_misses = events["cpu_core/LLC-load-misses"];
            double bytes_L3 = l3_misses * 64.0;
            double AI_L3 = flops / bytes_L3;
            cout << "L3 Misses = " << l3_misses << endl;
            cout << "AI (L3) = " << AI_L3 << " FLOPs/Byte\n";
        }

        if (events.count("cpu_core/cache-misses")) {
            long long cache_misses = events["cpu_core/cache-misses"];
            double bytes_from_dram = cache_misses * 64.0;
            double AI = flops / bytes_from_dram;
            cout << "Cache Misses = " << cache_misses << endl;
            cout << "Bytes from DRAM = " << bytes_from_dram / (1024 * 1024) << " MB\n";
            cout << "Arithmetic Intensity (AI) = " << AI << " FLOPs/Byte\n";
        }
        if (events.count("cpu_core/cycles") && events.count("cpu_core/instructions")) {
            long long cycles = events["cpu_core/cycles"];
            long long instr = events["cpu_core/instructions"];
            double IPC = (double)instr / cycles;
            cout << "Instructions = " << instr << endl;
            cout << "Cycles = " << cycles << endl;
            cout << "IPC = " << IPC << endl;
        }
    } else {
        cout << "No perf events found in perf_output.txt\n";
    }

    return 0;
}
