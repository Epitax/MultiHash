A small command line utility for MS-Windows written in C in VisualStudio 2019, which creates multiple MD5 hashes of a single file. Also known as "segmented hashing".

USAGE: MultiHash.exe FileToHash NumberOfHashSegments OffsetRange e.g.: 1BEEF-20000

For example:

MultiHash.exe BigFile.bin 100

...calculates 100 consecutive MD5 hashes of the entire Bigfile.bin.
In other words: it divides the BigFile.bin into 100 equal size segments and calculated a MD5 hash of each segment.

MultiHash.exe BigFile.bin 100 642c06f40-642f509a6

...calculates 100 consecutive MD5 hashes from the Bigfile.bin starting from the hexadecimal offset 642c06f40 and ending at the offset 642f509a6.

This utility does not write any files.  It only reads the file in BUFSIZE chunks (see the source in MultiHash.cpp) and calculates the hashes.

Q: WHAT IS THIS USEFUL FOR?

A: Scenario:  You have donloaded a 16TB file over a slow FTP connection for a week but a several bytes of the file came over corrupted.  
This utility will allow you to detect which bytes did not transfer correctly, by running it on the FTP server AND on the FTP client machine and comparing the hashes of the file before and after the transfer.
Once a mismatching hash is identified, you can narrow down the search to asmaller range of file offsets and find the corrupted bytes.
This is possible without doing the full 16TB file compare / re-donwload. Just several kB of hashes need to be tranferred and compared.
