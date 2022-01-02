# importing the dependencies
#import the system module for standered input output
import sys
from keras.models import load_model
import numpy as np
from sklearn import metrics

 
# ***************Main Code ************************************
#set the threshhold of prinitng data to console to maximum value
#so avoid the loss of data on console while displaying  
np.set_printoptions(threshold=sys.maxsize)    
#loading the pretrained model
model = load_model('model.h5')
#loading the testData and groundTruth data
testData = np.load('testData.npy')
groundTruth = np.load('groundTruth.npy')
# TEST
# print(testData)
# print(groundTruth)
# defining the class labels

#labelsRow = np.array(['Fault1','Fault2','Fault3','Fault4','No Operation','Normal'])
#labelsColumn = np.array(['Fault1','Fault2','Fault3','Fault4','No Operation','Normal',' '])

labelsRow = np.array(['Fault1','Fault2','Fault3','Normal'])
labelsColumn = np.array(['Fault1','Fault2','Fault3','Normal',' '])
# predicting the classes
predictions = model.predict(testData,verbose=2)
# TEST
# print(predictions)

# getting the class predicted and class in ground truth for creation of confusion matrix
predictedClass = np.zeros((predictions.shape[0]))
# TEST
# print(predictedClass)

groundTruthClass = np.zeros((groundTruth.shape[0]))
# TEST
# print(groundTruthClass)
    
for case in range (groundTruth.shape[0]):
    predictedClass[case] = np.argmax(predictions[case,:])
    groundTruthClass[case] = np.argmax(groundTruth[case,:])
    
# TEST
# print(predictedClass)
# print(groundTruthClass)    

# obtaining a confusion matrix  
ConfusionMatrix = metrics.confusion_matrix(groundTruthClass,predictedClass)
# TEST
# print(ConfusionMatrix)

# Calculate the normalized confusion matrix 
cmNormalize = np.around((ConfusionMatrix/ConfusionMatrix.sum(axis=1)[:,None])*100,2)


#print the normalized confusion marix
print(cmNormalize)
# Add labels to row and column
cmNormalizeFinal = np.row_stack((cmNormalize ,labelsRow))
cmNormalizeFinal = np.column_stack((cmNormalizeFinal ,labelsColumn))
#print final confusion matrix
print(cmNormalizeFinal)




