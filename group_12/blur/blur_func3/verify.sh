#!/bin/bash

echo "NOTE: this script relies on the blur binary to exist"

status=0
red=$(tput setaf 1)
green=$(tput setaf 2)
reset=$(tput sgr0)

# Check if blur binary exists and is executable
if [ ! -x "./blur" ]; then
    echo "${red}Error: blur binary not found or not executable${reset}"
    exit 1
fi

# Check if directories exist
if [ ! -d "data" ] || [ ! -d "data_o" ]; then
    echo "${red}Error: Required directories 'data' or 'data_o' missing${reset}"
    exit 1
fi

for image in im1 im2 im3 im4
do
    echo "Processing $image..."
    
    # Check if input file exists
    if [ ! -f "data/$image.ppm" ]; then
        echo "${red}Error: Input file data/$image.ppm not found${reset}"
        continue
    fi

    # Check if reference file exists
    if [ ! -f "data_o/${image}_seq.ppm" ]; then
        echo "${red}Error: Reference file data_o/${image}_seq.ppm not found${reset}"
        continue
    fi

    # Run blur and capture any errors
    if ! ./blur 15 "data/$image.ppm" "./data_o/blur_${image}test.ppm" 2>/dev/null; then
        echo "${red}Error: Blur operation failed for $image.ppm${reset}"
        status=1
        continue
    fi

    # Compare files and check file sizes
    if [ -f "./data_o/blur_${image}test.ppm" ]; then
        ref_size=$(stat -c%s "./data_o/${image}_seq.ppm")
        test_size=$(stat -c%s "./data_o/blur_${image}test.ppm")
        
        echo "Reference size: $ref_size bytes"
        echo "Output size: $test_size bytes"

        if ! cmp -s "./data_o/${image}_seq.ppm" "./data_o/blur_${image}test.ppm"; then
            echo "${red}Error: Output mismatch for $image.ppm${reset}"
            status=1
        else
            echo "${green}Success: Output matches for $image.ppm${reset}"
        fi
    else
        echo "${red}Error: Output file was not created for $image.ppm${reset}"
        status=1
    fi

    # Cleanup
    rm -f "./data_o/blur_${image}test.ppm"
    echo "----------------------------------------"
done

if [ $status -eq 0 ]; then
    echo "${green}All tests passed successfully!${reset}"
else
    echo "${red}Some tests failed. Check the output above for details.${reset}"
fi

exit $status