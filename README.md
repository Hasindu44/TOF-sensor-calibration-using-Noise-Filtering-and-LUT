\## How Calibration Works



This project uses \*\*Piecewise Linear Interpolation\*\*. Because ToF sensors have different error rates at different distances, a single offset isn't enough. We divide the range into 5 segments and calculate a unique \*\*Slope ($m$)\*\* and \*\*Offset ($c$)\*\* for each.



\### 1. The Calibration Formula

The code identifies which segment the raw reading falls into and applies:

$$Distance\_{corrected} = (Raw \\times m) + c$$



\### 2. Calculating the Slope ($m$)

To find the slope for a segment, we take the "True" distance (measured with a ruler) and the "Raw" distance (reported by the sensor) at two points (the start and end of the segment):



$$m = \\frac{True\_{end} - True\_{start}}{Raw\_{end} - Raw\_{start}}$$







\### 3. Calculating the Offset ($c$)

Once the slope is known, the offset (y-intercept) is calculated to align the sensor's starting point with reality:



$$c = True\_{start} - (m \\times Raw\_{start})$$



\### Example Calculation

If for Segment 1 (0-150mm):

\* At \*\*0mm\*\* (True), the sensor reads \*\*12mm\*\* (Raw).

\* At \*\*150mm\*\* (True), the sensor reads \*\*165mm\*\* (Raw).



\*\*Slope ($m$):\*\* $(150 - 0) / (165 - 12) = 150 / 153 \\approx \\mathbf{0.9804}$  

\*\*Offset ($c$):\*\* $0 - (0.9804 \\times 12) \\approx \\mathbf{-11.76}$



These values are then stored in the `s1\_slope\[]` and `s1\_offset\[]` arrays respectively.

