#!/usr/local/Cellar/python/2.7.6/bin/python
# -*- coding: utf-8 -*-

import sys
from numpy import *

import scipy.misc
from matplotlib import pyplot

def otsu2( hist, total ):
	"""
	This is the more common (optimized) implementation of otsu algorithm, the one you see on Wikipedia pages
	"""
	no_of_bins = len( hist ) # should be 256

	sum_total = 0
	for x in range( 0, no_of_bins ):
		sum_total += x * hist[x]
	
	weight_background 	  = 0.0
	sum_background 		  = 0.0
	inter_class_variances = []

	for threshold in range( 0, no_of_bins ):
		# background weight will be incremented, while foreground's will be reduced
		weight_background += hist[threshold]
		if weight_background == 0 :
			continue

		weight_foreground = total - weight_background
		if weight_foreground == 0 :
			break

		sum_background += threshold * hist[threshold]
		mean_background = sum_background / weight_background
		mean_foreground = (sum_total - sum_background) / weight_foreground

		inter_class_variances.append( weight_background * weight_foreground * (mean_background - mean_foreground)**2 )

	# find the threshold with maximum variances between classes
	return argmax(inter_class_variances)

def otsu1( hist, total ):
	"""
	This is the original otsu thresholding algorithm. No optimization is applied, in order 
	to illustrate what the algorithm is trying to do
	'hist' is the histogram of the image or data
	'total' is the total number of pixels of the image, or the size of data
	"""
	no_of_bins = len( hist ) # should be 256

	intra_class_variances = []

	for threshold in range( 0, no_of_bins ):
		# first we try to find the weight and variance on the background
		sum_background		= float(sum( hist[0:threshold] ))
		weight_background 	= sum_background / total
		mean_background 	= 0.0
		variance_background = 0.0

		if sum_background > 0.0: # avoid division by zero
			for x in range( 0, threshold ):
				mean_background += x * hist[x]
			mean_background /= sum_background

			for x in range( 0, threshold ):
				variance_background += (x - mean_background) ** 2 * hist[x]
			variance_background /= sum_background	


		# then we do it for the foreground
		sum_foreground		= float(sum( hist[threshold:no_of_bins] ))
		weight_foreground 	= sum_foreground / total
		mean_foreground 	= 0.0
		variance_foreground = 0.0

		if sum_foreground > 0.0:
			for x in range( threshold, no_of_bins ):
				mean_foreground += x * hist[x]
			mean_foreground /= sum_foreground

			for x in range( threshold, no_of_bins ):
				variance_foreground += (x - mean_foreground) ** 2 * hist[x]
			variance_foreground /= sum_foreground	

		# find the variances within these two classes
		intra_class_variances.append( weight_background * variance_background + weight_foreground * variance_foreground )

	# use the threshold that has the minimum intra class variance
	return argmin( intra_class_variances ) - 1
			

def main():
	img = scipy.misc.imread( '/Users/saburookita/Desktop/Bright_green_tree_-_Waikato.jpg' )

	# resize to a more managable size
	img = scipy.misc.imresize( img, (1944 / 4, 2592 / 4) ) 

	# convert to grayscale
	grayscale = img.dot( [0.299, 0.587, 0.144] ) 
	rows, cols = shape( grayscale )

	# create 256 bins histogram
	hist = histogram( grayscale, 256 )[0]

	# apply the otsu thresholding
	thresh = otsu2( hist, rows * cols )
	
	figure  = pyplot.figure( figsize=(14, 6) )
	figure.canvas.set_window_title( 'Otsu thresholding' )

	axes    = figure.add_subplot(121)
	axes.set_title('Original')
	axes.get_xaxis().set_visible( False )
	axes.get_yaxis().set_visible( False )
	axes.imshow( img, cmap='Greys_r' )

	axes    = figure.add_subplot(122)
	axes.set_title('Otsu thresholding')
	axes.get_xaxis().set_visible( False )
	axes.get_yaxis().set_visible( False )
	axes.imshow( grayscale >= thresh, cmap='Greys_r' )

	pyplot.show()

if __name__ == '__main__':
	main()