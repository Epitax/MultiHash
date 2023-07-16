This is a small command line utility for MS-Windows written in C in VisualStudio 2019, which creates multiple MD5 hashes of a single file. Also known as "segmented hashing".

USAGE: MultiHash.exe FileToHash NumberOfHashSegments OffsetRange e.g.: 1BEEF-20000

For example:

MultiHash.exe BigFile.bin 100

...calculates 100 consecutive MD5 hashes of the entire Bigfile.bin file.
In other words: it divides the BigFile.bin into 100 equal size segments and calculates a MD5 hash of each segment.

MultiHash.exe BigFile.bin 100 642c06f40-642f509a6

...calculates 100 consecutive MD5 hashes of the Bigfile.bin starting from the hexadecimal offset 642c06f40 and ending at the offset 642f509a6 (inclusive).

The file offsets can also be specified in an open form, e.g.:<br>
-1CB2 means from the beginning of the file (offset 0) up to the file offset 0x1cb2 and
1BC2- means from the file offset 0x1cb2 up to the end of file.

Note: This utility does not write any files.  It only reads the file in BUFSIZE chunks (see the source in MultiHash.cpp) and calculates the hashes.

Q: WHAT IS THIS USEFUL FOR?

A: Scenario:  You have been downloading a 16TB file over a slow FTP connection for a week but several bytes of the file came over corrupted.  
This utility allows you to detect which bytes did not transfer correctly without doing the full 16TB file compare / re-download.
This is done by running the MultiHash utility on the FTP server AND on the FTP client machine and comparing only the hashes of that big file before and after the transfer.
Once a mismatching hash is identified, you can narrow down the search to a smaller range of file offsets and find the corrupted bytes.
Just several kB of hashes need to be tranferred and compared to find the culprit in a huge file.  Once this is done the correct bytes can be downloaded anew and used to patch the downloaded corrupted file.
