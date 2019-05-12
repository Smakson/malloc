# data
realloc only reallocates 0 and its a very big chungus
up to 28k in 1 640k in second

very specific solution:
we extend and move the thing we are writing over 

# remember
we cannot move data that corresponds to other pointer
(as pointers will get invalidated)

# throughput is sad
half the points for noe
cause we iterate overall, instead of iterating
"smartly" on empty ones, or with 2 lists

( not priority but could be fun
# empty chungies can hold lots of pointers
we should build "somewhat" balanced trees using some generator
to choose a branch among the (lets say) 8 children if all taken)

# topic / trace specific

# binary thing
needs at least 2 lists:
allocating big-smol-big-smol, remove all small, add big
i thing we should do the splitting at 96B
but pay attention to next points:
the small lists things should be merged into biglistelement

# coalescing thing
requires merging smallboyz to put bigboyz

# realloc
for the last one we NEED to start with an indent
to avoid wasting space
