#!/bin/bash

echo "NOTE: this script relies on the blur binary to exist"

status=0
red=$(tput setaf 1)
green=$(tput setaf 2)
reset=$(tput sgr0)

# Create output directory if it doesn't exist
mkdir -p data_o

for image in im1 im2 im3 im4
do
    # Run blur on input image and save to temporary output
    ./blur 15 "data/$image.ppm" "data_o/blur_${image}.ppm"

    # Compare with reference output
    if ! cmp -s "data_o/${image}_seq.ppm" "data_o/blur_${image}.ppm"
    then
        echo "${red}Error: Incongruent output data detected when blurring image $image.ppm${reset}"
        status=1
    else
        echo "${green}Successfully verified $image.ppm${reset}"
    fi

    # Cleanup temporary output
    rm -f "data_o/blur_${image}.ppm"
done

exit $status