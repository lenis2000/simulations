#Simulation of the new multilayer PushTASEP, in inhomogeneous space

import numpy
numpy.set_printoptions(threshold=numpy.nan)

# 1. Set the parameters

n = 18 #size of the lattice
k = 3  #depth, number of layers
t_max = 250 #time till simulate

def xi(x):      #inhomogeneous space parameters
    if n/3 < x < 2*n/3:
        return .1
    else:
        return 1

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
    if( m % 50 == 0):
        print t
        print P


# 5. print results to txt files

# a = 1 #this is the filename which may be changed; must be matched by "a" in Mathematica file
#
# f = open('Hfout' + str(a) + '.txt', 'w')
#
# f.write("{")
# for t in xrange(0,n):
# 	f.write("{")
# 	for x in xrange(0,n):
# 		f.write("{" + str(W[t][x][0]) + "," + str(W[t][x][1]) + ","  + str(W[t][x][2]) + ","  + str(W[t][x][3]))
# 		if x<n-1:
# 			f.write("},")
# 		else:
# 			f.write("}")
# 	if t<n-1:
# 		f.write("},")
# 	else:
# 		f.write("}")
# f.write("}")
