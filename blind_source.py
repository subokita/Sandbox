#!/usr/local/Cellar/python/2.7.6/bin/python
# -*- coding: utf-8 -*-

import sys

from numpy import *
from scipy import signal
import scipy.io.wavfile

from matplotlib import pyplot

import sklearn.decomposition

def main():
	# First load the audio data, the audio data on this example is obtained from http://www.ism.ac.jp/~shiro/research/blindsep.html
	rate, source = scipy.io.wavfile.read('/Users/saburookita/Sandbox/blind_source_data/X_rsm2.wav')

	# The 2 sources are stored in left and right channels of the audio
	source_1, source_2 = source[:, 0], source[:, 1]
	data = c_[source_1, source_2]

	# Normalize the audio from int16 range to [-1, 1]
	data = data / 2.0 ** 15

	# Perform Fast ICA on the data to obtained separated sources
	fast_ica  = sklearn.decomposition.FastICA( n_components=2  )
	separated = fast_ica.fit_transform( data )

	# Check, data = separated X mixing_matrix + mean
	assert allclose( data, separated.dot( fast_ica.mixing_.T ) + fast_ica.mean_ )

	# Map the separated result into [-1, 1] range
	max_source, min_source = 1.0, -1.0
	max_result, min_result = max(separated.flatten()), min(separated.flatten())
	separated = map( lambda x: (2.0 * (x - min_result))/(max_result - min_result) + -1.0, separated.flatten() )
	separated = reshape( separated, (shape(separated)[0] / 2, 2) )
	
	# Store the separated audio, listen to them later
	scipy.io.wavfile.write( '/Users/saburookita/Sandbox/blind_source_data/separated_1.wav', rate, separated[:, 0] )
	scipy.io.wavfile.write( '/Users/saburookita/Sandbox/blind_source_data/separated_2.wav', rate, separated[:, 1] )

	# Plot the original and separated audio data
	fig = pyplot.figure( figsize=(10, 8) )
	fig.canvas.set_window_title( 'Blind Source Separation' )

	ax = fig.add_subplot(221)
	ax.set_title('Source #1')
	ax.set_ylim([-1, 1])
	ax.get_xaxis().set_visible( False )
	pyplot.plot( data[:, 0], color='r' )

	ax = fig.add_subplot(223)
	ax.set_ylim([-1, 1])
	ax.set_title('Source #2')
	ax.get_xaxis().set_visible( False )
	pyplot.plot( data[:, 1], color='r' )

	ax = fig.add_subplot(222)
	ax.set_ylim([-1, 1])
	ax.set_title('Separated #1')
	ax.get_xaxis().set_visible( False )
	pyplot.plot( separated[:, 0], color='g' )

	ax = fig.add_subplot(224)
	ax.set_ylim([-1, 1])
	ax.set_title('Separated #2')
	ax.get_xaxis().set_visible( False )
	pyplot.plot( separated[:, 1], color='g' )
	pyplot.show()



if __name__ == '__main__':
	main()