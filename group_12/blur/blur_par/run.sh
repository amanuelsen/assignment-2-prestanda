#!/bin/bash

echo "Making..."
make clean > /dev/null 2>&1
make > /dev/null 2>&1

# Create output directory if it doesn't exist
mkdir -p data_o

images=("im1.ppm" "im2.ppm" "im3.ppm" "im4.ppm")
threads=(1 2 4 8)

# Run performance tests for different thread counts
for img in "${images[@]}"; do
    for thread in "${threads[@]}"; do
        output="data_o/blur_${img%.*}_par.ppm"
        echo "Running blur on $img with $thread thread(s)..."
        /usr/bin/time -v ./blur_par 15 "data/$img" "$output" $thread 2>&1 | grep -E "Percent of CPU this job got|Elapsed \(wall clock\)|Maximum resident set size|Major \(requiring I/O\) page faults|Minor \(reclaiming a frame\) page faults|Voluntary context switches|Involuntary context switches|File system inputs|File system outputs|Page size|Exit status"
        echo "-----------------------------------------"
    done
done

# Run valgrind tests
echo "Running valgrind (callgrind) on im1.ppm with different thread counts..."
for thread in "${threads[@]}"; do
    echo "Testing with $thread thread(s)..."
    valgrind --tool=callgrind --callgrind-out-file=callgrind.out.$thread ./blur_par 15 "data/im1.ppm" "data_o/blur_im1_par.ppm" $thread
    echo "-----------------------------------------"
done