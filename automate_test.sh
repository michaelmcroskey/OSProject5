#!/bin/sh
# automate_test.sh :
#   * Runs tests for OS Project 5
#   * Outputs results for DISK_READS, DISK_WRITES, and PAGE_FAULT_NUM to csv file

# Variable definitions
declare -a PAGE_REPLACEMENTS=("rand" "fifo" "custom")
declare -a PAGE_NUMS=(12 36 60 84 108)
declare -a RATIO_FACTORS=(.25 .50 .75)
declare -a PROGRAMS=("sort" "focus" "scan")


touch test_results.csv
printf "COMMAND, Page Replacement Algorithm, Num Pages, Num Frames, Program, Ratio of Pages to Frames, NUM_FAULTS, DISK_READS, DISK_WRITES \n" >> test_results.csv


for pr in "${PAGE_REPLACEMENTS[@]}"
do
    for p in "${PROGRAMS[@]}"
    do
        for pn in "${PAGE_NUMS[@]}"
        do
            for rf in "${RATIO_FACTORS[@]}"
            do
                fn_float=$(echo "$pn * $rf" | bc)
                fn_int=${fn_float%.*}
                command="./virtmem $pn $fn_int $pr $p"
                printf "$command , $pr , $pn , $fn_int , $p , $rf , "
                ./virtmem $pn $fn_int $pr $p | grep "," >> test_results.csv
                printf " , \n" >> test_results.csv
                exit
            done
        done
    done
done
