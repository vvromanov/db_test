CPUPROFILE_FREQUENCY=1000 LD_PRELOAD=/usr/lib64/libprofiler.so CPUPROFILE=/tmp/db_test_my ./db_test_my
pprof --nodecount=50 --maxdegree=50  ./db_test_my /tmp/db_test_my --pdf > db_test_my.pdf
