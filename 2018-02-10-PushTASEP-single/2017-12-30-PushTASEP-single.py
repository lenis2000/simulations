# Simulation of the single layer inhomogeneous PushTASEP

import numpy
numpy.set_printoptions(threshold=numpy.nan)

from PIL import Image, ImageDraw

# 1. Set the parameters

n = 1200 #size of the lattice
k = 1  #depth, number of layers
t_max = 200 #time till which we simulate

def xi(x):      #inhomogeneous space parameters
    if x < 400:
        return 1
    else:
        return 2

###

a = 12 #output file number

# 2. initialize

P = numpy.ones((k,n)) #array of particles, initially step IC
XA = numpy.zeros((n)) #array of waiting times per sites
t = 0 #global time

for x in xrange(0,n):
    XA[x] = numpy.random.exponential(1./xi(x))

# 3. Some preliminary functions

def who_moves():
    global t
    global XA
    global n
    global xi
    j = numpy.argmin(XA)
    dt = XA[j]
    t += dt # update global time
    for xx in xrange(0,n):
        XA[xx] -= dt
    XA[j] = numpy.random.exponential(1./xi(j))
    return j

def make_move(j0):
    global P
    global k
    global n
    for i in xrange(0,k+1): # if no one moves, do nothing
        if i == k:
            break
        if P[i][j0] == 1:
            break
    if i == k:
        return

    P[i][j0] = 0 # particle jumps at layer i our of location j0
    if j0 == n-1: # if j0 is the last, that's it
        return

    for j1 in xrange(j0+1,n): #find till where we push
        if P[i][j1] == 0:
            P[i][j1] = 1
            return

# 4. Main simulation

m = 0
while t < t_max:
    j = who_moves()
    make_move(j)
    m += 1
    if( m % (min(10*n,5000)) == 0):
        print str(int(t)) + "/" + str(int(t_max))


# 5. print results to txt files

f = open('single-pushtasep-' + str(a) + '.txt', 'w')

f.write("{")
for i in xrange(0,k):
    f.write("{")
    for x in xrange(0,n):
        f.write(str(int(P[i][x])))
        if (x<n-1):
            f.write(",")
    if (i<k-1):
        f.write("},")
    else:
        f.write("}")
f.write("}")

