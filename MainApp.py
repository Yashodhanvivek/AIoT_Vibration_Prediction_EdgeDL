# importing libraries and dependecies 

#import the system module for standered input output
import sys
#import pandas library for data manipulation and analysis
import pandas as pd

#numpy is a multi-dimensional arrays and matrices, along with a large collection of 
#high-level mathematical functions to operate on these arrays.
import numpy as np

#Matplotlib is a plotting library for the Python programming language and its 
#numerical mathematics extension NumPy
import matplotlib.pyplot as plt

#import the sequential model from keras
from keras.models import Sequential

#import the needed layers for the convolutional neural Network
from keras.layers import Dense, Conv2D, MaxPooling2D, Flatten, Dropout
from keras import optimizers
import SupFunc as SF

#set the threshhold of prinitng data to console to maximum value
#so avoid the loss of data on console while displaying  
np.set_printoptions(threshold=sys.maxsize)
# setting up a random seed for reproducibility
random_seed = 611

np.random.seed(random_seed)

# matplotlib inline
plt.style.use('ggplot')
# defining function for loading the dataset

#************************MAIN CODE*********************************************

# Read The Data
dataset = SF.readData('Accl_Labeled_Data.txt')

for fault in np.unique(dataset['pattern']):
    #  a=1
    #select a subset of 180 samples for each pattern present in the pattern row
    subset = dataset[dataset['pattern']==fault][:180]

    SF.plotVibPattern(fault,subset)
    
# segmenting the signal in overlapping windows of 24 samples with 50% overlap
segments, Actuallabels = SF.split_data(dataset) 

#categorically defining the classes in one hot encoding
labels = np.asarray(pd.get_dummies(Actuallabels),dtype = np.int8)

while True:
 a=1
#-------------------PARAMETERS FOR Conv2D LAYER--------------------------------

# defining parameters for the input and network layers
# we are treating each segmeent or chunk as a 2D image (24 X 3)
NumberOfRows = segments.shape[1]
NumberOfColumns = segments.shape[2]
NumberOfChannels = 1
NumberOfFilters = 24 

# kernal size of the Conv2D layer
NumberOfRowsKernel = 16
NumberOfColumnsKernel = 1

#--------------------PARAMETERS FOR dropout LAYER------------------------------

# dropout ratio for dropout layer
dropOutRatio = 0.3

#------------------- PARAMETERS FOR MaxPooling2D LAYER-------------------------

# max pooling window size
NumberOfRowsPoolWindow = 2
NumberOfColumnsPoolWindow = 1
Stride1 = 2
Stride2 = 1

#------------------- PARAMETERS FOR Dense LAYER 1------------------------------

# number of Nodes in fully connected layers
NumberOfNeuronsDense = 12


#------------------- PARAMETERS FOR Output Dense LAYER 2-----------------------------

# number of total clases
NumberOfClasses = labels.shape[1]

#------------------- OTHER PARAMETERS -----------------------------------------

# split ratio for Training and Testing Data out of Captured Data
TrainingDataSplitRatio = 0.8

# number of NumberOfEpochs
epochs = 10

# BatchSize
BatchSize = 10

#------------------- PREPARE TRAINING AND TESTING DATA ------------------------

# reshaping the data for network input 
reshapedSegments = segments.reshape(segments.shape[0], NumberOfRows, NumberOfColumns,1)
# TEST
# print(reshapedSegments)

# splitting in training and testing data
trainSplit = np.random.rand(len(reshapedSegments)) < TrainingDataSplitRatio
# TEST
# print(trainSplit)

# Input and Output Training Data 
InputTrainingData = reshapedSegments[trainSplit]
InputTrainingData = np.nan_to_num(InputTrainingData)
# TEST
#print(InputTrainingData)
#while True:
# a=1
OutputTrainingData = labels[trainSplit]
# TEST
# print(OutputTrainingData)

# Input and Output Testing Data 
InputTestData = reshapedSegments[~trainSplit]
InputTestData = np.nan_to_num(InputTestData)
# TEST
# print(InputTestData)
OuputTestData = labels[~trainSplit]
# TEST
# print(OuputTestData)

#------------------- DEFINE CNN MODEL----------------- ------------------------

#Convolutional Neural network model to train on the vibrational pattern 
def TrainingModel():
    model = Sequential()
    # adding the first convolutionial layer with 24 filters and 2 by 1 kernal size, using the rectifier as the activation function
    model.add(Conv2D(NumberOfFilters, (NumberOfRowsKernel,NumberOfColumnsKernel),input_shape=(NumberOfRows, NumberOfColumns,1),activation='relu'))
    # adding a maxpooling layer
    model.add(MaxPooling2D(pool_size=(NumberOfRowsPoolWindow,NumberOfColumnsPoolWindow),strides=(Stride1,Stride2),padding='valid'))
    # flattening the output in order to apply the fully connected layer
    model.add(Flatten())
    # adding first fully connected layer with 12 units
    model.add(Dense(NumberOfNeuronsDense, activation='relu')) 
     # adding the dropout layer to avoid overfitting
    model.add(Dropout(dropOutRatio))
    # adding softmax layer for the classification
    model.add(Dense(NumberOfClasses, activation='softmax'))
    # Compiling the model to generate a model
    adam = optimizers.Adam(lr = 0.0001, decay=1e-6)
    model.compile(loss='categorical_crossentropy', optimizer=adam, metrics=['accuracy'])
    return model


#------------------- TRAIN THE CNN MODEL----------------- ---------------------
    
# Create an object of Neural Network Model
model = TrainingModel()

# Print the Layers used in the Neural Network Model
for layer in model.layers:
    print(layer.name)

#Train the Neural Network Model on training data
model.fit(InputTrainingData,OutputTrainingData, validation_split=1-TrainingDataSplitRatio,epochs=10,batch_size=BatchSize,verbose=2)

#Evaluate the model Accuracy on test data
score = model.evaluate(InputTestData,OuputTestData,verbose=2)

#print the model error
print(' Error: %.2f%%' %(100-score[1]*100))

#Save the model in a file
model.save('model.h5')

#Save the testData in .npy file
np.save('testData.npy',InputTestData)
np.save('groundTruth.npy',OuputTestData)
