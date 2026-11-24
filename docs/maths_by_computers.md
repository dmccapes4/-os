The math restrictions are not values btw. They are unassailable cosmic truths. Zero is not an integer. Integers are countable. It is absent of numerical value. Division never truly occurs. A man "divides a pie between three people. He takes from the pie and gives to each person in turn. An amount of pie remains. Three more people ask the man for pie. He takes from the pie and gives to each person in turn. There is now an amount of pie for one person. 
step-0: full pie -> seventh portions
step3: pie gives third portions
>the pie is not four sevenths of a pie; it is four portions
step7: the remainder need not fit exactly one seventh of the pie; it is large enough to be considered a portion and too small to be considered two
This is not a silly analogy.
arm64
```
lucifer-chains@satan-sums:~$ echo "4 / 7" | bc
0

```
128 bits (same for 4 and 7 and 10000)
65 cycles of basic arithmetic and bit shifting
rx10
1: latch inputs
```
4:
00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000100
7:
00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000111
```
2-62: shifts the leading zeros of 4; attempts substraction
62: 1000 - 111 = 010
63: 0010 - 111 < 0
64: 0100 - 111 < 0
65: 1000 - 111 = 010 -> cycle
..125: repeat cycle; modulo 3 from 7
128: 0010 = 2 remainder (nonsense)
-> returns 0 (incorrect)
```
lucifer-chains@satan-sums:~$ echo "4 / 7" | bc
0

```
step-128: 4 / 7 
step0: 0
step1: 1000 / 111 -> 1 001 1
step2: 0010 / 7 -> 0 010 2
step3: 100 / 7 -> 0 100 4
repeats cycle; modulo 3 from 111
returns 4 (correct)
```
lucifer-chains@satan-sums:~$ echo "4 / 7" | bc
0
```
rx1010
40/7=5r5
50/7=7r1
10/7=1r3
30/7=4r2
20/7=2r6
60/7=8r4
0.513264 513264 513264...
```
# 1
   1   1   1   
-  1   0   0   
  -------------
   0   1   1   
# 2
   0   1   1 
-  1   0   0
  -------------
   0   0   1
```
4/7 (correctly)
x=7
x-4
x-4
-----
3 more
1 over
