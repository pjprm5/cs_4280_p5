&&p4t4 : conditional, will obviously need to have variants checking each relational operator &&
declare y = 5 ;
program
start
   listen y ; 
   if [ y [ == ] 0 ] then
   start
      talk y ;
   stop ;
stop
