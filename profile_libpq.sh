CPUPROFILE_FREQUENCY=1000 LD_PRELOAD=/usr/lib64/libprofiler.so CPUPROFILE=/tmp/db_test_libpq ./db_test_libpq
pprof --nodecount=50 --maxdegree=50  ./db_test_libpq /tmp/db_test_libpq --pdf > db_test_libpq.pdf
