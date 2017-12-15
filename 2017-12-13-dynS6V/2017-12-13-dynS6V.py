#Simulation of the dynamic stochastic six vertex model

import numpy
numpy.set_printoptions(threshold=numpy.nan)

#1. Set the global parameters

u = 0.1
v = 0.9
tSpin = 0.5
s = 0.0

n = 200	        #time (vertical size) of the system
horn = 200	#horizontal size of the system

#2. initialize the main arrays of data for simulation

W = numpy.zeros((n,horn,4))
hf = numpy.zeros((n,horn))
for t in xrange(0,n):
	W[t][0][1] = 1
        hf[t][0] = t

#3. do the simulation

for t in xrange(0,n):
        if numpy.mod(t,100)==0:
                print t
        for x in xrange(0,horn):
                if W[t][x][1] > W[t][x][0]:
                        u = numpy.random.uniform(0,1)

                        if u < (v-u)/(v- tSpin*u)*(v-s*tSpin**(hf[t][x]+1))/(v-s*tSpin**(hf[t][x])) :
                                i2 = W[t][x][0]
                                j2 = W[t][x][1]
                        else:
                                i2 = W[t][x][1]
                                j2 = W[t][x][0]

                        W[t][x][2] = i2
                        if t < n-1:
                                W[t+1][x][0] = i2
                        W[t][x][3] = j2
                        if x < horn-1:
                                W[t][x+1][1] = j2
                        if x < horn - 1 and t < n - 1 :
                                hf[t+1][x] = hf[t][x] + W[t][x][1]
                                hf[t][x+1] = hf[t][x] - W[t][x][0]
                                hf[t+1][x+1] = hf[t][x] + W[t][x][1] - i2

                if W[t][x][1] < W[t][x][0]:
                        u = numpy.random.uniform(0,1)

                        if u < tSpin*(v-u)/(v- tSpin*u)*(u-s*tSpin**(hf[t][x]-1))/(u-s*tSpin**(hf[t][x])) :
                                i2 = W[t][x][0]
                                j2 = W[t][x][1]
                        else:
                                i2 = W[t][x][1]
                                j2 = W[t][x][0]

                        W[t][x][2] = i2
                        if t < n-1:
                                W[t+1][x][0] = i2
                        W[t][x][3] = j2
                        if x < horn-1:
                                W[t][x+1][1] = j2
                        if x < horn - 1 and t < n - 1 :
                                hf[t+1][x] = hf[t][x] + W[t][x][1]
                                hf[t][x+1] = hf[t][x] - W[t][x][0]
                                hf[t+1][x+1] = hf[t][x] + W[t][x][1] - i2

                if W[t][x][1] == W[t][x][0]:
                        i2 = W[t][x][0]
                        j2 = W[t][x][1]

                        W[t][x][2] = i2
                        if t < n-1:
                                W[t+1][x][0] = i2
                        W[t][x][3] = j2
                        if x < horn-1:
                                W[t][x+1][1] = j2
                        if x < horn - 1 and t < n - 1 :
                                hf[t+1][x] = hf[t][x] + W[t][x][1]
                                hf[t][x+1] = hf[t][x] - W[t][x][0]
                                hf[t+1][x+1] = hf[t][x] + W[t][x][1] - i2

# 4. print results to txt files

a = 1 #this is the filename which may be changed; must be matched by "a" in Mathematica file

f = open('Hfout' + str(a) + '.txt', 'w')

f.write("{")
for t in xrange(0,n):
	f.write("{")
	for x in xrange(0,n):
		f.write("{" + str(W[t][x][0]) + "," + str(W[t][x][1]) + ","  + str(W[t][x][2]) + ","  + str(W[t][x][3]))
		if x<n-1:
			f.write("},")
		else:
			f.write("}")
	if t<n-1:
		f.write("},")
	else:
		f.write("}")
f.write("}")
