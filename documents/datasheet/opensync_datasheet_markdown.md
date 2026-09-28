# OpenSync Digital Delay/Pulse Generator
An open source synchronizer for the velocimetry of fluids using a Raspberry Pi microcontroller.

<table>
  <tr></tr>
  <tr>
    <th colspan="2">General</th>
  </tr>
  <tr>
    <td>Output Channels</td>
    <td>8 Independent Channels</td>
  </tr>
  <tr>
    <td>Input Channels</td>
    <td>2 Channels (1 Ext. Trigger; 1 Gate)</td>
  </tr>
  <tr>
    <td>Communication</td>
    <td>USB On-the-go (OTG)</td>
  </tr>

  <tr>
    <th colspan="2">Internal Timing Generator</th>
  </tr>
  <tr>
    <td>Period Range</td>
    <td>0.0004 Hz to 3.125 MHz</td>
  </tr>
  <tr>
    <td>Resolution</td>
    <td>4 ns * Clock Divider</td>
  </tr>
  <tr>
    <td>Accuracy</td>
    <td>4 ns * Clock Divider</td>
  </tr>
  <tr>
    <td>PLL Frequency</td>
    <td>250 MHz</td>
  </tr>
  <tr>
    <td>Crystal Oscillator</td>
    <td>12 MHz 30 ppm</td>
  </tr>
  <tr>
    <td>Clock Divider Range</td>
    <td>1 to 65,500</td>
  </tr>
  <tr>
    <td>Jitter</td>
    <td>Usually < 0.1 ns</td>
  </tr>
  <tr>
  <tr>
    <td>Triggering</td>
    <td>Internal, External, Gated</td>
  </tr>
  <tr>
    <td>Counter Depths</td>
    <td>32 Bits</td>
  </tr>
  <tr>
    <td>Outputs</td>
    <td>T0 Event Out</td>
  </tr>
  <tr>
    <td>Pulse Duration</td>
    <td>20 ns * Clock Divider</td>
  </tr>
  <tr>
    <td>Output Voltage</td>
    <td>3.3 V or 5 V</td>
  </tr>
  <tr>
    <td>Output Impedance</td>
    <td>~50 Ohms</td>
  </tr>
  <tr>
    <td>Output Rise/Fall</td>
    <td>< 4 ns</td>
  </tr>

  <tr>
    <th colspan="2">Channel Timing Generator</th>
  </tr>
  <tr>
    <td>Pulse Range</td>
    <td>44 ns to 8 s * Clock Divider</td>
  </tr>
  <tr>
    <td>Resolution</td>
    <td>4 ns * Clock Divider</td>
  </tr>
  <tr>
    <td>Accuracy</td>
    <td>4 ns * Clock Divider</td>
  </tr>
  <tr>
    <td>Clock Divider Range</td>
    <td>1 to 65,500</td>
  </tr>
  <tr>
    <td>Single-channel Jitter</td>
    <td>Usually < 0.1 ns</td>
  </tr>
  <tr>
    <td>Inter-channel Jitter</td>
    <td>Usually < 0.4 ns</td>
  </tr>
  <tr>
    <td>Triggering</td>
    <td>T0, CH A-H, Gated</td>
  </tr>
  <tr>
    <td>Counter Depths</td>
    <td>32 Bits</td>
  </tr>
  <tr>
    <td>Delay Counter Depth</td>
    <td>31 Bits (1 bit used for outptu state)</td>
  </tr>
  <tr>
    <td>Output State/Delay Buffer</td>
    <td>6 Output State/Delay Pairs</td>
  </tr>
  <tr>
    <td>Output Voltage</td>
    <td>3.3 V or 5 V</td>
  </tr>
  <tr>
    <td>Output Impedance</td>
    <td>~50 Ohms</td>
  </tr>
  <tr>
    <td>Output Rise/Fall</td>
    <td>< 4 ns</td>
  </tr>

  <tr>
    <th colspan="2">Miscellaneous</th>
  </tr>
  <tr>
    <td>Min. Trigger Length</td>
    <td>12 ns * Clock Divider</td>
  </tr>
  <tr>
    <td>Trigger Jitter</td>
    <td>4 ns * Clock Divider</td>
  </tr>
  <tr>
    <td>Trigger to Output Delay</td>
    <td>28 ns * Clock Divider</td>
  </tr>
</table>
