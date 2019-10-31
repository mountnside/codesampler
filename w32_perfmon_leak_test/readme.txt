

The test program "w32_perfmon_leak_test.exe" demonstrates how to track a memory 
leak using PerfMon and the GlobalAlloc and GlobalFree functions of the Win32 SDK.


To luanch Perfmon, simply click the "Run" button on the "Start" menu, 
type perfmon and click "Ok".


1. Once the test window comes up, launch PerfMon and right click anywhere 
   in the empty line-graph area and select "Add Counters..." from the 
   context menu.

2. Pull down the "Performance Object:" list box and select "Process".

3. Find and select "Private Bytes" from the counter list box.

4. Find and select "perfmon_leak_test" from the instance list box.

5. Click the "Add" button and close the "Add Counters" dialog box.

6. Use the "Test" menu of the application to start the leak by selecting 
   "Leak Memory".

7. Note the leak in PerfMon as the "Private Bytes" counter continues to
   climb with each memory allocation.

7. Use the "Test" menu of the application to stop and clean up the 
   leak by selecting "Free Memory".

