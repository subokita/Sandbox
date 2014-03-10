#!/usr/local/Cellar/python/2.7.6/bin/python
# -*- coding: utf-8 -*-

import sys
from numpy import *

# for comparison purposes
import cv2
from matplotlib import pyplot, cm

import scipy.io, scipy.misc, scipy.signal

def gabor( size, sigma, theta, lambd, gamma, psi=0.5*pi, separate=False ):
	xmax, ymax = size / 2, size / 2

	x, y = meshgrid( linspace(-xmax, xmax, size ), linspace(-ymax, ymax, size ) )

	x_theta =  x * cos(theta) + y * sin(theta)
	y_theta = -x * sin(theta) + y * cos(theta)

	gauss 	= exp( - ( x_theta**2 + gamma**2 * y_theta **2 ) / (2. * sigma**2) )
	grating = cos( (2 * pi / lambd) * x_theta + psi )

	# if you want to have the gaussian and sinusoidal components separated
	if separate:
		return (-gauss, grating)
	else:
		return -gauss * grating
	
def gaborKernelExample():
	"""
	This example shows that gabor kernel is basically multiplication of two components,
	gaussian and sinusoidal
	"""
	size  = 20 		 # size of kernel
	sigma = 1.0 	 # the standard deviation of Gaussian, the higher the stdev, the less sensitive the filter is
	gamma = 1. 		 # basically controls gamma for y's sigma
	psi   = 0.5 * pi # phase shift
	sigma = 2.5
	lambd = 1.0

	for theta_iter in arange(0, 90.5, 15.):
		
		theta = theta_iter * pi / 180.  # theta is orientation

		# we get the gaussian and the sinusoidal components
		gauss, grating = gabor( size=size, sigma=sigma, theta=theta, lambd=lambd, gamma=gamma, psi=psi, separate=True )

		# plot each of them to show the differences
		fig = pyplot.figure( figsize=(14, 5) )
		fig.canvas.set_window_title( 'Gabor filter example' )

		ax  = fig.add_subplot( 131 )
		ax.set_title('Gaussian part')
		ax.imshow( gauss, cmap=cm.binary )
		
		ax  = fig.add_subplot( 132 )
		ax.set_title('Sinusoidal part')
		ax.imshow( grating, cmap=cm.binary )
		
		ax  = fig.add_subplot( 133 )
		ax.set_title('Gabor kernel')
		ax.imshow( gauss * grating, cmap=cm.binary )
		pyplot.show()



def main():
	img = scipy.misc.imread( '/Users/saburookita/Desktop/DS2.jpg' )
	rows, cols = shape( img )[0], shape(img)[1]

	# resize to a more managable size
	img = scipy.misc.imresize( img, (rows / 2, cols / 2) ) 
	
	# convert to grayscale
	grayscale = img.dot( [0.299, 0.587, 0.144] ) 

	rows, cols = shape( grayscale )

	theta = 0.0 * pi / 180.0
	lambd = 1.0 / 0.04
	size  = 20 		 # size of kernel
	sigma = 1.0 	 # the standard deviation of Gaussian, the higher the stdev, the less sensitive the filter is
	gamma = 1. 		 # basically controls gamma for y's sigma
	psi   = 0.5 * pi # phase shift

	for theta_iter in arange(0, 90.5, 15.):
		for freq in arange( 0.2, 1.0, 0.2 ):
			lambd = 1. / freq 				# lambda is wavelength
			theta = theta_iter * pi / 180.  # theta is orientation

			# create the kernel
			kernel = gabor( size=size, sigma=sigma, theta=theta, lambd=lambd, gamma=gamma, psi=psi )
			convolved = scipy.signal.convolve2d(grayscale, kernel, mode='same' )

			# kernel that we made from this example == kernel from opencv
			# kernel2 = cv2.getGaborKernel( ksize=(size, size), sigma=sigma, theta=theta, lambd=lambd, gamma=gamma, psi=psi )

			# draw the kernel inside our convolved image
			convolved[0:size*5, 0:size*5]   = scipy.misc.imresize( kernel, (size*5, size*5) ) 

			fig = pyplot.figure( figsize=(14, 8) )
			fig.canvas.set_window_title( 'Gabor filter example' )
			ax  = fig.add_subplot( 111 )
			ax.set_title('theta = %.1f degree, frequency = %.1f' % (theta_iter, freq) )
			ax.imshow( convolved, cmap=cm.binary )
			
			pyplot.show()



if __name__ == '__main__':
	main()