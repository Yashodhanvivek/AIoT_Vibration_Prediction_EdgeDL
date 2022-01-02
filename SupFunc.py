#importing libraries and dependecies 

#import pandas library for data manipulation and analysis
import pandas as pd

#numpy is a multi-dimensional arrays and matrices, along with a large collection of 
#high-level mathematical functions to operate on these arrays.
import numpy as np

#Matplotlib is a plotting library for the Python programming language and its 
#numerical mathematics extension NumPy
import matplotlib.pyplot as plt

#SciPy is a free and open-source Python library used for scientific computing 
#and technical computing. SciPy contains modules for optimization, linear algebra, 
#integration, interpolation, special functions, FFT, signal and image processing
from scipy import stats


# defining function for loading the dataset
def readData(filePath):
    # attributes of the dataset
    columnNames = ['motor_id','pattern','timestamp','x-axis','y-axis','z-axis']
    #read the specified file using pandas and return the data 
    data = pd.read_csv(filePath,header = None, names=columnNames,na_values=';')
    return data

# defining the function to plot a single axis data
def plotAxis(axis,x,y,title):
    axis.plot(x,y,color='green',linewidth=1)
    axis.set_title(title)
    axis.xaxis.set_visible(False)
    axis.set_ylim([min(y)-np.std(y),max(y)+np.std(y)])
    axis.set_xlim([min(x),max(x)])
    axis.grid(True)
    
# defining a function to plot the data for a given vibration pattern
def plotVibPattern(Vib,data):
    fig,(ax0,ax1,ax2) = plt.subplots(nrows=3, figsize=(20,10),sharex=True)
    plotAxis(ax0,data['timestamp'],data['x-axis'],'X-AXIS')
    plotAxis(ax1,data['timestamp'],data['y-axis'],'Y-AXIS')
    plotAxis(ax2,data['timestamp'],data['z-axis'],'Z-AXIS')
    plt.subplots_adjust(hspace=0.2)
    fig.suptitle(Vib)
    plt.subplots_adjust(top=0.9)
    plt.show()
    
# defining a window function for segmentation purposes
def windows(data,size):
    start = 0
    while start< data.count():
        yield int(start), int(start + size)
        start+= (size/2)
        
# split the time series data into window size of 24 
def split_data(data, window_size = 24):
    segments = np.empty((0,window_size,3))
    labels= np.empty((0))
    for (start, end) in windows(data['timestamp'],window_size):
        x = data['x-axis'][start:end]
        y = data['y-axis'][start:end]
        z = data['z-axis'][start:end]
			
        if(len(data['timestamp'][start:end])==window_size):
            segments = np.vstack([segments,np.dstack([x,y,z])])
            # TEST
            # print(segments)
            # while True:
            #  a=1
            labels = np.append(labels,stats.mode(data['pattern'][start:end])[0][0])
            # TEST
            # print(labels)
            # while True:
            #  a=1
            print(".")
    return segments, labels

