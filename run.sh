#$1 multh

#$1:num of threads   $2:io-size
for((i=2;i<33;i=i+2));
do
    for((j=4;j<=1024;j=j*2));
    do
        ./$1 $i $j
    done
done
