for ((i = 5; i > 0; i--)); do
    time mpirun -np $i life "../board" >ans
done
