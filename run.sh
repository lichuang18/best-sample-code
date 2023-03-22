#$1 multh

#$1:num of threads   $2:io-size
for((i=2;i<33;i=i+2));
do
    for((j=4;j<=4;j=j*2));
    do
        echo 3 > /proc/sys/vm/drop_caches
        ./$1 $i $j
    done
done
