datasets="asl-bu blocks"
frequencies="0.6 0.5 0.4 0.3 0.2"
growth_rates="0 1 2"
maximal_sizes="0 4"
output="classification_2.csv"
log_output="classification_2.log"
for dataset in $datasets
do
    for fmin in $frequencies
    do
        for gmin in $growth_rates
        do 
            for smax in $maximal_sizes
            do
                /home/yann/timeout/timeout -t 3600 -m 12000000 python3 classification.py ../datasets/$dataset --fmin $fmin --gmin $gmin --max_size $smax --out $output --no_closed_constraint >> $log_output
                /home/yann/timeout/timeout -t 3600 -m 12000000 python3 classification.py ../datasets/$dataset --fmin $fmin --gmin $gmin --max_size $smax --out $output >> $log_output
            done
        done
    done
done
