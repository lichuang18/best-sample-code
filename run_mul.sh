#$1 multh

#$1:num of threads   $2:io-size

for((j=4;j<=8192;j=j*2));
do
    echo 3 > /proc/sys/vm/drop_caches
    ./$1 $2 $j
done

