#./jobCommander ServerName 8000 issueJob ls -l &
#sleep 1
../bin/./jobCommander ServerName 9000 issueJob ./progDelay 15 &
sleep 1
../bin/./jobCommander ServerName 9000 issueJob ./progDelay 20 &
sleep 1
../bin/./jobCommander ServerName 9000 issueJob ./progDelay 22 &
sleep 1
../bin/./jobCommander ServerName 9000 issueJob ./progDelay 15 &

sleep 1
../bin/./jobCommander ServerName 9000 exit &

#./jobCommander ServerName 9000 stop job_3 &
#./jobCommander ServerName 9000 setConcurrency 3 &
#sleep 1
#./jobCommander ServerName 9000 poll &



#sleep 1
#./jobCommander ServerName 8000 issueJob cat makefile &

#./jobCommander ServerName 8000 issueJob ls &
#./jobCommander ServerName 8000 issueJob cat main.cpp &
#./jobCommander ServerName 8000 issueJob cat multi.sh &
