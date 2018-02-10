# Simulation of the single layer PushTASEP in inhomogeneous space

import numpy
numpy.set_printoptions(threshold=numpy.nan)

# 1. Set the parameters

n = 200 #number of particles
t_max = 150 #time till which we simulate

def xi(y):      #inhomogeneous space parameters
    if y <= 150: 
        return 1
    else:
        return 20

###

a = 2 #output file number

# 2. initialize

P = numpy.zeros((n)) #array of particles' locations, initially step IC
XA = numpy.zeros((n)) #array of waiting times per sites
t = 0 #global time

for x in xrange(0,n):
    XA[x] = numpy.random.exponential(1./xi(x))

for x in xrange(0,n):
    P[x] = x

print P
print XA

# 3. Main simulation

m = 0
while t < t_max:
    
    dt = numpy.amin(XA)
    j = numpy.argmin(XA)
 
    t += dt # update global time
    for xx in xrange(0,n):
        XA[xx] -= dt

    P[j] = 0
    if j < n-1:
    for j1 in xrange(j+1,n): #find till where we push
        if P[j1] == 0:
            P[j1] = 1
            break
    
    XA[j] = numpy.random.exponential(1./xi(P[j1]))
    

    m += 1
    if( m % (min(10*n,5000)) == 0):
        print str(int(t)) + "/" + str(int(t_max))

# 4. print results to txt files

f = open('single-layer-pushtasep-' + str(a) + '.txt', 'w')

f.write("{")
for x in xrange(0,n):
    f.write(str(int(P[x])))
    if (x<n-1):
        f.write(",")
f.write("}")
