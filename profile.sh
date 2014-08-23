CPUPROFILE_FREQUENCY=1000 LD_PRELOAD=/usr/lib64/libprofiler.so CPUPROFILE=/tmp/db_test_pq ./db_test_pq
pprof --nodecount=50 --maxdegree=50  ./db_test_pq /tmp/db_test_pq --pdf > db_test_pq.pdf
