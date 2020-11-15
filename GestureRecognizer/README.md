# Shape-Based Gesture Recognizer

### Overview

For this project, I recorded a set of gestures using an accelerometer connected to an ESP32 microcontroller. I then built and trained an ML model to recognize each type of gesture. To do this, I used signal processing and filtering on the raw accelerometer data. After that, I experimented with different distance algorithms to determine the optimal gesture matching approach. I tried multiple different algorithms across a combination of features. Finally, I ran my highest performing algorithms across a wider dataset to measure accuracy. 

### Links

- [Slide deck with charts](https://docs.google.com/presentation/d/1VnA4nG_r2b08-Sm5n64igiwtp-mF8sDTAhV9Tsow6E8/edit?usp=sharing)


# Feature-Based Gesture Recognizer

### Overview

This project builds upon the shape-based gesture recognizer. I built a gesture classification model based off of a combination of different features extracted from the data. I created two versions of the model which used the SVM and kNN algorithms. I then used this model on wider dataset beyond my own gestures to determine which features yielded the highest classification accuracy across datasets.

### Links

- [Slide deck with charts](https://docs.google.com/presentation/d/1rPvx1ku-5KCze9gaMdHAQLJlt70SZ-HKFg8G5BC0i8U/edit?usp=sharing)
