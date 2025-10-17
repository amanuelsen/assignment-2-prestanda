#!/bin/bash

echo "Making..."
make clean > /dev/null 2>&1
make > /dev/null 2>&1

images=("im1.ppm" "im2.ppm" "im3.ppm" "im4.ppm")

for img in "${images[@]}"; do
    output="output_${img}"
    echo "Running blur on $img..."
    /usr/bin/time -v ./blur 15 "data/$img" "$output" 2>&1 | grep -E "Percent of CPU this job got|Elapsed \(wall clock\)|Maximum resident set size|Major \(requiring I/O\) page faults|Minor \(reclaiming a frame\) page faults|Voluntary context switches|Involuntary context switches|File system inputs|File system outputs|Page size|Exit status"
    echo "-----------------------------------------"
done

# Run valgrind only after all timing is done, for all images

echo "Running valgrind (callgrind) on im1"
valgrind --tool=callgrind ./blur 15 "data/im1.ppm" "output_im1.ppm"
echo "-----------------------------------------"