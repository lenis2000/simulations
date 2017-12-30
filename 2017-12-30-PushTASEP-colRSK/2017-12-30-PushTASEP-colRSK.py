# Simulation of the multilayer PushTASEP corresponding to the column RSK

import numpy
numpy.set_printoptions(threshold=numpy.nan)

from PIL import Image, ImageDraw

# 1. Set the parameters

n = 10 #size of the lattice
k = 5  #depth, number of layers
t_max = 3 #time till which we simulate

def xi(y):      #inhomogeneous space parameters
    if y <= 150: 
        return 1
    else:
        return 20

###

a = 1 #output file number
graph_mult = 10 #the scale at which we display the result

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

    print "start move"

    l = 0
    s = j0

    while True:
        while P[l][s] == 0:
            l = l + 1
            print "=================================="
            if l == k:
                return
        P[l][s] = 0

        if s == n - 1:
            return
        else: 
            s = s + 1

        while P[l][s] == 1:
            s = s + 1
            print "site"
            if s == n:
                return
        P[l][s] = 1

        if l == k - 1:
            return
        else:
            l = l + 1
        

# 4. Main simulation

m = 0
while t < t_max:
    j = who_moves()
    print j
    make_move(j)
    print P
    m += 1
    if( m % (min(10*n,5000)) == 0):
        print str(int(t)) + "/" + str(int(t_max))


# 5. print results to txt files

f = open('multilayer-pushtasep' + str(a) + '.txt', 'w')

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

# 6. Graphics output

im = Image.new('RGB', (n*graph_mult, k*graph_mult), (255,255,255))
draw = ImageDraw.Draw(im)

for mi in xrange(0,k):
    for mx in xrange(0,n):
        if (int(P[mi][mx]) == 1):
            draw.rectangle([mx*graph_mult,mi*graph_mult,(mx+1)*graph_mult,(mi+1)*graph_mult], fill="black")

im.show()
im.save('multilayer-pushtasep-graph' + str(a) + '.png')
