#!/bin/sh
# automate_test.sh :
#   * Runs tests for OS Project 5
#   * Outputs results for DISK_READS, DISK_WRITES, and PAGE_FAULT_NUM to csv file

# Usage
usage() {
echo "usage:  automate_test.sh [-can] [-r algorithm]"
echo "  -c:             run the custom test only and append results to the test_results.csv file"
echo "  -n:             run the fifo & rand tests only and print results to the test_results.csv file"
echo "  -a:             run fifo, rand and custom & print results to the test_results.csv file"
echo "  -r algorithm:   removes a specific algorithm's results from the test file"
}

# Variable definitions
declare -a PAGE_REPLACEMENTS=()
declare -a PAGE_NUMS=(12 36 60 84 108)
declare -a RATIO_FACTORS=(.25 .50 .75)
declare -a PROGRAMS=("sort" "focus" "scan")


touch test_results.csv

if [ $# -eq 0 ]; then
    usage
    exit 1
fi

while getopts 'canr:' flag; do
    case "${flag}" in
        c)
            declare -a PAGE_REPLACEMENTS=("custom")
            ;;
        a)
            declare -a PAGE_REPLACEMENTS=("rand" "fifo" "custom")
            if [ -f test_results.csv ]
            then
                rm test_results.csv
            fi
            touch test_results.csv
            printf "COMMAND, Page Replacement Algorithm, Num Pages, Num Frames, Program, Ratio of Pages to Frames, NUM_FAULTS, DISK_READS, DISK_WRITES \n" >> test_results.csv
            ;;
        n)
            declare -a PAGE_REPLACEMENTS=("rand" "fifo")
            if [ -f test_results.csv ]
            then
                rm test_results.csv
            fi
            touch test_results.csv
            printf "COMMAND, Page Replacement Algorithm, Num Pages, Num Frames, Program, Ratio of Pages to Frames, NUM_FAULTS, DISK_READS, DISK_WRITES \n" >> test_results.csv
            ;;
        r)
            remove_algo=${OPTARG}
            sed -i "/\b\($remove_algo\)\b/d" test_results.csv
            printf "removed $remove_algo results from test_results.csv! \n"
            exit 1
            ;;
        *)
            error "Unexpected option ${flag}"
            exit 1
        ;;
    esac
done

for pr in "${PAGE_REPLACEMENTS[@]}"
do
    echo "Testing $pr ... "
    for p in "${PROGRAMS[@]}"
    do
        printf "\t\t Program: \t $p \n "
        for rf in "${RATIO_FACTORS[@]}"
        do
            printf "\t\t Ratio Factor: \t $rf  \n "
            for pn in "${PAGE_NUMS[@]}"
            do
                printf "\t\t Page Number: \t $pn \n "
                fn_float=$(echo "$pn * $rf" | bc)
                fn_int=${fn_float%.*}
                printf "\t\t Frame Number: \t $fn_int \n "
                command="./virtmem $pn $fn_int $pr $p"
                printf "$command , $pr , $pn , $fn_int , $p , $rf , " >> test_results.csv
                ./virtmem $pn $fn_int $pr $p | grep "," >> test_results.csv
                printf "\t\t .........................  \n "
            done
        done
    done
done
