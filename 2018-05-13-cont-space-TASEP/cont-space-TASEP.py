#Simulation of continuous space TASEP (q = 0)

import numpy as np
np.set_printoptions(threshold=np.nan)
# import matplotlib.pyplot as plt
# from mpl_toolkits.mplot3d import Axes3D
# from matplotlib import cm
import sys
sys.path.append("/usr/local/Cellar/pillow/2.7.0/lib/python2.7/site-packages")
import PIL.ImageDraw as ImageDraw, PIL.Image as Image, PIL.ImageShow as ImageShow
import PIL.ImageFont as ImageFont
# import matplotlib.pyplot as plt


number_of_jumps = 20000000	#max number of total particle jumps
max_time = 1000				#max time to which simulate
nu_scale = 1				#assuming scaled nu is equal to 1
first_rate = 1				#the rate of incoming particles, a_0

global time_ct					#counter of continuous time
time_ct = 0

file_name = "HF_3.txt"		#file for output of coordinates of particles
pic_name = "pic0.png"		#file for output of picture of the height function

##################################################################################################

def a_rates( y ):						#inhomogeneity function a(y), can redefine
	# "This is the rate function a(y)"
	return 1

def particles_mass( particles ):
	"This function returns the normalization constant"
	a_y_mass = first_rate
	for x in xrange(0,n_particles):
		a_y_mass += a_rates( particles[x][0] ) 
	return a_y_mass

def who_jumps( particles ):
	"Determines randomly who will jump (returns the number of occupied location or 0 if adding new particle)"
	u = np.random.uniform(0, particles_mass(particles))
	
	global time_ct

	time_to_add = np.random.exponential( float(1) / particles_mass(particles) )
	time_ct += time_to_add	#counting global continuous time as jumps go

	a_y_mass = first_rate
	for x in xrange(0,n_particles):
		if u < a_y_mass:
			return x
		a_y_mass += a_rates( particles[x][0] ) 
	return n_particles

##################################################################################################

n_particles = 0 #total number of particles, ignoring multiplicities - initially empty
particles = np.zeros((n_particles), dtype=[('x', float), ('mult', int)])

for t in xrange(0,number_of_jumps):
	if(time_ct > max_time):
		break

	if np.mod(t,500) == 0:
		print "jumps = " + str(t) + ";    time = " + str(time_ct)

	who_jumps_now = who_jumps(particles)

	if who_jumps_now > 0:
		curr_loc = particles[who_jumps_now - 1][0]
		particles[who_jumps_now - 1][1] -= 1
	else: 
		curr_loc = 0

	desired_loc = curr_loc + np.random.exponential(1/nu_scale)

	looking_at = who_jumps_now

	while(1):
		if looking_at == n_particles or ( particles[looking_at][0] > desired_loc ):
			# there is nothing to overjump

			n_particles += 1
			particles.resize((n_particles))
			particles[n_particles-1] = (desired_loc,1)
			particles.sort(order = 'x')
			break

		else:
			particles[looking_at][1] += 1
			break
		
		looking_at += 1
		
	for y in xrange(0, n_particles):
		if particles[y][1] == 0:
			particles = np.delete(particles, y)
			n_particles -= 1
			break

# print particles to file

f = open(file_name, 'w')
f.write("{")
for x in xrange(0, n_particles):
		for i in xrange(0,particles[x][1]):
			f.write(str(particles[x][0]))
			f.write(", ")
f.write("}")
