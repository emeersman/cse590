# Step Tracker (AKA The FitByte)

### Overview

The FitByte contains some of the functionality of a Fitbit in a much larger package. It can track the number of steps you take and visualize it on a built in screen. Step counting works by processing input from an accelerometer, smoothing the signal, and detecting signal peaks (steps) within a certain timeframe. Much like a Fitbit, the FitByte has multiple display screens. The user can cycle through these screens by tapping either side of the device. The display screens rotate between a readout of the number of steps taken, a visualization of the number of steps relative to the user’s goal steps, and a battery readout.

### Signal Processing

Before I could start signal processing, I needed a signal. I wired up the accelerometer and read acceleration values in each dimension. Following the rough signal processing approach outlined in a Mladenov paper, I first extracted the magnitude of each accelerometer reading. I then smoothed the signal using a running average of magnitude values. I played around with the sampling frequency until I could detect a reasonably smooth signal processed over a short timespan. Next, I implemented a peak detection algorithm. I modified it to only detect peaks over a certain threshold separated by a certain amount of time. Both of these values were determined by trial and error. This signal processing approach assumed a fixed placement of the device on the wearer so as to limit extraneous noise. Once the peak detection constants were dialed in, I was able to count steps based on accelerometer input!

### Feedback Interface

I chose to visualize the number of steps taken towards a goal value as a growing tree. Regardless of research linking abstract visualizations and motivation, I would have chosen this type of display because I enjoy designs related to nature.
My interface depicts a branching binary tree that gains a level whenever the user is a certain percent along the way towards reaching their goal steps. The tree is fully grown once the user reaches that goal.
I extended this visualization to other modes on the screen. The battery percentage screen displays the tree flipped upside down to resemble roots. The tree grows in reverse as the battery wanes.
I also built in a straightforward screen to display the total step count. This both helps with debugging and allows the user to see their concrete activity data.

### Reflection

In general, my favorite parts of the project related to the visualizations. I enjoyed representing data through abstract visual patterns. I think that this is a fascinating part of ubiquitous computing and I hope to explore it more in the future.
I was surprised by the number of moving parts in this assignment. Even though there were only a few major components of the step tracker (signal processing and visual display), there were a lot of things to monitor in the code. I kept thinking of additional constants and edge cases to add.
Throughout the assignment, I found so many places where I wanted to add more functionality. Once I had a basic step tracking algorithm, it felt like everything else was icing on the cake. I wanted to add more display modes, bonus IoT functionality, and countless features borrowed from Fitbit. While I didn’t succumb to feature creep and try to build a complete Fitbit, I gained a newfound appreciation for the amount of functionality packaged within existing step trackers.

### Links

- [Code](https://github.com/emeersman/cse590/tree/master/StepTracker)
- [Video](https://drive.google.com/open?id=1KJFUtvIc9BwxluaNcBnbTybWzPCVGRyq)
- [OLED code inspired by](https://learn.adafruit.com/monochrome-oled-breakouts/arduino-library-and-examples)
- [Signal smoothing code](https://www.arduino.cc/en/tutorial/smoothing)
- [Peak detection code](https://colab.research.google.com/github/makeabilitylab/signals/blob/master/Projects/StepTracker/StepTracker-Exercises.ipynb)
