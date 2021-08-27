# DesktopFilmProcessor
Part of a series of "FilmNX" projects, this machine is designed to precisely and quickly deliver temperature controlled developer chemistry to film developer tank(s) and without residual contamination (increasing life of chemistry and reducing waste water). 

Demo Video of hardware in progress > https://youtu.be/uSjSORCjSXM https://youtu.be/JuO49YhMA7I

Project is v0.02. Early hardware validation. Can work for intended purpose, however the present design doesn't include aduiquite hardware for reliable/error free operation in the case of simple code defects or user errors.  Namely, adding weight sensors or fluid level sensors to the source tanks to ensure the device doesn't accidently pump fluid back to the wrong tank and cause an overflow. 

The Arduino code sketches are located under /firmware.  Currently this project only has some basic code to test the hardware, under "/firmware/bringup".  

Mechanical files are located under /hardware.  

BOM - TBD
