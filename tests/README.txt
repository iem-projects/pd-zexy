regression test framework
=========================

this is a simple framework for regression tests for pd-objects

HOW TESTS WORK:
---------------
each test is a patch with one (1) inlet and one (1) outlet.
a bang is sent to the inlet to start the test.
the test has to send "1" to the outlet if the test succeeded.
any other result (be it !=1 or no result at all will be 
considered a failure of the test.

example tests of [==]:

test1:
 [inlet]
 |
 [10 10(
 |
 [==]
 |
 [outlet]

test2:
 [inlet]
 |
 [10 10.1(
 |
 [==]
 |
 [select 0]
 |
 [1(
 |
 [outlet]


HOW THE FRAMEWORK WORKS
-----------------------
only .pd-files in one-level-subdirectories are considered tests.
(e.g. ./subdir/patch1.pd is tested, while ./patch2.pd and 
./sub/dir/patch3.pd are not taken into account)
see directory layout below
at the beginning of the testrun a file "runtests.txt" is generated
which contains all test-patches, one per line and each line
terminated by semicolon.
then pd is started and "runtests.txt" is read (via [textfile]).
for each line in the file, an object is created, a bang is
sent to the object and the result is received and compared with "1".
a result-message is printed.
when all the tests have been run, a summary of how many tests have
been run (and how many tests have been run successfully), and pd quits.
printout is first done into a logfile "runtests.log".
this logfile is printed to the stdout after pd quit.


CAVEATS
=======
 no testing of signal objects
 no testing of timed objects





