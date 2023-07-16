A small command line utility for MS-Windows written in C in VisualStudio 2019, which creates multiple MD5 hashes of a single file. Also known as "segmented hashing".

USAGE: MultiHash.exe FileToHash NumberOfHashSegments OffsetRange e.g.: 1BEEF-20000

For example:

MultiHash.exe BigFile.bin 100

...calculates 100 consecutive MD5 hashes from the entire Bigfile.bin.
In other words: it dividec the BigFile.bin into 100 equal size segments and calculated a MD5 hash of each segment.

MultiHash.exe BigFile.bin 100 642c06f40-642f509a6

...calculates 100 consecutive MD5 hashes from the Bigfile.bin starting from haxadecimal offset 642c06f40 and ending on the offset 642f509a6.

This utility does not write any files.  It only reads the file in BUFSIZE chunks (see the source code) and calculates the hashes.

WHAT IS THIS USEFUL FOR:
Scenario:  You have donloaded a 16TB file over a slow FTP connection but a several bytes of the file became corrupted.  This utility will allow you to detect which bytes did not transfer correctly, by running it on the FTP server AND on the FTP client machine and comparing the hashes of the file before and after the transfer.  Once a mismatching hash is identified, you can narrow down the file offsets to find the corrupted bytes, without doing the full 16TB file compare. Just several kB of hashes need to be compared.
