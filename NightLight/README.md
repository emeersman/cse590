# Pond Night Light

### Overview

- It's pond themed! Complete with three lily pads, a frog, and a luminous water lily.
- Electronics hidden underneath the “water” with an open back for sensor inputs and button interaction.
- Three modes plus a bonus “frog mode”
	- Mode 1: Light crossfades through the RGB spectrum, inversely proportional to light.
	- Mode 2: Light shines purple, the brightness fluctuates based on surrounding noise.
	- Mode 3: A wire coming out of the back of the pond can be used to touch different parts of the empty lily pad, changing the color of the light.
- Frog mode: At any time, pressing on the frog causes the light to blink green until the frog is pressed again.

### Design Process

I decided I wanted to use origami for my project because the materials were readily available and I thought it would be a nice complement to the light. I looked online and saw a few instances of LED and origami flower integration. At that point I knew I wanted to put the main light in a flower and create other features in the same theme to satisfy the project requirements. I built up the project gradually. First, I wired up a button and wrote code to change between three different modes (plus a zero state) whenever the button was pressed. Next, I hooked up the RGB LED and modified the code so that it would flash white whenever the mode was changed.

I then wired up the photocell and figured out how to process its input. I used the crossfade code from [Output Lesson 6](https://makeabilitylab.github.io/physcomp/arduino/rgb-led-fade.html#crossfading-in-the-rgb-color-space) for color and mapped the photocell signal to brightness. It took some adjusting to dial in the brightness based on the sensitivity of the photocell. I hooked up the microphone and read its input with the help of [online documentation](https://learn.adafruit.com/adafruit-microphone-amplifier-breakout/measuring-sound-levels). I had to use some roundabout mapping to convert the signal into brightness for an LED. However, when I tried to adjust the LED values based on the microphone signal, I got garbled input from the microphone. I asked a question and moved on while I waited for help. I wired Mode 3 with the help of a force-sensitive resistor. The code and circuit were very straightforward. I replaced the FSR with two wires and dialed in the input range with a test pencil scribble.

Decorating came near the end. After a few [YouTube tutorials](https://www.youtube.com/watch?v=PhL-fnhJvbs), I figured out how to make a relatively simple origami lotus. I wanted a nice-looking enclosure so I found a cardboard box and a lot of decorative paper and set to work. I cut holes in the box and passed the RGB LED and one of the lo-fi input wires to the top. Using many different kinds of tape, I secured everything to the top of the box and connected all necessary wires underneath. I had waited until the end to figure out the creative feature. I wanted to include a frog and eventually thought to make it a button. By wrapping conductive tape around its legs, I made the frog complete a circuit when pressed down. After the decorating was finished, I hooked my computer up and tested things out. Apart from a bit of calibration and some loose connections, the pond night light was a success!

### Reflection

This project gave me a lot of hands-on experience with a wide range of sensors. I learned that some sensors are incredibly sensitive and don’t always play well with other components. My experience with the microphone taught me a lot about debugging both hardware and software, especially how to hone in on the cause of an issue. Each sensor was delicate in its own way, whether with input value processing, physical size, or method of input. When implementing the lo-fi sensor, I learned to make sure the simple version of something works before moving onto the more difficult version. I envisioned many elaborate lo-fi inputs for this project but realized that all of them were digital. Late in the project, I pivoted and eventually ended up with a reliable graphite resistor. In projects like this, I am learning to balance practicality with ambition. I have a tendency to think up complex solutions that aren’t feasible to implement in the time given. Since this project had a lot of independent requirements, I was able to implement effective solutions for everything and pick a few small areas in which to challenge myself. I was able to experiment with new materials for my frog switch and I made a nicely designed enclosure which integrated art and tech. In future projects, I hope to challenge myself with entertaining yet reasonable goals.

### Links

- [Slide deck](https://docs.google.com/presentation/d/1ftNE5EDQNSa49ZbwteBGeZeEAb3FS483XhTf2g5VBug/edit?usp=sharing)
- [Video](https://drive.google.com/file/d/1Bl_fUCUqefV3wg519NLwhM8FjxQhrphG/view?usp=sharing)

